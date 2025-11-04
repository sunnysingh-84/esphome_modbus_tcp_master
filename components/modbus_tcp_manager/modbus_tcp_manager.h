#pragma once

#include "esphome.h"
#include <WiFiClient.h>
#include <memory>
#include <vector>
#include <chrono>

namespace esphome {
namespace modbus {

static const char *TAG = "modbus_tcp_master";

class ModbusTcpMaster : public PollingComponent {
 public:
  ModbusTcpMaster() : PollingComponent(5000) {}

  // --- Configuration ---
  std::string host_;
  uint16_t port_ = 502;
  uint8_t unit_id_ = 1;

  // --- Persistent TCP members ---
  std::unique_ptr<WiFiClient> client_;
  bool connected_ = false;
  std::chrono::steady_clock::time_point last_activity_;
  std::chrono::milliseconds request_timeout_{5000};  // default 5s
  std::chrono::milliseconds keepalive_interval_{0};  // default disabled
  bool keepalive_enabled_{false};

  // --- Config setters ---
  void set_request_timeout(std::chrono::milliseconds t) { request_timeout_ = t; }
  void set_keepalive_interval(std::chrono::milliseconds t) { keepalive_interval_ = t; keepalive_enabled_ = t.count() > 0; }

  // --- Connection management ---
  void connect_now() {
    if (!client_) client_.reset(new WiFiClient());
    if (client_->connected()) return;
    if (!client_->connect(host_.c_str(), port_)) {
      ESP_LOGW(TAG, "Failed to connect to %s:%d", host_.c_str(), port_);
      connected_ = false;
      return;
    }
    ESP_LOGI(TAG, "Connected to %s:%d", host_.c_str(), port_);
    connected_ = true;
    last_activity_ = std::chrono::steady_clock::now();
  }

  bool ensure_connected() {
    if (client_ && client_->connected()) { connected_ = true; return true; }
    connect_now();
    return client_ && client_->connected();
  }

  void close_connection() {
    if (client_) client_->stop();
    connected_ = false;
  }

  // --- Soft keepalive ---
  void do_keepalive_if_needed() {
    if (!keepalive_enabled_ || keepalive_interval_.count() == 0) return;
    auto now = std::chrono::steady_clock::now();
    if (now - last_activity_ < keepalive_interval_) return;

    ESP_LOGD(TAG, "Keepalive triggered");
    if (!ensure_connected()) {
      ESP_LOGW(TAG, "Keepalive: not connected - attempted reconnect");
      last_activity_ = now;
      return;
    }
    if (!client_->connected()) { close_connection(); return; }
    last_activity_ = now;
  }

  // --- Low-level send/receive helpers ---
  bool send_bytes(const uint8_t *buf, size_t len) {
    if (!ensure_connected()) return false;
    int sent = client_->write(buf, len);
    if (sent != (int)len) { close_connection(); return false; }
    last_activity_ = std::chrono::steady_clock::now();
    return true;
  }

  bool recv_bytes_until(std::vector<uint8_t> &buffer, size_t bytes_expected) {
    if (!ensure_connected()) return false;
    auto start = std::chrono::steady_clock::now();
    while (buffer.size() < bytes_expected) {
      if (client_->available() > 0) {
        uint8_t b;
        int r = client_->read(&b, 1);
        if (r == 1) { buffer.push_back(b); last_activity_ = std::chrono::steady_clock::now(); continue; }
        close_connection(); return false;
      }
      auto now = std::chrono::steady_clock::now();
      if (now - start > request_timeout_) { close_connection(); return false; }
      delay(1);
    }
    return true;
  }

  // --- Setup and update ---
  void setup() override {
    client_.reset(new WiFiClient());
    connect_now();
    // ... existing setup logic
  }

  void update() override {
    do_keepalive_if_needed();
    // ... existing update logic
  }

  // --- Modbus read/write methods ---
  // Replace existing read/write logic to use send_bytes() and recv_bytes_until()
  bool read_registers(uint16_t start_reg, uint16_t quantity, std::vector<uint8_t> &out) {
    // Build Modbus TCP request, send_bytes(), recv_bytes_until()
    return false;  // placeholder
  }

  bool write_register(uint16_t reg, int16_t value) {
    return false;  // placeholder
  }

  bool write_registers(uint16_t start_reg, const std::vector<int16_t> &values) {
    return false;  // placeholder
  }
};

}  // namespace modbus
}  // namespace esphome
