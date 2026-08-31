# SensBlueMonarch — Arduino library

Companion library for the **ATRONIA SensBlue Monarch** IoT controller
board (ESP32-WROOM-32E-N8, hardware revision 2.41).

Provides Arduino-style access to the on-board peripherals that sit
behind the PCAL6416A I²C expander:

- RGB status LED
- 2× isolated digital outputs (high-side)
- 2× isolated digital inputs (opto-coupled)
- 2× analog inputs, mode-switchable (0-10 V / 4-20 mA)
- RS485 direction control (ISL83483)
- UART2 mux (RS485 ↔ piggyback header)
- BG95 modem power sequencing
- Rail enables (VCC_2, VCC_3, boost, VBAT_2)
- Battery / panel / USB / output voltage monitors
- User button, EEPROM write-protect, piggyback GPIOs

## Requirements

- **ESP32 core by Espressif Systems** installed via the standard
  Arduino Board Manager URL:
  ```
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  ```
- A board of the ESP32 family selected in Arduino IDE (for example,
  *ESP32 Dev Module* while the SensBlue Monarch variant is not yet
  merged into the Espressif core).

## Install

### Option 1 — Library Manager (once approved by Arduino)

*In progress.* When the library is accepted:

1. Arduino IDE → **Tools → Manage Libraries…**
2. Search for `SensBlueMonarch`.
3. **Install**.

### Option 2 — ZIP install (available now)

1. Download the latest ZIP from
   [Releases](https://github.com/sensblue-monarch/SensBlueMonarch/releases).
2. Arduino IDE → **Sketch → Include Library → Add .ZIP Library…**
3. Pick the ZIP.

The examples appear under **File → Examples → SensBlueMonarch**.

## Quick start

```cpp
#include <SensBlueMonarch.h>

void setup() {
    Serial.begin(115200);
    if (!SensBlueMonarch.begin()) {
        Serial.println("Expander not responding.");
        while (true) delay(1000);
    }

    SensBlueMonarch.setRGB(true, false, false);         // red on
    SensBlueMonarch.setDigitalOutput(1, true);          // OUT1 on
    bool in = SensBlueMonarch.readDigitalInput(2);      // IN2

    SensBlueMonarch.setAnalogInputMode(SB_ANALOG_MODE_4_20MA);
    float mA = SensBlueMonarch.readAnalog(1);           // channel 1, mA

    SensBlueMonarch.selectUart2(SB_UART2_RS485);
    Serial2.begin(9600, SERIAL_8N1, SB_SERIAL2_RX, SB_SERIAL2_TX);
    SensBlueMonarch.rs485BeginTx();
    Serial2.print("hello");
    Serial2.flush();
    SensBlueMonarch.rs485EndTx();

    SensBlueMonarch.modemPowerOn();
    // → Serial1.begin(115200, SERIAL_8N1, SB_SERIAL1_RX, SB_SERIAL1_TX);
    // → then TinyGSM or raw AT commands
}
```

## Board selection

Until the ATRONIA SensBlue Monarch is a first-class variant in the
Espressif `arduino-esp32` core (planned via upstream PR), select
**ESP32 Dev Module** as the target board. The library uses raw GPIO
numbers (documented in `src/SensBlueMonarchPins.h`), so it works with
any generic ESP32 board configuration.

Recommended Arduino IDE settings for the Monarch:

| Setting | Value |
|---|---|
| Board | ESP32 Dev Module |
| Flash Size | 8MB (64Mb) |
| Flash Mode | DIO |
| Flash Frequency | 80 MHz |
| CPU Frequency | 240 MHz |
| Partition Scheme | Default 8MB with spiffs |
| Upload Speed | 921600 |

## Full pinout

See [`docs/pinout.md`](docs/pinout.md).

## Known caveats

- **PCAL6416A `!INT` and `!RESET` are not wired to the ESP32.** No
  interrupt-driven reads from the expander; polling only. Hardware
  reset of the expander requires a power cycle.
- **The analog input mode is shared between both channels.** You
  cannot read one channel as 0-10 V and the other as 4-20 mA
  simultaneously.
- **Divider values** in `SensBlueMonarchPins.h` were read from the
  schematic. Verify against your populated board before relying on
  the returned engineering units; the ESP32 ADCs are non-linear and
  this library does no calibration.
- **`modemPowerOn/Off` polarity** assumes the expander bit driving
  `POWERKEY` pulls the modem `PWRKEY` to ground when the bit is high.
  Confirm on the bench for your hardware population before shipping.
- **UART2 mux is exclusive:** RS485 xor piggyback.
- **The +12 V boost is enabled automatically** when you switch to
  4-20 mA mode. Disable it (`enableBoost(false)`) to save power in
  low-power states.

## Licence

MIT. See [`LICENSE`](LICENSE).
