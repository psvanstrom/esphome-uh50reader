# esphome-uh50reader

An [ESPHome](https://esphome.io/) **external component** for communicating with Landis+Gyr
T550 (UH50) heat/cold meters and reading usage data. The UH50 meter communicates over an
optical interface using the standardized IEC 62056-21 (mode B) protocol. If the meter is
battery powered, each request for data will drain the battery life, so read it sparingly.

> **Note:** ESPHome removed support for `custom` components. This project has been rewritten
> as an [`external_components`](https://esphome.io/components/external_components.html)
> package. If you are migrating from the old `sensor: platform: custom` / `includes:`
> setup, replace it with the `external_components` + `platform: uh50` configuration shown
> below.

## Features

- Reads cumulative energy, cumulative volume, current power, flow rate and
  flow/return/difference temperatures (see below)
- Non-blocking read cycle — the main loop and Wi-Fi/API stay responsive while polling
- Optional on-demand read via a Home Assistant **button** or an ESPHome **action**
  (`uh50.read`)

### Sensors

All sensors are optional — declare only the ones you want.

| Key                  | OBIS  | Unit  | Device class | State class        |
|----------------------|-------|-------|--------------|--------------------|
| `cumulative_energy`  | 6.8   | MWh   | energy       | total_increasing   |
| `cumulative_volume`  | 6.26  | m³    | volume       | total_increasing   |
| `current_power`      | 6.4   | kW    | power        | measurement        |
| `flow_rate`          | 6.27  | m³/h  | –            | measurement        |
| `flow_temp`          | 6.29  | °C    | temperature  | measurement        |
| `return_temp`        | 6.28  | °C    | temperature  | measurement        |
| `diff_temp`          | 6.30  | °C    | temperature  | measurement        |

## ESPHome version

The current version in `main` is written for the `external_components` API and is tested
against recent ESPHome releases (2024.x and newer). The old `custom` component API is no
longer supported by ESPHome.

## Hardware

The optical eye hardware I'm using was ordered as a kit from here:
https://wiki.hal9k.dk/projects/kamstrup — they no longer sell kits but provide all the
necessary information for printing the enclosure as well as PCB schematics and component
list. This optical eye is connected to a NodeMCU ESP-controller with the RX pin connected
to the RX pin (GPIO3) and the TX pin connected to the D4 pin (GPIO2) on the NodeMCU.

### UART

Since the protocol requires outgoing communication (TX) at 300 baud and the incoming
communication (RX) at 2400 baud, both hardware UARTs on the NodeMCU are used. The
component uses the RX UART (`uart_id`) to read the meter response and the TX UART
(`uart_out_id`) to send the data request. Other ESP8266 based controllers will work as
well, but make sure the controller supports two hardware UARTs and check which pins they
are located at.

### Parts

- 1 Optical eye, see recommendation above
- 1 ESP12 NodeMCU
- Wires
- Wall plug and USB cable to power the NodeMCU

## Installation

Create a companion `secrets.yaml` file next to `uh50reader.yaml` (see
`secrets.yaml.example`):

```yaml
wifi_ssid: <your wifi SSID>
wifi_password: <your wifi password>
fallback_password: <fallback AP password>
ota_password: <the OTA password>
encryption_key: <the Home Assistant API encryption key>
```

The `fallback_password` and `ota_password` fields can be set to any password before doing
the initial upload of the firmware.

The component is pulled in via `external_components`, so ESPHome downloads it automatically
at build time — you do **not** need to clone this repository:

```yaml
external_components:
  - source: github://psvanstrom/esphome-uh50reader@main
    components: [uh50]
```

Then flash the microcontroller:

- Install the `esphome` [command line tool](https://esphome.io/guides/getting_started_command_line.html)
- Plug the microcontroller into your USB port and run `esphome run uh50reader.yaml`
- Remove the USB connection and connect the microcontroller to the rest of the circuit
- If everything works, Home Assistant will auto-detect the new ESPHome integration

Check the logs with `esphome logs uh50reader.yaml` (or use the ESPHome dashboard):

```
[I][uh50.reader:056]: Data cmd sent
[D][sensor:094]: 'UH50 Cumulative Active Import': Sending state 71.87300 MWh
[D][sensor:094]: 'UH50 Cumulative Volume': Sending state 1804.89 m3
```

## Reading on demand

The meter is read automatically every `update_interval` (30 min by default). To trigger a
read on demand you can either:

- Add a `read_button:` to the sensor config (shown in `uh50reader.yaml`), which appears as
  a button entity in Home Assistant, or
- Call the `uh50.read` action from an ESPHome automation:

  ```yaml
  on_...:
    then:
      - uh50.read: uh50_reader   # the id of the uh50 sensor platform
  ```

## Local development

To build against a local checkout of this repository instead of GitHub, point
`external_components` at the local path:

```yaml
external_components:
  - source:
      type: local
      path: components
    components: [uh50]
```

## Build and install tips for Windows

Use the ESPHome docker image to build without installing the build tools:

```
docker run -p 6052:6052 -v C:\some-directory\esphome-uh50reader:/config -it esphome/esphome
```

Open a web browser to http://localhost:6052, click install and download the firmware using
the modern format, then flash it.

## Technical documentation

UH50 overview:
https://www.landisgyr.com/webfoo/wp-content/uploads/product-files/ConfigInstr_m_T550_UH50_en.pdf

IEC 62056 telegram structure:
http://manuals.lian98.biz/doc.en/html/u_iec62056_struct.htm
