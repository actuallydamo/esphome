#pragma once

namespace esphome {
namespace petkit_fountain {

static const char *const HUB_TAG = "petkit_fountain";

// BLE Characteristics UUIDs
static const uint16_t PETKIT_WRITE_UUID = 0xAAA2;
static const uint16_t PETKIT_READ_UUID = 0xAAA1;

// Protocol constants
static const uint8_t HEADER_BYTE_1 = 0xFA;  // 250
static const uint8_t HEADER_BYTE_2 = 0xFC;  // 252
static const uint8_t HEADER_BYTE_3 = 0xFD;  // 253
static const uint8_t END_BYTE = 0xFB;       // 251

// Packet types (command/response identifiers)
static const uint8_t PACKET_TYPE_BATTERY = 66;
static const uint8_t PACKET_TYPE_CONFIG = 221;
static const uint8_t PACKET_TYPE_DATETIME = 84;
static const uint8_t PACKET_TYPE_DEVICE_CONFIG = 211;
static const uint8_t PACKET_TYPE_DEVICE_FIRMWARE = 200;
static const uint8_t PACKET_TYPE_DEVICE_IDENTIFIERS = 213;
static const uint8_t PACKET_TYPE_DEVICE_STATE = 210;
static const uint8_t PACKET_TYPE_DEVICE_STATUS = 230;
static const uint8_t PACKET_TYPE_DEVICE_SYNC = 86;
static const uint8_t PACKET_TYPE_DEVICE_TYPE = 201;
static const uint8_t PACKET_TYPE_INIT_DEVICE = 73;
static const uint8_t PACKET_TYPE_MODE = 220;
static const uint8_t PACKET_TYPE_RESET_FILTER = 222;

// Message direction
static const uint8_t MESSAGE_DIRECTION_SEND = 1;
static const uint8_t MESSAGE_DIRECTION_RECEIVE = 2;

// Device modes
static const uint8_t MODE_NORMAL = 0;
static const uint8_t MODE_SMART = 1;

// Connection state machine states
enum PetkitConnectionState : uint8_t {
  STATE_DISCONNECTED = 0,
  STATE_CONNECTING = 1,
  STATE_CONNECTED = 2,
  STATE_INITIALIZED = 3,
  STATE_READY = 4,
};

// Command queue entry structure
struct CommandQueueEntry {
  uint8_t packet_type;
  std::vector<uint8_t> data;
  uint8_t retries_left;
  uint32_t timestamp;
};

}  // namespace petkit_fountain
}  // namespace esphome
