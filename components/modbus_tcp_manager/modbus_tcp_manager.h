#pragma once


#include "esphome.h"
#include <memory>


namespace esphome {
namespace modbus {


class ModbusTcpManager : public PollingComponent {
public:
ModbusTcpManager();


void setup() override;
void update() override;


// existing API (reading/writing registers) -- keep as-is
bool write_register(uint16_t reg, int16_t value);
bool write_registers(uint16_t start_reg, const std::vector<int16_t> &values);
bool read_registers(uint16_t start_reg, uint16_t quantity, std::vector<uint8_t> &out);


// new config setters
void set_request_timeout(std::chrono::milliseconds t) { this->request_timeout_ = t; }
void set_keepalive_interval(std::chrono::milliseconds t) { this->keepalive_interval_ = t; }
void set_keepalive_enabled(bool en) { this->keepalive_enabled_ = en; }


protected:
// connection management
bool ensure_connected();
void connect_now();
void close_connection();


// keepalive
void do_keepalive_if_needed();


// low-level send/recv helpers
bool send_bytes(const uint8_t *buf, size_t len);
bool recv_bytes_until(std::vector<uint8_t> &buffer, size_t bytes_expected);


// socket client (ESPhome TCP client wrapper)
std::unique_ptr<esphome::tcp::TCPClient> client_;


// configuration / state
std::string host_;
uint16_t port_ = 502;
uint8_t unit_id_ = 1;


std::chrono::milliseconds request_timeout_{5000}; // default 5s
std::chrono::milliseconds keepalive_interval_{0}; // 0 => disabled
bool keepalive_enabled_{false};


std::chrono::steady_clock::time_point last_activity_;
bool connected_{false};


// internal: simple reconnect backoff
uint32_t reconnect_attempts_ = 0;
};


} // namespace modbus
} // namespace esphome
