/**
 * Test sketch for retrieving telegrams from the optical port of an L&G T550 meter
 * Connect LED RX to D5 and TX to D6
 */
#include <SoftwareSerial.h>


#define BAUD_RATE_INIT 300
#define BAUD_RATE_DATA 2400

#define RX_PIN D5
#define TX_PIN D6

SoftwareSerial swSer;

byte data_cmd[] = { 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x2F, 0x3F, 0x21, 0x0D, 0x0A
};

void setup() {
  Serial.begin(115200);
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);
  Serial.println("\nEnergy Meter test started");
}

void receive(long wait_time) {

  byte rxdata[50];  // buffer to hold received data
  unsigned long rxindex = 0;
  unsigned long starttime = millis();
  
  swSer.flush();  // flush serial buffer - might contain noise

  byte r;
  
  // loop until EOL received or timeout
  while(r != 0x0d){
    
    // handle rx timeout
    if(millis()-starttime > wait_time) {
      Serial.println("Timed out listening for data");
      return;
    }

    // handle incoming data
    if (swSer.available()) {

      // receive byte
      r = swSer.read();
      if(r != 0x02) {  // don't append if we see the start marker
        // append data
        rxdata[rxindex] = r;
        rxindex++; 
      }
      
    }
  }
  Serial.write(rxdata, rxindex);

}

void loop() {
  // send data command
  swSer.begin(BAUD_RATE_INIT, SWSERIAL_7E2, RX_PIN, TX_PIN, false);
  for (int i = 0; i < sizeof(data_cmd); i++) {
    swSer.write(data_cmd[i]);
  }

  Serial.println("\n------------------- telegram start --------------------");
  
  swSer.flush();
  swSer.begin(BAUD_RATE_DATA, SWSERIAL_7E2, RX_PIN, TX_PIN, false);
  receive(10000);

  Serial.println("------------------- telegram end --------------------\n");

  delay(5000);
}
