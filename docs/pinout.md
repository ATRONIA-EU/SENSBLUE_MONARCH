# SensBlue Monarch — Pinout

Hardware revision 2.41. Signals are grouped by function.

## ESP32 direct GPIOs

| Function | ESP32 GPIO | Notes |
|---|---|---|
| USB TX (Serial) | IO1 (TXD0) | via CP2102N |
| USB RX (Serial) | IO3 (RXD0) | via CP2102N |
| I²C SDA | IO21 | expander, EEPROM, BME280, OLED |
| I²C SCL | IO22 | idem |
| SPI MOSI | IO13 | piggyback only |
| SPI MISO | IO12 | piggyback only. **Strapping pin** |
| SPI SCK  | IO14 | piggyback only |
| SPI SS   | IO5  | piggyback only. **Strapping pin** |
| UART1 TX (BG95) | IO33 | via level shifter |
| UART1 RX (BG95) | IO25 | via level shifter |
| UART2 TX (mux) | IO16 | RS485 or piggyback |
| UART2 RX (mux) | IO17 | RS485 or piggyback |
| RS485 DE/!RE | IO18 | HIGH = TX, LOW = RX |
| DIG_IN_1 | IO26 | opto-coupled, active LOW at pin |
| DIG_IN_2 | IO15 | opto-coupled. **Strapping pin** |
| VCC_3_EN | IO2  | peripherals rail. **Strapping pin** |
| Button (BTN) | IO4 | pull-up internal, pressed = LOW |
| BOOT | IO0 | shared with CP2102N RTS via Q13 |
| Analog IN 1 (raw) | IO39 / VN | shared 0-10 V / 4-20 mA via mux |
| Analog IN 2 (raw) | IO35 | idem |
| VBAT monitor | IO34 | via divider (gated by VBAT_2_EN) |
| PV / solar monitor | IO36 / VP | via divider |
| POUT feedback | IO32 | switched output rail |
| VUSB monitor | IO23 | |

Strapping pin notes:
- IO0 (BOOT), IO2, IO5, IO12, IO15 must satisfy the ESP32 strapping
  requirements at reset. In this board they are handled by the CP2102N
  auto-boot circuit and pull networks; do not add pull-downs on IO12
  (would force 1.8 V flash mode) or drive IO0 low permanently.

## PCAL6416A expander (I²C 0x20)

Access exclusively via the `SensBlueMonarch` library.

| Expander bit | Signal | Direction | Notes |
|---|---|---|---|
| P0_0 | — | — | reserved / unused |
| P0_1 | DIG_OUT_EN_1 | out | 1 = OUT1 conducting |
| P0_2 | DIG_OUT_EN_2 | out | 1 = OUT2 conducting |
| P0_3 | BOOST_EN | out | +12 V boost for 4-20 mA loop |
| P0_4 | VBAT_2_EN | out | enables VBAT divider bias |
| P0_5 | POWERKEY | out | BG95 modem PWRKEY (level shifted) |
| P0_6 | VCC_2_EN | out | secondary rail (modem-side) |
| P0_7 | WP | out | EEPROM write-protect (1 = protected) |
| P1_0 | IOEXP_P1_0 | out | piggyback, general purpose |
| P1_1 | IOEXP_P1_1 | out | piggyback, general purpose |
| P1_2 | — | — | reserved |
| P1_3 | INPUT_IV_SEL | out | 0 = 0-10 V, 1 = 4-20 mA |
| P1_4 | USART_SEL | out | 0 = RS485, 1 = piggyback UART |
| P1_5 | LED_B | out | active LOW at expander |
| P1_6 | LED_G | out | active LOW at expander |
| P1_7 | LED_R | out | active LOW at expander |

`!INT` (pin 1) and `!RESET` (pin 3) of the PCAL6416A are pulled high by
R53 and **not wired to the ESP32**. There is no interrupt-driven read of
the expander and no software reset — a power cycle is the only way to
force the chip to POR state.

## I²C peripheral addresses (defaults)

| Address | Device | Notes |
|---|---|---|
| 0x20 | PCAL6416A expander | ADDR pin tied to GND |
| 0x50 | AT24C02 EEPROM | 256 bytes, A0/A1/A2 = GND |
| 0x76 | BME280 | SDO tied low. **Verify on hardware** |
| 0x3C / 0x3D | OLED (optional) | depends on OLED module used on the P2 header |

## Serial ports

| Handle | Peripheral | Baud (typical) | Notes |
|---|---|---|---|
| `Serial`  | USB CDC via CP2102N | 115 200 | programming + debug |
| `Serial1` | BG95 modem | 115 200 | AT commands, level shifted |
| `Serial2` | RS485 or piggyback (muxed) | app-specific | see `USART_SEL` |

BG95 GNSS output is available on the modem's dedicated GNSS UART, also
level-shifted to the ESP32; it shares the same conceptual "modem" bus.
Use the BG95's AT+QGPS commands to enable/disable and stream NMEA over
the same Serial1 by default (see Quectel documentation).
