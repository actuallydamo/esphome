#ifdef USE_ESP32

#include "petkit_fountain_hub.h"
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace petkit_fountain {

static const uint32_t COMMAND_TIMEOUT_MS = 5000;
static const uint32_t HEARTBEAT_INTERVAL_MS = 60000;
static const uint8_t MAX_RETRIES = 3;
static const uint32_t COMMAND_DELAY_MS = 750;

PetkitFountainHub::PetkitFountainHub() : PollingComponent(15000) {}

void PetkitFountainHub::setup() {
  ESP_LOGCONFIG(HUB_TAG, "Setting up Petkit Fountain Hub...");
}

void PetkitFountainHub::loop() {
  if (this->is_connected()) {
    this->process_command_queue_();
    this->check_command_timeout_();
  }

  static uint32_t last_heartbeat = 0;
  if (this->is_ready() && millis() - last_heartbeat > HEARTBEAT_INTERVAL_MS) {
    last_heartbeat = millis();
    this->get_device_state_();
  }
}

void PetkitFountainHub::update() {
  if (!this->is_connected() && !this->operation_in_progress_) {
    ESP_LOGD(HUB_TAG, "Attempting to connect/reconnect to Petkit Fountain");
    this->node_state = espbt::ClientState::CONNECTING;
    this->connection_state_ = PetkitConnectionState::STATE_CONNECTING;
    this->parent_->connect();
  } else if (this->is_initialized()) {
    // Refresh device state
    this->get_device_state_();
    this->get_battery_();
  }
}

void PetkitFountainHub::dump_config() {
  ESP_LOGCONFIG(HUB_TAG, "Petkit Fountain:");
  ESP_LOGCONFIG(HUB_TAG, "  MAC Address: %s", this->parent_->address_str().c_str());

  if (this->is_ready()) {
    if (!this->device_name_.empty())
      ESP_LOGCONFIG(HUB_TAG, "  Device Name: %s", this->device_name_.c_str());

    if (!this->firmware_version_.empty())
      ESP_LOGCONFIG(HUB_TAG, "  Firmware Version: %s", this->firmware_version_.c_str());

    if (this->serial_ != 0)
      ESP_LOGCONFIG(HUB_TAG, "  Serial: %u", this->serial_);

    ESP_LOGCONFIG(HUB_TAG, "  Battery: %.1fV (%u%%)", this->battery_voltage_, this->battery_percentage_);
    ESP_LOGCONFIG(HUB_TAG, "  Power Status: %s", this->power_status_ ? "ON" : "OFF");
    ESP_LOGCONFIG(HUB_TAG, "  Mode: %s", this->mode_ == MODE_SMART ? "Smart" : "Normal");
    ESP_LOGCONFIG(HUB_TAG, "  Filter: %0.f%%", this->filter_percentage_ * 100);
  } else {
    ESP_LOGCONFIG(HUB_TAG, "  Not connected");
  }

  if (this->warning_breakdown_ || this->warning_water_missing_ || this->warning_filter_) {
    ESP_LOGCONFIG(HUB_TAG, "  Warnings:");
    if (this->warning_breakdown_)
      ESP_LOGCONFIG(HUB_TAG, "    Breakdown: Yes");
    if (this->warning_water_missing_)
      ESP_LOGCONFIG(HUB_TAG, "    Water Missing: Yes");
    if (this->warning_filter_)
      ESP_LOGCONFIG(HUB_TAG, "    Filter: Yes");
  }
}

void PetkitFountainHub::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                            esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_CONNECT_EVT: {
      this->connection_state_ = PetkitConnectionState::STATE_CONNECTING;
      ESP_LOGD(HUB_TAG, "Connected to Petkit Fountain");
      break;
    }

    case ESP_GATTC_DISCONNECT_EVT: {
      this->connection_state_ = PetkitConnectionState::STATE_DISCONNECTED;
      this->char_write_handle_ = 0;
      this->char_read_handle_ = 0;
      ESP_LOGW(HUB_TAG, "Disconnected from Petkit Fountain");
      break;
    }

    case ESP_GATTC_SEARCH_CMPL_EVT: {
      this->connection_state_ = PetkitConnectionState::STATE_CONNECTED;
      ESP_LOGI(HUB_TAG, "Service discovery completed");

      auto *char_write = this->parent_->get_characteristic(0, PETKIT_WRITE_UUID);
      if (char_write == nullptr) {
        ESP_LOGW(HUB_TAG, "No write characteristic found");
        break;
      }
      this->char_write_handle_ = char_write->handle;

      auto *char_read = this->parent_->get_characteristic(0, PETKIT_READ_UUID);
      if (char_read == nullptr) {
        ESP_LOGW(HUB_TAG, "No read characteristic found");
        break;
      }
      this->char_read_handle_ = char_read->handle;

      // Register for notifications
      if (this->char_read_handle_ != 0) {
        auto status = esp_ble_gattc_register_for_notify(this->parent_->get_gattc_if(), this->parent_->get_remote_bda(),
                                                        this->char_read_handle_);
        if (status) {
          ESP_LOGW(HUB_TAG, "Failed to register for notifications, status=%d", status);
        }
      }

      // Initialize the connection
      this->initialize_connection_();
      break;
    }

    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      ESP_LOGI(HUB_TAG, "Registered for notifications");
      break;
    }

    case ESP_GATTC_NOTIFY_EVT: {
      if (param->notify.handle == this->char_read_handle_) {
        this->handle_notification_(param->notify.value, param->notify.value_len);
      }
      break;
    }

    case ESP_GATTC_WRITE_CHAR_EVT: {
      if (this->handle_write_result_(&param->write)) {
        this->last_command_time_ = millis();
      }
      break;
    }

    default:
      break;
  }
}

bool PetkitFountainHub::handle_write_result_(esp_ble_gattc_cb_param_t::gattc_write_evt_param *param) {
  return param->status == ESP_GATT_OK;
}

void PetkitFountainHub::initialize_connection_() {
  ESP_LOGI(HUB_TAG, "Initializing Petkit Fountain");

  // Reset state
  this->sequence_number_ = 0;
  this->current_packet_type_ = 0;
  this->operation_in_progress_ = false;

  // Clear command queue
  while (!this->command_queue_.empty()) {
    this->command_queue_.pop();
  }

  // Start the initialization sequence
  this->init_device_();
}

void PetkitFountainHub::reset_connection_() {
  ESP_LOGI(HUB_TAG, "Resetting connection");

  // Clear pending operations
  this->operation_in_progress_ = false;
  this->current_packet_type_ = 0;

  // Try to disconnect and reconnect
  if (this->connection_state_ != PetkitConnectionState::STATE_DISCONNECTED) {
    this->connection_state_ = PetkitConnectionState::STATE_DISCONNECTED;
    this->parent_->disconnect();
  }
}

std::vector<uint8_t> PetkitFountainHub::build_command_(uint8_t packet_type, const std::vector<uint8_t> &data) {
  std::vector<uint8_t> command = {
      HEADER_BYTE_1,
      HEADER_BYTE_2,
      HEADER_BYTE_3,
      packet_type,
      MESSAGE_DIRECTION_SEND,
      this->sequence_number_,
      static_cast<uint8_t>(data.size()),
      0
  };

  command.insert(command.end(), data.begin(), data.end());

  command.push_back(END_BYTE);

  this->sequence_number_ = (this->sequence_number_ + 1) % 256;

  return command;
}

void PetkitFountainHub::queue_command_(uint8_t packet_type, const std::vector<uint8_t> &data) {
  if (!this->is_connected()) {
    ESP_LOGW(HUB_TAG, "Not connected, can't queue command %d", packet_type);
    return;
  }

  // Check if we already have a command of this type in the queue
  for (size_t i = 0; i < this->command_queue_.size(); i++) {
    auto entry = this->command_queue_.front();
    this->command_queue_.pop();

    if (entry.packet_type == packet_type) {
      // Replace with new data
      ESP_LOGD(HUB_TAG, "Replacing queued command %d with new data", packet_type);
      entry.data = data;
      entry.retries_left = MAX_RETRIES;
      entry.timestamp = millis();
      this->command_queue_.push(entry);
      return;
    } else {
      // Put it back in the queue
      this->command_queue_.push(entry);
    }
  }

  // Create a new entry
  CommandQueueEntry entry = {
      .packet_type = packet_type, .data = data, .retries_left = MAX_RETRIES, .timestamp = millis()};

  this->command_queue_.push(entry);
  ESP_LOGD(HUB_TAG, "Queued command %d, queue size: %zu", packet_type, this->command_queue_.size());
}

void PetkitFountainHub::process_command_queue_() {
  if (this->operation_in_progress_ || this->command_queue_.empty()) {
    return;
  }

  // Check if we need to wait between commands
  if (millis() - this->last_command_time_ < COMMAND_DELAY_MS) {
    return;
  }

  // Process the next command
  auto entry = this->command_queue_.front();
  this->command_queue_.pop();

  // Build the command
  auto command = this->build_command_(entry.packet_type, entry.data);

  // Set the current operation
  this->operation_in_progress_ = true;
  this->current_packet_type_ = entry.packet_type;
  this->last_command_time_ = millis();

  ESP_LOGD(HUB_TAG, "Sending command %d, seq=%d", entry.packet_type, this->sequence_number_ - 1);

  // Send the command
  auto status =
      esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(), this->char_write_handle_,
                               command.size(), command.data(), ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);

  if (status != ESP_OK) {
    ESP_LOGW(HUB_TAG, "Failed to send command %d, status=%d", entry.packet_type, status);
    this->operation_in_progress_ = false;

    // Requeue with retry
    if (entry.retries_left > 0) {
      entry.retries_left--;
      this->command_queue_.push(entry);
    }
  }
}

void PetkitFountainHub::check_command_timeout_() {
  if (!this->operation_in_progress_) {
    return;
  }

  // Check if the current operation has timed out
  if (millis() - this->last_command_time_ > COMMAND_TIMEOUT_MS) {
    ESP_LOGW(HUB_TAG, "Command %d timed out", this->current_packet_type_);
    this->operation_in_progress_ = false;
    this->current_packet_type_ = 0;

    // If we've had multiple timeouts, reset the connection
    this->reconnect_attempts_++;
    if (this->reconnect_attempts_ >= 3) {
      ESP_LOGW(HUB_TAG, "Too many timeouts, resetting connection");
      this->reset_connection_();
      this->reconnect_attempts_ = 0;
    }
  }
}

void PetkitFountainHub::handle_notification_(const uint8_t *data, uint16_t length) {
  // Reset reconnection attempts on successful notification
  this->reconnect_attempts_ = 0;

  if (length < 9) {
    ESP_LOGW(HUB_TAG, "Notification too short: %d bytes", length);
    return;
  }

  // Validate the header
  if (data[0] != HEADER_BYTE_1 || data[1] != HEADER_BYTE_2 || data[2] != HEADER_BYTE_3) {
    ESP_LOGW(HUB_TAG, "Invalid notification header");
    return;
  }

  // Validate the end byte
  if (data[length - 1] != END_BYTE) {
    ESP_LOGW(HUB_TAG, "Invalid notification end byte");
    return;
  }

  // Extract packet info
  uint8_t packet_type = data[3];
  // data[4] is direction (always 2 here)
  uint8_t sequence = data[5];
  uint8_t data_length = data[6];

  ESP_LOGD(HUB_TAG, "Received notification: type=%d, seq=%d, len=%d", packet_type, sequence, data_length);

  // Extract data
  std::vector<uint8_t> payload;
  for (int i = 8; i < length - 1; i++) {
    payload.push_back(data[i]);
  }

  // Process the notification
  switch (packet_type) {
    case PACKET_TYPE_BATTERY:
      this->parse_battery_data_(payload);
      break;
    case PACKET_TYPE_DEVICE_IDENTIFIERS:
      this->parse_device_identifiers_(payload);
      break;
    case PACKET_TYPE_DEVICE_SYNC:
      this->parse_device_sync_(payload);
      break;
    case PACKET_TYPE_DEVICE_FIRMWARE:
      this->parse_device_info_(payload);
      break;
    case PACKET_TYPE_DEVICE_TYPE:
      this->parse_device_type_(payload);
      break;
    case PACKET_TYPE_DEVICE_STATE:
      this->parse_device_state_(payload);
      break;
    case PACKET_TYPE_DEVICE_CONFIG:
      this->parse_device_config_(payload);
      break;
    default:
      ESP_LOGD(HUB_TAG, "Unhandled notification type: %d", packet_type);
      break;
  }

  // If this matches our current operation, mark it as complete
  if (packet_type == this->current_packet_type_) {
    ESP_LOGD(HUB_TAG, "Operation %d complete", packet_type);
    this->operation_in_progress_ = false;
    this->current_packet_type_ = 0;
  }

  // Update child components
  this->update_children_();

  // If we've initialized the device, move to the next step
  if (this->connection_state_ == PetkitConnectionState::STATE_CONNECTED) {
    if (packet_type == PACKET_TYPE_DEVICE_IDENTIFIERS) {
      // Get device sync after getting IDs
      this->get_device_sync_();
    } else if (packet_type == PACKET_TYPE_DEVICE_SYNC) {
      // Set datetime after sync
      this->set_datetime_();
    } else if (packet_type == PACKET_TYPE_DATETIME) {
      // Get device info after setting datetime
      this->get_device_info_();
    } else if (packet_type == PACKET_TYPE_DEVICE_FIRMWARE) {
      // Get device type after info
      this->get_device_type_();
    } else if (packet_type == PACKET_TYPE_DEVICE_TYPE) {
      // Get battery after device type
      this->get_battery_();
    } else if (packet_type == PACKET_TYPE_BATTERY) {
      // Get device state after battery
      this->get_device_state_();
    } else if (packet_type == PACKET_TYPE_DEVICE_STATE) {
      // Get device config after state
      this->get_device_config_();
    } else if (packet_type == PACKET_TYPE_DEVICE_CONFIG) {
      // Device is fully initialized after getting config
      this->connection_state_ = PetkitConnectionState::STATE_INITIALIZED;
      ESP_LOGI(HUB_TAG, "Petkit Fountain initialized");
      this->update_children_();
    }
  }
}

void PetkitFountainHub::update_children_() {
  for (auto *child : this->children_) {
    child->update_state();
  }
}

// Command implementations
void PetkitFountainHub::init_device_() {
  ESP_LOGI(HUB_TAG, "Initializing device");

  // First, get device identifiers
  this->get_device_identifiers_();
}

void PetkitFountainHub::get_device_identifiers_() {
  ESP_LOGD(HUB_TAG, "Getting device identifiers");
  this->queue_command_(PACKET_TYPE_DEVICE_IDENTIFIERS, {});
}

void PetkitFountainHub::get_device_sync_() {
  ESP_LOGD(HUB_TAG, "Getting device sync");
  std::vector<uint8_t> data = {0, 0};

  // Add secret if we have one
  if (!this->secret_bytes_.empty()) {
    data.insert(data.end(), this->secret_bytes_.begin(), this->secret_bytes_.end());
  } else {
    // Default secret
    data.insert(data.end(), {0, 0, 0, 0, 0, 0, 13, 37});
  }

  this->queue_command_(PACKET_TYPE_DEVICE_SYNC, data);
}

void PetkitFountainHub::set_datetime_() {
  ESP_LOGD(HUB_TAG, "Setting datetime");

  auto time_bytes = this->get_time_bytes_();
  this->queue_command_(PACKET_TYPE_DATETIME, time_bytes);
}

void PetkitFountainHub::get_device_info_() {
  ESP_LOGD(HUB_TAG, "Getting device info");
  this->queue_command_(PACKET_TYPE_DEVICE_FIRMWARE, {});
}

void PetkitFountainHub::get_device_type_() {
  ESP_LOGD(HUB_TAG, "Getting device type");
  this->queue_command_(PACKET_TYPE_DEVICE_TYPE, {});
}

void PetkitFountainHub::get_battery_() {
  ESP_LOGD(HUB_TAG, "Getting battery status");
  std::vector<uint8_t> data = {0, 0};
  this->queue_command_(PACKET_TYPE_BATTERY, data);
}

void PetkitFountainHub::get_device_state_() {
  ESP_LOGD(HUB_TAG, "Getting device state");
  std::vector<uint8_t> data = {0, 0};
  this->queue_command_(PACKET_TYPE_DEVICE_STATE, data);
}

void PetkitFountainHub::get_device_config_() {
  ESP_LOGD(HUB_TAG, "Getting device config");
  std::vector<uint8_t> data = {0, 0};
  this->queue_command_(PACKET_TYPE_DEVICE_CONFIG, data);
}

bool PetkitFountainHub::set_mode(uint8_t mode) {
  if (!this->is_initialized()) {
    ESP_LOGW(HUB_TAG, "Device not initialized");
    return false;
  }

  ESP_LOGI(HUB_TAG, "Setting mode to %d", mode);
  std::vector<uint8_t> data = {mode};
  this->queue_command_(PACKET_TYPE_MODE, data);

  // Update local state immediately for responsive UI
  this->mode_ = mode;
  this->update_children_();

  return true;
}

bool PetkitFountainHub::reset_filter() {
  if (!this->is_initialized()) {
    ESP_LOGW(HUB_TAG, "Device not initialized");
    return false;
  }

  ESP_LOGI(HUB_TAG, "Resetting filter");
  std::vector<uint8_t> data = {1};  // 1 to reset filter
  this->queue_command_(PACKET_TYPE_RESET_FILTER, data);
  return true;
}

bool PetkitFountainHub::set_power_status(bool power_on) {
  if (!this->is_initialized()) {
    ESP_LOGW(HUB_TAG, "Device not initialized");
    return false;
  }

  ESP_LOGI(HUB_TAG, "Setting power status to %d", power_on);

  // Use config command to set power status
  std::vector<uint8_t> data = {static_cast<uint8_t>(power_on ? 1 : 0)};
  this->queue_command_(PACKET_TYPE_CONFIG, data);

  // Update local state immediately for responsive UI
  this->power_status_ = power_on ? 1 : 0;
  this->update_children_();

  return true;
}

bool PetkitFountainHub::set_led_switch(bool led_on) {
  if (!this->is_initialized()) {
    ESP_LOGW(HUB_TAG, "Device not initialized");
    return false;
  }

  ESP_LOGI(HUB_TAG, "Setting LED switch to %d", led_on);

  // Create config data with LED switch value
  std::vector<uint8_t> data(14, 0);  // Initialize with 14 zeros
  data[2] = led_on ? 1 : 0;          // Set LED switch value

  // Fill in other values from current state
  data[0] = this->mode_;            // Smart time on
  data[1] = this->mode_;            // Smart time off (match mode for simplicity)
  data[3] = this->led_brightness_;  // LED brightness

  this->queue_command_(PACKET_TYPE_DEVICE_CONFIG, data);

  // Update local state immediately for responsive UI
  this->led_switch_ = led_on ? 1 : 0;
  this->update_children_();

  return true;
}

// Data parsers
void PetkitFountainHub::parse_battery_data_(const std::vector<uint8_t> &data) {
  if (data.size() < 3) {
    ESP_LOGW(HUB_TAG, "Battery data too short");
    return;
  }

  this->battery_voltage_ = ((data[0] * 256) + data[1]) / 1000.0f;
  this->battery_percentage_ = data[2];

  ESP_LOGI(HUB_TAG, "Battery: %.2fV (%d%%)", this->battery_voltage_, this->battery_percentage_);
}

void PetkitFountainHub::parse_device_identifiers_(const std::vector<uint8_t> &data) {
  if (data.size() < 12) {
    ESP_LOGW(HUB_TAG, "Device identifiers data too short");
    return;
  }

  // Extract device ID and serial
  this->device_id_bytes_ = std::vector<uint8_t>(data.begin(), data.begin() + 8);
  this->device_id_ = 0;
  for (int i = 0; i < 4; i++) {
    this->device_id_ = (this->device_id_ << 8) | this->device_id_bytes_[i];
  }

  this->serial_ = 0;
  for (int i = 7; i >= 4; i--) {
    this->serial_ = (this->serial_ << 8) | data[i];
  }

  // Create secret from device ID
  this->secret_bytes_.clear();
  // Reverse the device ID bytes and replace last two bytes with 13, 37 if they're zero
  for (int i = this->device_id_bytes_.size() - 1; i >= 0; i--) {
    if (i == 1 && this->device_id_bytes_[i] == 0 && this->device_id_bytes_[0] == 0) {
      this->secret_bytes_.push_back(13);
    } else if (i == 0 && this->device_id_bytes_[i] == 0 && this->device_id_bytes_[1] == 0) {
      this->secret_bytes_.push_back(37);
    } else {
      this->secret_bytes_.push_back(this->device_id_bytes_[i]);
    }
  }

  ESP_LOGI(HUB_TAG, "Device ID: %u, Serial: %u", this->device_id_, this->serial_);
}

void PetkitFountainHub::parse_device_sync_(const std::vector<uint8_t> &data) {
  if (data.empty()) {
    ESP_LOGW(HUB_TAG, "Device sync data too short");
    return;
  }

  bool initialized = data[0] != 0;
  ESP_LOGI(HUB_TAG, "Device initialized: %s", initialized ? "yes" : "no");

  if (!initialized) {
    // If the device isn't initialized, we need to initialize it
    std::vector<uint8_t> init_data = {0, 0};

    // Add device ID
    init_data.insert(init_data.end(), this->device_id_bytes_.begin(), this->device_id_bytes_.end());

    // Add secret
    init_data.insert(init_data.end(), this->secret_bytes_.begin(), this->secret_bytes_.end());

    ESP_LOGI(HUB_TAG, "Sending init device command");
    this->queue_command_(PACKET_TYPE_INIT_DEVICE, init_data);
  }
}

void PetkitFountainHub::parse_device_info_(const std::vector<uint8_t> &data) {
  if (data.size() < 2) {
    ESP_LOGW(HUB_TAG, "Device info data too short");
    return;
  }

  this->firmware_version_ = to_string(data[0]) + "." + to_string(data[1]);
  ESP_LOGI(HUB_TAG, "Firmware version: %s", this->firmware_version_.c_str());
}

void PetkitFountainHub::parse_device_type_(const std::vector<uint8_t> &data) {
  std::map<uint8_t, std::string> model_map = {{0, "Petkit_W5"},   {1, "Petkit_W5C"}, {2, "Petkit_W5N"},
                                              {3, "Petkit_CTW2"}, {4, "Petkit_W4X"}, {5, "Petkit_W4XUVC"}};

  if (!data.empty()) {
    uint8_t model_code = data[0];
    if (model_map.find(model_code) != model_map.end()) {
      this->device_name_ = model_map[model_code];
    } else {
      this->device_name_ = "Petkit_Unknown";
    }

    ESP_LOGI(HUB_TAG, "Device type: %s", this->device_name_.c_str());
  }
}

void PetkitFountainHub::parse_device_state_(const std::vector<uint8_t> &data) {
  if (data.size() < 12) {
    ESP_LOGW(HUB_TAG, "Device state data too short");
    return;
  }

  this->power_status_ = data[0];
  this->mode_ = data[1];
  // data[2] is DND state
  this->warning_breakdown_ = data[3];
  this->warning_water_missing_ = data[4];
  this->warning_filter_ = data[5];

  // Pump runtime is a 4-byte integer (bytes 6-9)
  this->pump_runtime_ = (data[6] << 24) | (data[7] << 16) | (data[8] << 8) | data[9];

  // Filter percenHUB_TAGe is byte 10 / 100
  this->filter_percentage_ = data[10] / 100.0f;

  // Running status is byte 11
  this->running_status_ = data[11];

  ESP_LOGI(HUB_TAG, "Device state: power=%d, mode=%d, filter=%.0f%%, running=%d", this->power_status_, this->mode_,
           this->filter_percentage_ * 100, this->running_status_);

  if (this->warning_breakdown_ || this->warning_water_missing_ || this->warning_filter_) {
    ESP_LOGW(HUB_TAG, "Warnings: breakdown=%d, water_missing=%d, filter=%d", this->warning_breakdown_,
             this->warning_water_missing_, this->warning_filter_);
  }
}

void PetkitFountainHub::parse_device_config_(const std::vector<uint8_t> &data) {
  if (data.size() < 14) {
    ESP_LOGW(HUB_TAG, "Device config data too short");
    return;
  }

  // data[0] is smart time on
  // data[1] is smart time off
  this->led_switch_ = data[2];
  this->led_brightness_ = data[3];
  // Bytes 4-13 are timing related values

  ESP_LOGI(HUB_TAG, "Device config: led_switch=%d, led_brightness=%d", this->led_switch_, this->led_brightness_);

  // Mark device as ready
  this->connection_state_ = PetkitConnectionState::STATE_READY;
}

std::vector<uint8_t> PetkitFountainHub::get_time_bytes_() {
  // Get current time
  time_t now = time_t(nullptr);
  struct tm *timeinfo = localtime(&now);

  std::vector<uint8_t> time_bytes(8, 0);

  // Format: year (2 bytes), month, day, hour, minute, second, weekday
  time_bytes[0] = (timeinfo->tm_year + 1900) >> 8;
  time_bytes[1] = (timeinfo->tm_year + 1900) & 0xFF;
  time_bytes[2] = timeinfo->tm_mon + 1;
  time_bytes[3] = timeinfo->tm_mday;
  time_bytes[4] = timeinfo->tm_hour;
  time_bytes[5] = timeinfo->tm_min;
  time_bytes[6] = timeinfo->tm_sec;
  time_bytes[7] = timeinfo->tm_wday;

  return time_bytes;
}

}  // namespace petkit_fountain
}  // namespace esphome

#endif  // USE_ESP32
