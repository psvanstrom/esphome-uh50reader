#include "esphome.h"

#define BUF_SIZE 100
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

class UH50Reader : public Component {
  const char* DELIMITERS = "(*";
  UARTDevice uart_in;
  UARTDevice uart_out;
  unsigned long timeLastRun;
  char buffer[BUF_SIZE];

  public:
    Sensor *cumulativeActiveImport = new Sensor();
    Sensor *cumulativeVolume = new Sensor();

    UH50Reader(UARTComponent *uart_in, UARTComponent *uart_out) : uart_in(uart_in), uart_out(uart_out) { }

    void setup() override {
      sendDataCmd();
      timeLastRun = millis() - (WAIT_TIME - 1) * 60000;
    }

    void loop() override {
      if (millis() - timeLastRun > WAIT_TIME * 60000) {
        sendDataCmd();
        readTelegram();
        timeLastRun = millis();
      }
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
      
      // fast forward until we find the STX byte (start-of-text)
      byte b = 0x00;
      while (uart_in.available() && b != 0x02) {
        b = uart_in.read();
      }

      while (uart_in.available()) {
        int len = uart_in.readBytesUntil('\n', buffer, BUF_SIZE);

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
            }
            obis_code = strtok_single(NULL, "(");
          }
         
        }

        // clean buffer
        memset(buffer, 0, BUF_SIZE - 1);
      }
    }

};