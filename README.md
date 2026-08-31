# SensBlueMonarch — Arduino library

Companion library for the **ATRONIA SensBlue Monarch** IoT controller
board (ESP32-WROOM-32E-N8, hardware revision 2.41).

Covers every on-board peripheral so the user can drop a couple of
lines in `setup()` and start prototyping without chasing datasheets
and I²C addresses:

- RGB status LED (via PCAL6416A expander)
- 2× isolated digital outputs (high-side)
- 2× isolated digital inputs (opto-coupled)
- 2× analog inputs, mode-switchable (0-10 V / 4-20 mA)
- RS485 direction control (ISL83483)
- UART2 mux (RS485 ↔ BG95 GNSS path)
- Quectel BG95 modem power sequencing (raw AT — use TinyGSM for stack)
- GNSS via BG95 built-in receiver (raw NMEA — use TinyGPSPlus to parse)
- Rail enables (VCC_2, VCC_3, boost, VBAT_2)
- Battery / panel / USB / output voltage monitors
- BME280 environmental sensor (T, H, P)
- SSD1306 OLED display, including a built-in status screen
- AT24C02 EEPROM (256 bytes, page-aware read/write)
- Battery state-of-charge estimation (Li-ion / LiFePO4)
- User button, piggyback GPIOs

## Requirements

- **ESP32 core by Espressif Systems** installed via the standard
  Arduino Board Manager URL:
  ```
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  ```
- **Board selected:** ESP32 Dev Module (while the SensBlue Monarch
  variant is not yet merged into the Espressif core). Recommended
  settings:

  | Setting | Value |
  |---|---|
  | Flash Size | 8MB (64Mb) |
  | Flash Mode | DIO |
  | Flash Frequency | 80 MHz |
  | CPU Frequency | 240 MHz |
  | Partition Scheme | Default 8MB with spiffs |
  | Upload Speed | 921600 |

## Install

### Option 1 — Library Manager (once approved by Arduino)

*In progress.* When accepted:

1. Arduino IDE → **Tools → Manage Libraries…**
2. Search for `SensBlueMonarch`.
3. **Install**. The Adafruit dependencies are pulled automatically.

### Option 2 — ZIP install

1. Download the latest ZIP from
   [Releases](https://github.com/sensblue-monarch/SensBlueMonarch/releases).
2. Arduino IDE → **Sketch → Include Library → Add .ZIP Library…**
3. Pick the ZIP.
4. Install the Adafruit dependencies manually via **Manage Libraries**:
   - `Adafruit BME280 Library`
   - `Adafruit SSD1306`
   - `Adafruit GFX Library`
   - `Adafruit Unified Sensor`
   - `Adafruit BusIO`

Examples appear under **File → Examples → SensBlueMonarch**.

## Quick start

```cpp
#include <SensBlueMonarch.h>

void setup() {
    Serial.begin(115200);

    SensBlueMonarch.begin();              // I2C + expander + safe defaults
    SensBlueMonarch.beginEnvSensor();     // BME280 (optional)
    SensBlueMonarch.beginDisplay();       // SSD1306 (optional)
    SensBlueMonarch.beginEeprom();        // AT24C02 (optional)
    SensBlueMonarch.beginBattery();       // battery estimator (optional)

    // Simple LED
    SensBlueMonarch.setRGB(false, true, false);   // green

    // Environmental readings
    Serial.printf("T=%.1f°C  RH=%.0f%%  P=%.0f hPa\n",
                  SensBlueMonarch.readTemperature(),
                  SensBlueMonarch.readHumidity(),
                  SensBlueMonarch.readPressure());

    // Battery
    Serial.printf("Battery %.0f %%\n",
                  SensBlueMonarch.readBatteryPercentage());

    // Draw the built-in status screen
    SensBlueMonarch.showStatusScreen();

    // For custom drawing, access the Adafruit_SSD1306 directly:
    SensBlueMonarch.display.setCursor(0, 56);
    SensBlueMonarch.display.print("Custom line");
    SensBlueMonarch.display.display();
}

void loop() {}
```

## API reference

All access goes through the singleton `SensBlueMonarch`. High-level
helpers cover the common cases; the underlying `Adafruit_BME280`,
`Adafruit_SSD1306` and `BatteryEstimator` instances are exposed as
public members (`.bme`, `.display`, `.battery`) for advanced use.

See the examples under `examples/` for concrete recipes:

| Example | Covers |
|---|---|
| 01_Blink_RGB | On-board RGB LED via expander |
| 02_Read_Button | User button on GPIO4 |
| 03_Digital_IO | Isolated digital I/O |
| 04_Analog_Inputs | 0-10 V / 4-20 mA inputs, mode switching |
| 05_RS485_Echo | Half-duplex RS485 echo |
| 06_Modem_Info | BG95 power-on and identity queries |
| 07_BME280_Read | Temperature / humidity / pressure |
| 08_OLED_Hello | Direct Adafruit_SSD1306 access |
| 09_OLED_StatusScreen | Built-in status screen |
| 10_EEPROM_Storage | Persistent boot counter in the AT24C02 |
| 11_GNSS_Read | Raw NMEA from the BG95 built-in receiver |
| 12_Battery_Percent | State-of-charge with the on-board algorithm |

Full pinout in [`docs/pinout.md`](docs/pinout.md). Board schematics (JPG, one per sheet) are in [`docs/schematics/`](docs/schematics/README.md).

## Known caveats

- **PCAL6416A `!INT` and `!RESET` are not wired** to the ESP32 on
  rev 2.41. Interrupt-driven reads from the expander are not
  available; polling only.
- **Analog input mode is shared** between both channels — the mux
  is board-wide, not per-channel.
- **Analog divider constants** in `SensBlueMonarchPins.h` were read
  from the schematic. Verify against your populated board before
  relying on the returned engineering units. The ESP32 ADCs are
  non-linear; no calibration is applied.
- **`modemPowerOn/Off` polarity** assumes the expander bit driving
  `POWERKEY` pulls the modem `PWRKEY` to ground when the bit is
  high. Confirm on the bench for your hardware population.
- **UART2 is shared** between RS485 and the BG95 GNSS receiver.
  `enableGNSS(true)` routes UART2 to GNSS; `enableGNSS(false)`
  restores RS485. Switching mid-frame corrupts data.
- **+12 V boost** is enabled automatically when switching to
  4-20 mA mode. Disable it (`enableBoost(false)`) to save power.
- **BatteryEstimator** stores state in ESP32 NVS (namespace
  `battery`). Call `SensBlueMonarch.battery.saveToNVS()` periodically
  in production.
- **EEPROM WP** — the write-protect resistor was removed on rev 2.41;
  the AT24C02 is always writable in software.

## Licence

MIT. See [`LICENSE`](LICENSE).
