# Landis+Gyr UH50 (T550) Heat Meter Reader

An [ESPHome](https://esphome.io/) external component that reads a **Landis+Gyr Ultraheat
UH50 / T550** heat/cold meter through its optical interface and publishes the values to
Home Assistant.

The meter speaks the [IEC 62056-21](https://en.wikipedia.org/wiki/IEC_62056) (mode B)
protocol over the optical head. This component sends the data request, reads the response
telegram, parses the OBIS records and decodes the measured values into standard ESPHome
sensors. If the meter is battery powered, every request drains the battery a little, so
poll it sparingly.

> **Migrating from the old custom component?** ESPHome removed support for `custom`
> components. Replace the old `sensor: platform: custom` / `includes:` setup with the
> `external_components` + `platform: uh50` configuration shown under [Setup](#setup).

## Features

- Reads cumulative energy, cumulative volume, current power, flow rate and
  flow/return/difference temperatures (see below)
- Non-blocking read cycle — the main loop and Wi-Fi/API stay responsive while polling
- OBIS telegram parsing with bounds-checked field handling
- Optional on-demand read via a Home Assistant **button** or an ESPHome **action**

### Sensors

All sensors are optional — declare only the ones you want.

| Key                 | OBIS  | Unit  | Device class | State class        |
|---------------------|-------|-------|--------------|--------------------|
| `cumulative_energy` | 6.8   | MWh¹  | energy       | total_increasing   |
| `cumulative_volume` | 6.26  | m³    | volume       | total_increasing   |
| `current_power`     | 6.4   | kW    | power        | measurement        |
| `flow_rate`         | 6.27  | m³/h  | –            | measurement        |
| `flow_temp`         | 6.29  | °C    | temperature  | measurement        |
| `return_temp`       | 6.28  | °C    | temperature  | measurement        |
| `diff_temp`         | 6.30  | °C    | temperature  | measurement        |

## Hardware

- An **ESP8266** board with **two hardware UARTs** — the reference configuration uses a
  **NodeMCU** (ESP-12). A Wemos/LOLIN **D1 mini** also works.
- An **optical IEC 62056-21 read head** (an "IR eye") with TTL-level TX/RX, positioned
  over the meter's optical port. The one used here was built from the
  [hal9k Kamstrup optical eye](https://wiki.hal9k.dk/projects/kamstrup) kit (enclosure,
  PCB schematics and component list are published there).

### Wiring

The protocol sends the request at **300 baud** and receives the response at **2400 baud**,
so both hardware UARTs of the ESP8266 are used: UART0 RX (GPIO3) reads the meter, and
UART1 TX (GPIO2 / D4) sends the request.

| Optical read head    | NodeMCU pin  |
|----------------------|--------------|
| TX (meter → ESP)     | GPIO3 (RX)   |
| RX (ESP → meter)     | GPIO2 (D4)   |
| VCC                  | 3.3V         |
| GND                  | GND          |

> **Note:** TX on the read head goes to **RX** on the ESP and vice-versa. Serial logging
> should be disabled (`logger:` with `baud_rate: 0`) so console output doesn't collide with
> meter communication — view logs over the network instead.

## Requirements

- ESPHome (developed and tested with `2026.6.4`). The component auto-adapts to the
  `register_action(synchronous=…)` API that newer ESPHome versions introduced, so it also
  works on older releases.
- A controller with **two hardware UARTs** (the request and response run at different baud
  rates simultaneously).

## Setup

### 1. Add the external component

In your ESPHome YAML, pull the component straight from this repository:

```yaml
external_components:
  - source: github://psvanstrom/esphome-uh50reader@main
    components: [uh50]
```

### 2. Create `secrets.yaml`

Copy `secrets.yaml.example` to `secrets.yaml` and fill in your values. The reference
configuration uses these secrets:

```yaml
wifi_ssid: "your_wifi_ssid"
wifi_password: "your_wifi_password"
fallback_password: "fallback_hotspot_password"
ota_password: "your_ota_password"
encryption_key: "your_home_assistant_api_encryption_key"
```

The `fallback_password` and `ota_password` fields can be set to any password before the
initial upload.

### 3. Configure the UARTs and sensor

A minimal working configuration:

```yaml
logger:
  baud_rate: 0            # disable UART logging; view logs over the network

uart:
  - id: uart_in           # meter → ESP, response
    rx_pin: RX            # GPIO3
    baud_rate: 2400
    data_bits: 7
    parity: EVEN
    stop_bits: 2
    rx_buffer_size: 2048
  - id: uart_out          # ESP → meter, request
    tx_pin: D4            # GPIO2
    baud_rate: 300
    data_bits: 7
    parity: EVEN
    stop_bits: 2

sensor:
  - platform: uh50
    uart_id: uart_in
    uart_out_id: uart_out
    update_interval: 30min
    cumulative_energy:
      name: "UH50 Cumulative Active Import"
    cumulative_volume:
      name: "UH50 Cumulative Volume"
    current_power:
      name: "UH50 Current Power"
    flow_temp:
      name: "UH50 Flow Temperature"
    return_temp:
      name: "UH50 Return Temperature"
```

See [`uh50reader.yaml`](uh50reader.yaml) for a complete, ready-to-flash example including
Wi-Fi, the Home Assistant API, OTA and a read button.

### 4. Compile and flash

```bash
esphome run uh50reader.yaml
```

If everything works, Home Assistant will auto-detect the new ESPHome integration. Check the
logs over the network with `esphome logs uh50reader.yaml` (or use the ESPHome dashboard):

```
[I][uh50.reader:056]: Data cmd sent
[D][sensor:094]: 'UH50 Cumulative Active Import': Sending state 71.87300 MWh
[D][sensor:094]: 'UH50 Cumulative Volume': Sending state 1804.89 m3
```

## Configuration reference

The `uh50` platform extends the standard ESPHome polling-component and UART-device schemas.

| Option            | Type   | Default | Description                                                        |
|-------------------|--------|---------|--------------------------------------------------------------------|
| `uart_id`         | ID     | –       | RX UART bus that reads the meter response (2400 baud, 7E2).        |
| `uart_out_id`     | ID     | –       | **Required.** TX UART bus that sends the data request (300 baud, 7E2). |
| `update_interval` | time   | `30min` | How often to poll the meter. For battery meters keep this high.    |
| `read_button`     | button | –       | Creates a Home Assistant button that triggers an on-demand read.   |
| `cumulative_energy` … `diff_temp` | sensor | – | Optional sensors (see the table above). Each accepts the usual ESPHome sensor options (`name`, `id`, `filters`, `accuracy_decimals`, …). |

Both UARTs must use **7 data bits, even parity, 2 stop bits** — the RX bus at 2400 baud and
the TX bus at 300 baud.

### On-demand reads

Trigger a read from any ESPHome automation with the `uh50.read` action:

```yaml
sensor:
  - platform: uh50
    id: heat_meter
    # ...

# e.g. read whenever a template button is pressed
button:
  - platform: template
    name: "Read Heat Meter Now"
    on_press:
      - uh50.read: heat_meter
```

Or expose a dedicated button directly from the platform:

```yaml
sensor:
  - platform: uh50
    read_button:
      name: "Read Heat Meter"
```

## How it works

Each read cycle runs as a non-blocking state machine:

1. **Request** — send the IEC 62056-21 mode B wake-up sequence and data request
   (`/#!\r\n`, preceded by NUL wake-up bytes) on the 300-baud TX line.
2. **Read** — receive the response telegram (`STX … ! CR LF ETX BCC`) on the 2400-baud RX
   line, accumulating bytes until the ETX terminator (or a timeout).
3. **Parse & publish** — split the telegram into OBIS `code(value*unit)` records and map
   the recognised codes to sensors.

Recognised OBIS codes:

| OBIS   | Sensor              | Unit  |
|--------|---------------------|-------|
| `6.8`  | `cumulative_energy` | MWh¹  |
| `6.26` | `cumulative_volume` | m³    |
| `6.4`  | `current_power`     | kW    |
| `6.27` | `flow_rate`         | m³/h  |
| `6.29` | `flow_temp`         | °C    |
| `6.28` | `return_temp`       | °C    |
| `6.30` | `diff_temp`         | °C    |

¹ Meters configured to report energy in GJ, kWh or MJ are converted to MWh automatically.

> **Meter-specific decoding:** the OBIS codes emitted by the meter vary between firmware
> revisions. If a value never appears, enable debug logging and inspect the raw telegram —
> your meter may report that quantity under a different OBIS code than the ones above.

## Troubleshooting

- **No data / "Timed out waiting for telegram":** check that TX/RX aren't swapped, that the
  read head is seated correctly over the optical port, and that both UARTs are configured
  (2400 7E2 for RX, 300 7E2 for TX).
- **Some values never update:** your meter may emit those quantities under different OBIS
  codes; enable `logger:` at `DEBUG` and inspect the parsed telegram in the logs.
- **Nothing in the logs about the meter:** remember `logger:` must have `baud_rate: 0`;
  view logs over the network (`esphome logs uh50reader.yaml`) or the Home Assistant API,
  not the USB serial console.
- **Reads are infrequent by design:** the meter is polled on `update_interval` (30 min in
  the example) to spare the battery. Use the read button/action for an immediate reading.

## Large telegrams and buffer size

Some UH50 units emit a large amount of data around the readout — see
[issue #11](https://github.com/psvanstrom/esphome-uh50reader/issues/11). The reader skips
everything before the telegram's `STX` marker, so the meter's preamble no longer needs to
be buffered, and the telegram body is read into a bounds-checked **2500-byte** internal
buffer (larger readouts are safely truncated rather than overflowing memory).

If your meter has a particularly large readout you may still need to:

- Raise `rx_buffer_size` on the `uart_in` bus so bytes aren't dropped before the component
  reads them.
- Prefer an **ESP32** over an ESP8266 — it has far more RAM for the UART and internal
  buffers.
- If the readout itself exceeds 2500 bytes (trailing OBIS codes go missing), enlarge
  `BUF_SIZE` in `components/uh50/uh50.h`. This requires a local copy of the component —
  clone this repository and point `external_components` at it with
  `source: {type: local, path: components}` instead of the `github://` source.

## References

- UH50 overview:
  https://www.landisgyr.com/webfoo/wp-content/uploads/product-files/ConfigInstr_m_T550_UH50_en.pdf
- IEC 62056 telegram structure:
  http://manuals.lian98.biz/doc.en/html/u_iec62056_struct.htm

## License

[MIT](LICENSE) © Pär Svanström
