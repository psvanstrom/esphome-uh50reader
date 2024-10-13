
//-------------------------------------------------------------------------------------
// ESPHome Landis+Gyr Ultraheat UH50 (T550) Heat Meter custom sensor
// Copyright 2020 Pär Svanström
// 
// History
//  0.1.0 2021-02-03:   Initial release
//
// MIT License
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
// IN THE SOFTWARE.
//-------------------------------------------------------------------------------------

#include "esphome.h"

#define BUF_SIZE 1500
#define WAIT_TIME 10

byte data_cmd[] = { 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x2F, 0x3F, 0x21, 0x0D, 0x0A
};

// Landis & Gyr EN 61107 mode B protocol
// http://manuals.lian98.biz/doc.en/html/u_iec62056_struct.htm
// https://www.ungelesen.net/protagWork/media/downloads/solar-steuerung/iec62056-21%7Bed1.0%7Den_.pdf
// http://tunn.us/arduino/landisgyr.php
// https://www.elvaco.se/Image/GetDocument/en/286/cmi4110-users-manual-english.pdf
// https://github.com/lvzon/dsmr-p1-parser/blob/master/doc/IEC-62056-21-notes.md
// http://womp3-14.vijfwal.nl/p2013c.html

class ParsedMessage {
  public:
    double cumulativeActiveImport;
    double cumulativeVolume;
};

class UH50Reader : public Component, public UARTDevice, public CustomAPIDevice {
  const char* DELIMITERS = "(*";
  UARTDevice uart_out;
  unsigned long timeLastRun;
  char buffer[BUF_SIZE];

  public:
    Sensor *cumulativeActiveImport = new Sensor();
    Sensor *cumulativeVolume = new Sensor();

    UH50Reader(UARTComponent *uart_in, UARTComponent *uart_out) : UARTDevice(uart_in), uart_out(uart_out) { }

    void setup() override {
      sendDataCmd();
      timeLastRun = millis() - (WAIT_TIME - 1) * 60000;

      //register the service which Home Assistant can call to read the meter
      register_service(&UH50Reader::read_meter, "start_read_meter");
    }

    void loop() override {
      if (millis() - timeLastRun > WAIT_TIME * 60000) {
        sendDataCmd();
        readTelegram();
        timeLastRun = millis();
      }
    }

    void read_meter() {
        sendDataCmd();
        readTelegram();
    }

  private:
    char* strtok_single (char * str, char const * delims) {
      static char  * src = NULL;
      char  *  p,  * ret = 0;

      if (str != NULL)
        src = str;

      if (src == NULL)
        return NULL;

      if ((p = strpbrk (src, delims)) != NULL) {
        *p  = 0;
        ret = src;
        src = ++p;

      } else if (*src) {
        ret = src;
        src = NULL;
      }

      return ret;
    }

    void parseRow(ParsedMessage* parsed, char* obis_code, char* value) {
      if (strncmp(obis_code, "6.8", 6) == 0) {
        parsed->cumulativeActiveImport = atof(value) * 1000;

      } else if (strncmp(obis_code, "6.26", 6) == 0) {
        parsed->cumulativeVolume = atof(value);

      }
    }

    void publishSensors(ParsedMessage* parsed) {
      cumulativeActiveImport->publish_state(parsed->cumulativeActiveImport);
      cumulativeVolume->publish_state(parsed->cumulativeVolume);
    }

    void sendDataCmd() {
      for (int i = 0; i < sizeof(data_cmd); i++) {
        uart_out.write(data_cmd[i]);
      }
      ESP_LOGI("cmd", "data cmd sent");
    }

    void readTelegram() {
      ParsedMessage parsed = ParsedMessage();
      
      bool publish=false;
      // fast forward until we find the STX byte (start-of-text)
      byte b = 0x00;
      while (available() && b != 0x02) {
        b = read();
      }

      while (int len = available()) {
        ESP_LOGD("readTelegram", "Got %d bytes available to read", len);
        if (!read_array((uint8_t *) buffer, len))
               ESP_LOGW("readTelegram", "read_array() returned false, meter reading may be incomplete");
        ESP_LOGD("readTelegram", "Read %s", buffer);

        if (len > 0) {
          // end character reached
          if (buffer[0] == '!') {
            publishSensors(&parsed);
            return;
          }

          char* obis_code = strtok_single(buffer, "(");
          while (obis_code != NULL) {
            char* value = strtok_single(NULL, "*)");
            char* unit = strtok_single(NULL, "*)");
            
            if (value != NULL) {
              //ESP_LOGI("data", "%s=[%s]", obis_code, value);
              parseRow(&parsed, obis_code, value);
              publish=true;
            }
            obis_code = strtok_single(NULL, "(");
          }
         
        }

        // clean buffer
        memset(buffer, 0, BUF_SIZE - 1);

      }

      if (publish == true) {
        ESP_LOGD("readTelegram", "Publishing sensor data");
        publishSensors(&parsed);
      }
    }

};
