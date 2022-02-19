# esphome-uh50reader
ESPHome custom component for communicating with Landis+Gyr T550 (UH50) heat/cold meters and reading usage data. The UH50 meter communicates over an optical interface using the standardized IEC 62056-21 protocol. If the meter is battery powered, each request for data will drain the battery life. 

## ESPHome version
The current version in main is tested with ESPHome version `2022.2.3`. Make sure your ESPHome version is up to date if you experience compile problems.

## Hardware
The optical eye hardware I'm using was ordered as a kit from here: https://wiki.hal9k.dk/projects/kamstrup, they no longer sell kits but provide all the necessary information for printing the enclosure as well as PCB schematics and component list. This optical eye is then connected to a NodeMCU ESP-controller with the RX pin connected to the RX pin (GPIO3) on the NodeMCU and the TX pin connected to the D4 pin (GPIO2) on the NodeMCU.

### UART
Since the protocol requires outgoing communication (TX) using 300 baud and the incoming communication (RX) using 2400 baud, both hardware UARTs on the NodeMCU are used. Other ESP8266 based controllers will work as well, but make sure that the controller support two hardware UARTs and check which pins they are located at.

### Parts
- 1 Optical eye, see recommendation above
- 1 ESP12 NodeMCU
- 1 Wires
- Wall plug and USB cable to power the NodeMCU

## Installation
Clone the repository and create a companion `secrets.yaml` file with the following fields:
```
wifi_ssid: <your wifi SSID>
wifi_password: <your wifi password>
fallback_password: <fallback AP password>
hass_api_password: <the Home Assistant API password>
ota_password: <The OTA password>
```
Make sure to place the `secrets.yaml` file in the root path of the cloned project. The `fallback_password` and `ota_password` fields can be set to any password before doing the initial upload of the firmware.

Prepare the microcontroller with ESPHome before you connect it to the circuit:
- Install the `esphome` [command line tool](https://esphome.io/guides/getting_started_command_line.html)
- Plug in the microcontroller to your USB port and run `esphome uh50reader.yaml run` to flash the firmware
- Remove the USB connection and connect the microcontroller to the rest of the circuit and plug it into the P1 port.
- If everything works, your Home Assistant will now auto detect your new ESPHome integration.

You can check the logs by issuing `esphome uh50reader.yaml logs` (or use the super awesome ESPHome dashboard available as a Hass.io add-on or standalone). The logs should output data similar at a certain interval (`WAIT_TIME`):
```
[01:09:42][I][cmd:097]: data cmd sent
[01:09:42][D][sensor:113]: 'Cumulative Active Import': Sending state 85548.00000 kWh with 2 decimals of accuracy
[01:09:42][D][sensor:113]: 'Cumulative Volume': Sending state 2144.70996 m3 with 6 decimals of accuracy
```

### Wait time
The current wait time before requesting a new reading is set to 10 minutes, this can be changed by updating this row in uh50reader.h:
```
#define WAIT_TIME 10
```

## Technical documentation
UH50 overview:
https://www.landisgyr.com/webfoo/wp-content/uploads/product-files/ConfigInstr_m_T550_UH50_en.pdf

IEC 62056 telegram structure:
http://manuals.lian98.biz/doc.en/html/u_iec62056_struct.htm
