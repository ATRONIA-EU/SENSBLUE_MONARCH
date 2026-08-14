# ATRONIA SensBlue Monarch — Arduino support

Arduino IDE board support package and companion library for the
**ATRONIA SensBlue Monarch** IoT controller (ESP32-WROOM-32E-N8,
hardware revision 2.41).

- ESP32-WROOM-32E-N8 (8 MB flash, no PSRAM), 240 MHz
- USB programming via CP2102N (auto-reset / auto-boot)
- On-board Quectel BG95-M3 modem (LTE-M / NB-IoT + GNSS)
- BME280 environmental sensor, AT24C02 EEPROM, OLED header
- RS485 (ISL83483), 2× isolated digital inputs, 2× high-side digital outputs
- 2× analog inputs, mode-switchable (0-10 V or 4-20 mA)
- Battery-backed via LM66200 ideal-diode, USB / solar / VEXT sources
- Piggyback expansion header (SPI + I²C + spare GPIOs)

## Install

1. Open the Arduino IDE, go to **File → Preferences**.
2. In **Additional Board Manager URLs**, add:

   ```
   https://raw.githubusercontent.com/sensblue-monarch/arduino-monarch/main/package_sensblue_index.json
   ```

3. Open **Tools → Board → Boards Manager**, search for **SensBlue** and
   install **ATRONIA SensBlue Monarch (ESP32)**. The required toolchain
   is pulled automatically from the Espressif `esp32` package.
4. Select **Tools → Board → ATRONIA SensBlue Monarch**.
5. Plug the board over USB. On Linux and macOS a `/dev/ttyUSB*` (or
   `/dev/cu.SLAB_USBtoUART`) port appears. On Windows install the
   Silicon Labs CP210x VCP driver first.
6. **File → Examples → SensBlueMonarch** → try `01_Blink_RGB`.

## Repository layout

```
.
├── boards_snippet.txt                 # boards.txt entries appended to upstream
├── package_sensblue_index.json        # Board Manager index
├── variants/sensblue_monarch/         # variant with pins_arduino.h
├── libraries/SensBlueMonarch/         # companion library + examples
├── tools/build_release.sh             # release automation
└── docs/pinout.md                     # human-readable pin map
```

## Publishing a new release

The build script wraps a pinned upstream `arduino-esp32`, injects our
variant and boards.txt entries, produces a `.tar.bz2` release asset and
updates `package_sensblue_index.json` with the new URL, size and SHA-256:

```bash
./tools/build_release.sh 0.1.0 3.3.2   # <sensblue version> <arduino-esp32 ref>
```

Then:

1. `git commit` the updated `package_sensblue_index.json`.
2. `git tag v0.1.0 && git push --tags`.
3. Create a GitHub Release for the tag and upload
   `dist/sensblue-monarch-0.1.0.tar.bz2` as the release asset.

Whenever the Espressif `arduino-esp32` core publishes a new version we
consider stable, bump the `<upstream_ref>` argument and cut a new
release.

## Board API — quick reference

```cpp
#include <SensBlueMonarch.h>

void setup() {
    SensBlueMonarch.begin();                            // I2C + expander init

    SensBlueMonarch.setRGB(true, false, false);         // red on
    SensBlueMonarch.setDigitalOutput(1, true);          // isolated OUT1 on
    bool in = SensBlueMonarch.readDigitalInput(2);      // isolated IN2

    SensBlueMonarch.setAnalogInputMode(ANALOG_MODE_4_20MA);
    float mA = SensBlueMonarch.readAnalog(1);           // channel 1, mA

    SensBlueMonarch.selectUart2(UART2_RS485);
    Serial2.begin(9600, SERIAL_8N1, PIN_SERIAL2_RX, PIN_SERIAL2_TX);
    SensBlueMonarch.rs485BeginTx();
    Serial2.print("hello");
    Serial2.flush();
    SensBlueMonarch.rs485EndTx();

    SensBlueMonarch.modemPowerOn();                     // BG95 boot pulse
    // → then Serial1.begin(115200, SERIAL_8N1, PIN_SERIAL1_RX, PIN_SERIAL1_TX);
}
```

Full pinout in [`docs/pinout.md`](docs/pinout.md).

## Known caveats

- **PCAL6416A `!INT` and `!RESET` are not wired to the ESP32.** No
  interrupt-driven reads from the expander; polling only. Hardware reset
  of the expander requires a power cycle.
- **The analog input mode is shared between both channels.** You cannot
  read one channel as 0-10 V and the other as 4-20 mA simultaneously.
- **Divider values in `pins_arduino.h` were read from the schematic**
  (`R73/R75`, `R70/R72`, shunt 100 Ω). Verify against your populated
  board before relying on the returned engineering units; the ESP32
  ADCs are non-linear and this library does no calibration.
- **`modemPowerOn/Off` polarity** assumes the expander bit driving
  `POWERKEY` pulls the modem's `PWRKEY` to ground when the bit is high
  (there is a level shifter in between). Confirm on the bench for your
  hardware population before shipping firmware.
- **UART2 mux is exclusive:** RS485 xor piggyback. Switching mid-frame
  will corrupt whatever was being transmitted.
- **The +12 V boost is enabled automatically** when you switch to
  4-20 mA mode. That draws a few extra mA from the battery; disable
  it (`enableBoost(false)`) if you are in a low-power state.

## Licence

MIT. See [`LICENSE`](LICENSE).
