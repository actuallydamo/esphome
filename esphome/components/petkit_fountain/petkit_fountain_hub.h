#pragma once
#ifdef USE_ESP32

#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "petkit_fountain_const.h"
#include "petkit_fountain_child.h"

#include <vector>
#include <queue>

namespace esphome {
namespace petkit_fountain {

namespace espbt = esphome::esp32_ble_tracker;

class PetkitFountainHub : public esphome::ble_client::BLEClientNode, public PollingComponent {
 public:
  PetkitFountainHub();

  // Component implementation
  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_BLUETOOTH; }

  // BLEClientNode implementation
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

  // API functions for child components
  void register_child(PetkitFountainChild *child) { this->children_.push_back(child); }
  bool is_connected() const { return this->connection_state_ >= PetkitConnectionState::STATE_CONNECTED; }
  bool is_initialized() const { return this->connection_state_ >= PetkitConnectionState::STATE_INITIALIZED; }
  bool is_ready() const { return this->connection_state_ >= PetkitConnectionState::STATE_READY; }

  // Command functions
  bool set_mode(uint8_t mode);
  bool reset_filter();
  bool set_power_status(bool power_on);
  bool set_led_switch(bool led_on);

  // Getters for device state
  float get_battery_voltage() const { return this->battery_voltage_; }
  uint8_t get_battery_percentage() const { return this->battery_percentage_; }
  uint8_t get_power_status() const { return this->power_status_; }
  uint8_t get_mode() const { return this->mode_; }
  uint8_t get_led_switch() const { return this->led_switch_; }
  uint8_t get_led_brightness() const { return this->led_brightness_; }
  uint8_t get_warning_breakdown() const { return this->warning_breakdown_; }
  uint8_t get_warning_water_missing() const { return this->warning_water_missing_; }
  uint8_t get_warning_filter() const { return this->warning_filter_; }
  uint32_t get_pump_runtime() const { return this->pump_runtime_; }
  uint8_t get_filter_percentage() const { return this->filter_percentage_; }
  uint8_t get_running_status() const { return this->running_status_; }
  std::string get_device_name() const { return this->device_name_; }
  std::string get_firmware_version() const { return this->firmware_version_; }
  uint32_t get_device_id() const { return this->device_id_; }
  uint32_t get_serial() const { return this->serial_; }

 protected:
  // BLE communication
  void initialize_connection_();
  void reset_connection_();
  void queue_command_(uint8_t packet_type, const std::vector<uint8_t> &data);
  void process_command_queue_();
  void handle_notification_(const uint8_t *data, uint16_t length);

  // Command builders
  std::vector<uint8_t> build_command_(uint8_t packet_type, const std::vector<uint8_t> &data);

  // Initialization sequence methods
  void init_device_();
  void get_device_identifiers_();
  void get_device_sync_();
  void set_datetime_();
  void get_device_info_();
  void get_device_type_();
  void get_device_state_();
  void get_device_config_();
  void get_battery_();

  // Data parsing
  void parse_battery_data_(const std::vector<uint8_t> &data);
  void parse_device_identifiers_(const std::vector<uint8_t> &data);
  void parse_device_sync_(const std::vector<uint8_t> &data);
  void parse_device_info_(const std::vector<uint8_t> &data);
  void parse_device_type_(const std::vector<uint8_t> &data);
  void parse_device_state_(const std::vector<uint8_t> &data);
  void parse_device_config_(const std::vector<uint8_t> &data);

  // Helper methods
  std::vector<uint8_t> get_time_bytes_();
  void update_children_();
  void check_command_timeout_();
  bool handle_write_result_(esp_ble_gattc_cb_param_t::gattc_write_evt_param *param);

  // BLE characteristics
  uint16_t char_write_handle_{0};
  uint16_t char_read_handle_{0};

  // Connection state
  PetkitConnectionState connection_state_{STATE_DISCONNECTED};
  uint8_t sequence_number_{0};
  uint32_t last_notification_time_{0};
  bool operation_in_progress_{false};
  uint8_t current_packet_type_{0};
  uint32_t last_command_time_{0};
  uint32_t reconnect_attempts_{0};
  std::vector<uint8_t> device_id_bytes_{};
  std::vector<uint8_t> secret_bytes_{};

  // Command queue
  std::queue<CommandQueueEntry> command_queue_{};

  // Device state properties
  float battery_voltage_{0};
  uint8_t battery_percentage_{0};
  uint8_t power_status_{0};
  uint8_t mode_{0};
  uint8_t led_switch_{0};
  uint8_t led_brightness_{0};
  uint8_t warning_breakdown_{0};
  uint8_t warning_water_missing_{0};
  uint8_t warning_filter_{0};
  uint32_t pump_runtime_{0};
  float filter_percentage_{0};
  uint8_t running_status_{0};

  // Device info properties
  std::string device_name_{};
  std::string firmware_version_{};
  uint32_t device_id_{0};
  uint32_t serial_{0};

  // Child components
  std::vector<PetkitFountainChild *> children_{};
};

}  // namespace petkit_fountain
}  // namespace esphome

#endif  // USE_ESP32
