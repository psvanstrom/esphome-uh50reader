#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/api/custom_api_device.h"

namespace esphome {
namespace uh50 {

#define BUF_SIZE 4000
#define READ_TIMEOUT 25000
#define SILENCE_TIMEOUT 1500

static const byte SIGN_ON_MSG[] = {0x2F, 0x23, 0x21, 0x0D, 0x0A};

class UH50Component : public Component, public uart::UARTDevice, public api::CustomAPIDevice {
 public:
  void set_cumulative_active_import_sensor(sensor::Sensor *s) { cumulative_active_import = s; }
  void set_cumulative_volume_sensor(sensor::Sensor *s) { cumulative_volume = s; }
  void set_current_power_sensor(sensor::Sensor *s) { current_power = s; }
  void set_flow_rate_sensor(sensor::Sensor *s) { flow_rate = s; }
  void set_temperature_flow_sensor(sensor::Sensor *s) { temperature_flow = s; }
  void set_temperature_return_sensor(sensor::Sensor *s) { temperature_return = s; }
  void set_temperature_diff_sensor(sensor::Sensor *s) { temperature_diff = s; }
  void set_operating_hours_sensor(sensor::Sensor *s) { operating_hours = s; }
  void set_max_power_period_sensor(sensor::Sensor *s) { max_power_period = s; }
  void set_max_flow_period_sensor(sensor::Sensor *s) { max_flow_period = s; }
  void set_max_temp_flow_period_sensor(sensor::Sensor *s) { max_temp_flow_period = s; }
  void set_max_temp_return_period_sensor(sensor::Sensor *s) { max_temp_return_period = s; }
  void set_prev_month_energy_sensor(sensor::Sensor *s) { prev_month_energy = s; }
  void set_prev_month_volume_sensor(sensor::Sensor *s) { prev_month_volume = s; }
  void set_error_code_sensor(sensor::Sensor *s) { error_code = s; }

  void setup() override {
    time_last_run_ = 0;
    register_service(&UH50Component::trigger_read, "start_read_meter");
  }
  void set_update_interval(uint32_t interval) { update_interval_ = interval; }

  void trigger_read() {
    if (state_ == STATE_IDLE) {
      manual_trigger_ = true;
      ESP_LOGI("uh50", "Auslesung gestartet.");
    }
  }

  void loop() override {
    unsigned long now = millis();

    switch (state_) {
      case STATE_IDLE:
        if (manual_trigger_ ||
            (time_last_run_ == 0 && now > 10000UL) ||
            (update_interval_ > 0 && time_last_run_ != 0 && now - time_last_run_ >= update_interval_)) {
          state_ = STATE_INIT_BAUD;
          manual_trigger_ = false;
          time_last_run_ = now;
          state_timer_ = now;
        }
        break;

      case STATE_INIT_BAUD:
        set_baud_(300);
        state_ = STATE_WAIT_WAKEUP_DELAY;
        state_timer_ = now;
        break;

      case STATE_WAIT_WAKEUP_DELAY:
        if (now - state_timer_ >= 50) {
          uint8_t nuls[65] = {0};
          write_array(nuls, 65);
          state_ = STATE_WAIT_AFTER_WAKEUP;
          state_timer_ = now;
        }
        break;

      case STATE_WAIT_AFTER_WAKEUP:
        // 65 NUL bytes at 300 baud and 7E2 = 715 bits = ~2.38 seconds transmission time.
        // IEC 62056-21 requires a 1.5s pause after the wake-up sequence.
        // Wait 4000ms from enqueue to provide this pause without blocking the ESPHome loop.
        if (now - state_timer_ >= 4000) {
          buf_pos_ = 0;
          memset(buffer_, 0, BUF_SIZE);
          write_array(SIGN_ON_MSG, sizeof(SIGN_ON_MSG));
          state_ = STATE_WAIT_FOR_IDENT;
          state_timer_ = now;
        }
        break;

      case STATE_WAIT_FOR_IDENT:
        while (available()) {
          char c = read();
          if (buf_pos_ < BUF_SIZE - 1) buffer_[buf_pos_++] = c;
          if (c == 0x0A) {
            buffer_[buf_pos_] = 0;
            ESP_LOGI("uh50", "IDENT: %s", buffer_);
            char speed_char = (buf_pos_ >= 5 && buffer_[0] == '/') ? buffer_[4] : '3';
            negotiated_baud_ = baud_from_char_(speed_char);
            ESP_LOGI("uh50", "Aushandelte Baudrate: %d (Zeichen '%c')", negotiated_baud_, speed_char);
            uint8_t ack[] = {0x06, 0x30, (uint8_t) speed_char, 0x30, 0x0D, 0x0A};
            write_array(ack, sizeof(ack));
            flush();
            state_ = STATE_WAIT_BAUD_SWITCH;
            state_timer_ = now;
            return;
          }
        }
        if (now - state_timer_ > 5000) state_ = STATE_IDLE;
        break;

      case STATE_WAIT_BAUD_SWITCH:
        if (now - state_timer_ >= 300) {
          set_baud_(negotiated_baud_);
          buf_pos_ = 0;
          memset(buffer_, 0, BUF_SIZE);
          state_ = STATE_READING_DATA;
          state_timer_ = now;
          last_rx_time_ = now;
        }
        break;


      case STATE_READING_DATA:
        while (available()) {
          char c = read();
          if (buf_pos_ < BUF_SIZE - 1) buffer_[buf_pos_++] = c;
          last_rx_time_ = now;
        }
        if (buf_pos_ > 0 && (now - last_rx_time_ > SILENCE_TIMEOUT)) {
          ESP_LOGI("uh50", "Empfang beendet (%d Bytes).", buf_pos_);
          process_buffer_();
          state_ = STATE_IDLE;
        }
        if (now - state_timer_ > READ_TIMEOUT) {
          if (buf_pos_ > 0) process_buffer_();
          state_ = STATE_IDLE;
        }
        break;
    }
  }

 private:
  enum State { STATE_IDLE, STATE_INIT_BAUD, STATE_WAIT_WAKEUP_DELAY, STATE_WAIT_AFTER_WAKEUP, STATE_WAIT_FOR_IDENT, STATE_WAIT_BAUD_SWITCH, STATE_READING_DATA };
  State state_{STATE_IDLE};
  uint32_t update_interval_{0};
  unsigned long time_last_run_{0};
  unsigned long state_timer_{0};
  unsigned long last_rx_time_{0};
  bool manual_trigger_{false};
  char buffer_[BUF_SIZE];
  int buf_pos_{0};
  int negotiated_baud_{2400};

  int baud_from_char_(char c) {
    switch (c) {
      // Mode C (digits)
      case '0': return 300;
      case '1': return 600;
      case '2': return 1200;
      case '3': return 2400;
      case '4': return 4800;
      case '5': return 9600;
      case '6': return 19200;
      // Mode B letters removed: Mode B switches automatically and does not take the Mode C ACK.
      // Since this component explicitly sends a Mode C ACK, claiming Mode B support was invalid.
      default:  return 2400;
    }
  }

  sensor::Sensor *cumulative_active_import{nullptr};
  sensor::Sensor *cumulative_volume{nullptr};
  sensor::Sensor *current_power{nullptr};
  sensor::Sensor *flow_rate{nullptr};
  sensor::Sensor *temperature_flow{nullptr};
  sensor::Sensor *temperature_return{nullptr};
  sensor::Sensor *temperature_diff{nullptr};
  sensor::Sensor *operating_hours{nullptr};
  sensor::Sensor *max_power_period{nullptr};
  sensor::Sensor *max_flow_period{nullptr};
  sensor::Sensor *max_temp_flow_period{nullptr};
  sensor::Sensor *max_temp_return_period{nullptr};
  sensor::Sensor *prev_month_energy{nullptr};
  sensor::Sensor *prev_month_volume{nullptr};
  sensor::Sensor *error_code{nullptr};

  void set_baud_(int baud) {
    this->parent_->set_baud_rate(baud);
    this->parent_->load_settings();
  }

  float extract_value_(const char *buf, const char *code) {
    char search[32];
    snprintf(search, sizeof(search), "%s(", code);
    char *pos = strstr((char *) buf, search);
    while (pos != nullptr) {
      bool fp = false;
      if (pos > buf) {
        char prev = *(pos - 1);
        if (prev == '.' || (prev >= '0' && prev <= '9')) fp = true;
      }
      if (!fp) return atof(pos + strlen(search));
      pos = strstr(pos + 1, search);
    }
    return -9999.0f;
  }

  float extract_combined_value_(const char *buf, const char *code, int index) {
    char search[32];
    snprintf(search, sizeof(search), "%s(", code);
    char *pos = strstr((char *) buf, search);
    if (pos) {
      char *val = pos + strlen(search);
      if (index == 0) return atof(val);
      if (index == 1) {
        char *sep = strchr(val, '&');
        if (sep) return atof(sep + 1);
      }
    }
    return -9999.0f;
  }

  float extract_history_(const char *buf, const char *code) {
    char search[32];
    snprintf(search, sizeof(search), "%s*01(", code);
    char *pos = strstr((char *) buf, search);
    if (pos) return atof(pos + strlen(search));
    return -9999.0f;
  }

  void process_buffer_() {
    buffer_[buf_pos_] = 0;
    ESP_LOGD("uh50", "RAW DATA:\n%s", buffer_);
    float val;

    val = extract_value_(buffer_, "6.8");
    if (val != -9999.0f && cumulative_active_import) cumulative_active_import->publish_state(val);

    val = extract_value_(buffer_, "6.26");
    if (val != -9999.0f && cumulative_volume) cumulative_volume->publish_state(val);

    val = extract_value_(buffer_, "6.4");
    if (val != -9999.0f && current_power) current_power->publish_state(val);

    val = extract_value_(buffer_, "6.27");
    if (val != -9999.0f && flow_rate) flow_rate->publish_state(val);

    val = extract_value_(buffer_, "6.29");
    if (val != -9999.0f && temperature_flow) temperature_flow->publish_state(val);

    val = extract_value_(buffer_, "6.28");
    if (val != -9999.0f && temperature_return) temperature_return->publish_state(val);

    if (temperature_diff) {
      val = extract_value_(buffer_, "6.30");
      if (val != -9999.0f) {
        temperature_diff->publish_state(val);
      } else {
        float tf = extract_value_(buffer_, "6.29");
        float tr = extract_value_(buffer_, "6.28");
        if (tf != -9999.0f && tr != -9999.0f) temperature_diff->publish_state(fabsf(tf - tr));
      }
    }

    val = extract_value_(buffer_, "6.31");
    if (val != -9999.0f && operating_hours) operating_hours->publish_state(val);

    val = extract_value_(buffer_, "6.6");
    if (val != -9999.0f && max_power_period) max_power_period->publish_state(val);

    val = extract_value_(buffer_, "6.33");
    if (val != -9999.0f && max_flow_period) max_flow_period->publish_state(val);

    val = extract_combined_value_(buffer_, "9.4", 0);
    if (val != -9999.0f && max_temp_flow_period) max_temp_flow_period->publish_state(val);

    val = extract_combined_value_(buffer_, "9.4", 1);
    if (val != -9999.0f && max_temp_return_period) max_temp_return_period->publish_state(val);

    val = extract_history_(buffer_, "6.8");
    if (val != -9999.0f && prev_month_energy) prev_month_energy->publish_state(val);

    val = extract_history_(buffer_, "6.26");
    if (val != -9999.0f && prev_month_volume) prev_month_volume->publish_state(val);

    char *fpos = strstr(buffer_, "\nF(");
    if (!fpos) fpos = strstr(buffer_, " F(");
    if (fpos) {
      int fval = atoi(fpos + 3);
      if (error_code) error_code->publish_state((float) fval);
      if (fval == 0) {
        ESP_LOGD("uh50", "Fehlerstatus: OK (0)");
      } else {
        const char *msg = "Unbekannter Fehler";
        if      (fval == 1) msg = "Unterbrechung Temperaturfühler warme Seite";
        else if (fval == 2) msg = "Unterbrechung Temperaturfühler kalte Seite";
        else if (fval == 3) msg = "Elektronik Temperaturauswertung defekt — Gerät tauschen";
        else if (fval == 4) msg = "BATTERIE LEER — Batterie tauschen!";
        else if (fval == 5) msg = "Kurzschluss Temperaturfühler warme Seite";
        else if (fval == 6) msg = "Kurzschluss Temperaturfühler kalte Seite";
        else if (fval == 7) msg = "Störung interner Speicher — Gerät tauschen";
        else if (fval == 8) msg = "Manipulationsschutz ausgelöst — Service erforderlich";
        else if (fval == 9) msg = "Elektronikfehler — Gerät tauschen";
        ESP_LOGW("uh50", "FEHLER F%d: %s", fval, msg);
      }
    }
  }
};

}  // namespace uh50
}  // namespace esphome
