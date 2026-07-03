#include "uh50.h"
#include "obis.h"
#include "esphome/core/log.h"
#include <cstdlib>
#include <cstring>

namespace esphome {
namespace uh50 {

static const char *const TAG = "uh50.reader";

// IEC 62056-21 mode B request. The leading NUL bytes wake the optical head,
// followed by the "/#!\r\n" data request. Sent on the 300 baud TX line.
static const uint8_t DATA_CMD[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
    '/',  '#',  '!',  0x0D, 0x0A};

void UH50Reader::setup() { ESP_LOGCONFIG(TAG, "Setting up UH50 reader..."); }

void UH50Reader::update() {
  if (this->state_ != State::IDLE) {
    ESP_LOGW(TAG, "Read already in progress, skipping this update");
    return;
  }
  this->start_read();
}

void UH50Reader::trigger_read() {
  if (this->state_ != State::IDLE) {
    ESP_LOGW(TAG, "Read already in progress, ignoring trigger");
    return;
  }
  this->start_read();
}

void UH50ReadButton::press_action() { this->parent_->trigger_read(); }

void UH50Reader::start_read() {
  ESP_LOGD(TAG, "Starting UH50 read cycle...");
  this->send_data_cmd();
  this->buf_pos_ = 0;
  this->started_ = false;
  this->read_start_ = millis();
  this->state_ = State::READING;
}

void UH50Reader::send_data_cmd() {
  if (this->uart_out_ == nullptr) {
    ESP_LOGW(TAG, "No output UART configured, cannot send data request");
    return;
  }
  this->uart_out_->write_array(DATA_CMD, sizeof(DATA_CMD));
  ESP_LOGI(TAG, "Data cmd sent");
}

void UH50Reader::loop() {
  if (this->state_ != State::READING)
    return;

  // Consume the bytes ready now, but cap the work per loop() call so we always
  // yield back to the rest of the application.
  int budget = 128;
  while (budget-- > 0 && this->available()) {
    uint8_t b = this->read();

    // Fast-forward until the STX byte (start-of-text) that opens the telegram.
    if (!this->started_) {
      if (b == 0x02)
        this->started_ = true;
      continue;
    }

    // ETX (end-of-text) closes the telegram; parse what we collected.
    if (b == 0x03) {
      this->buffer_[this->buf_pos_] = '\0';
      this->process_telegram();
      this->finish_read();
      return;
    }

    if (this->buf_pos_ < BUF_SIZE - 1)
      this->buffer_[this->buf_pos_++] = (char) b;
  }

  if (millis() - this->read_start_ >= READ_TIMEOUT_MS) {
    if (this->buf_pos_ > 0) {
      ESP_LOGD(TAG, "Read timeout with %u bytes, parsing partial telegram", (unsigned) this->buf_pos_);
      this->buffer_[this->buf_pos_] = '\0';
      this->process_telegram();
    } else {
      ESP_LOGW(TAG, "Timed out waiting for telegram");
    }
    this->finish_read();
  }
}

void UH50Reader::process_telegram() {
  ESP_LOGD(TAG, "Read telegram: %s", this->buffer_);

  int count = 0;
  parse_obis(this->buffer_, this->obisdata_, &count);
  print_parsed_data(this->obisdata_, count);

  for (int i = 0; i < count; i++) {
    const char *code = this->obisdata_[i].obis_code;
    float value = atof(this->obisdata_[i].value);
    if (!strcmp(code, "6.8") && this->cumulative_energy_sensor_ != nullptr)
      this->cumulative_energy_sensor_->publish_state(value);
    else if (!strcmp(code, "6.26") && this->cumulative_volume_sensor_ != nullptr)
      this->cumulative_volume_sensor_->publish_state(value);
    else if (!strcmp(code, "6.4") && this->current_power_sensor_ != nullptr)
      this->current_power_sensor_->publish_state(value);
    else if (!strcmp(code, "6.27") && this->flow_rate_sensor_ != nullptr)
      this->flow_rate_sensor_->publish_state(value);
    else if (!strcmp(code, "6.29") && this->flow_temp_sensor_ != nullptr)
      this->flow_temp_sensor_->publish_state(value);
    else if (!strcmp(code, "6.28") && this->return_temp_sensor_ != nullptr)
      this->return_temp_sensor_->publish_state(value);
    else if (!strcmp(code, "6.30") && this->diff_temp_sensor_ != nullptr)
      this->diff_temp_sensor_->publish_state(value);
  }
}

void UH50Reader::finish_read() {
  this->buf_pos_ = 0;
  this->started_ = false;
  this->state_ = State::IDLE;
}

void UH50Reader::dump_config() {
  ESP_LOGCONFIG(TAG, "UH50 Reader:");
  ESP_LOGCONFIG(TAG, "  Read timeout: %u ms", (unsigned) READ_TIMEOUT_MS);
  LOG_SENSOR("  ", "Cumulative energy", this->cumulative_energy_sensor_);
  LOG_SENSOR("  ", "Cumulative volume", this->cumulative_volume_sensor_);
  LOG_SENSOR("  ", "Current power", this->current_power_sensor_);
  LOG_SENSOR("  ", "Flow rate", this->flow_rate_sensor_);
  LOG_SENSOR("  ", "Flow temperature", this->flow_temp_sensor_);
  LOG_SENSOR("  ", "Return temperature", this->return_temp_sensor_);
  LOG_SENSOR("  ", "Diff temperature", this->diff_temp_sensor_);
}

}  // namespace uh50
}  // namespace esphome
