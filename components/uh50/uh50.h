//-------------------------------------------------------------------------------------
// ESPHome Landis+Gyr Ultraheat UH50 (T550) heat/cold meter reader
// Copyright 2020 Pär Svanström
//
// MIT License
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be included in all copies
// or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
//-------------------------------------------------------------------------------------
#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/uart/uart.h"
#include "obis.h"

namespace esphome {
namespace uh50 {

// Landis & Gyr EN 61107 mode B protocol
// http://manuals.lian98.biz/doc.en/html/u_iec62056_struct.htm
// https://github.com/lvzon/dsmr-p1-parser/blob/master/doc/IEC-62056-21-notes.md
class UH50Reader : public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void trigger_read();

  void set_uart_out(uart::UARTComponent *uart_out) { uart_out_ = uart_out; }
  void set_cumulative_energy_sensor(sensor::Sensor *s) { cumulative_energy_sensor_ = s; }
  void set_cumulative_volume_sensor(sensor::Sensor *s) { cumulative_volume_sensor_ = s; }
  void set_current_power_sensor(sensor::Sensor *s) { current_power_sensor_ = s; }
  void set_flow_rate_sensor(sensor::Sensor *s) { flow_rate_sensor_ = s; }
  void set_flow_temp_sensor(sensor::Sensor *s) { flow_temp_sensor_ = s; }
  void set_return_temp_sensor(sensor::Sensor *s) { return_temp_sensor_ = s; }
  void set_diff_temp_sensor(sensor::Sensor *s) { diff_temp_sensor_ = s; }

 protected:
  // The read is driven as a non-blocking state machine from loop() so the main
  // loop (Wi-Fi/API) stays responsive while the meter replies at 2400 baud.
  enum class State : uint8_t {
    IDLE,     // nothing to do
    READING,  // request sent, accumulating the telegram
  };

  void start_read();
  void send_data_cmd();
  void process_telegram();
  void finish_read();

  uart::UARTComponent *uart_out_{nullptr};

  sensor::Sensor *cumulative_energy_sensor_{nullptr};
  sensor::Sensor *cumulative_volume_sensor_{nullptr};
  sensor::Sensor *current_power_sensor_{nullptr};
  sensor::Sensor *flow_rate_sensor_{nullptr};
  sensor::Sensor *flow_temp_sensor_{nullptr};
  sensor::Sensor *return_temp_sensor_{nullptr};
  sensor::Sensor *diff_temp_sensor_{nullptr};

  static const size_t BUF_SIZE = 2500;
  static const uint32_t READ_TIMEOUT_MS = 10000;

  State state_{State::IDLE};
  uint32_t read_start_{0};  // millis() when the request was sent
  bool started_{false};     // seen the STX (start-of-text) byte yet
  size_t buf_pos_{0};
  char buffer_[BUF_SIZE];
  OBISData obisdata_[MAX_OBIS_CODES];
};

template<typename... Ts> class UH50ReadAction : public Action<Ts...>, public Parented<UH50Reader> {
 public:
  void play(Ts... x) override { this->parent_->trigger_read(); }
};

class UH50ReadButton : public button::Button, public Parented<UH50Reader> {
 protected:
  void press_action() override;
};

}  // namespace uh50
}  // namespace esphome
