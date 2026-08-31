# SensBlue Monarch — Pinout

Hardware revision 2.41. All defines live in `src/SensBlueMonarchPins.h`.

## ESP32 direct GPIOs

| Function | ESP32 GPIO | Define | Notes |
|---|---|---|---|
| USB TX (Serial) | IO1 | — | via CP2102N |
| USB RX (Serial) | IO3 | — | via CP2102N |
| I²C SDA | IO21 | `SB_SDA` | expander, EEPROM, BME280, OLED |
| I²C SCL | IO22 | `SB_SCL` | idem |
| SPI MOSI | IO13 | `SB_SPI_MOSI` | piggyback only |
| SPI MISO | IO12 | `SB_SPI_MISO` | piggyback only. **Strapping pin** |
| SPI SCK  | IO14 | `SB_SPI_SCK` | piggyback only |
| SPI SS   | IO5  | `SB_SPI_SS`  | piggyback only. **Strapping pin** |
| UART1 TX (BG95) | IO33 | `SB_SERIAL1_TX` | via level shifter |
| UART1 RX (BG95) | IO25 | `SB_SERIAL1_RX` | via level shifter |
| UART2 TX (mux) | IO16 | `SB_SERIAL2_TX` | RS485 or piggyback |
| UART2 RX (mux) | IO17 | `SB_SERIAL2_RX` | RS485 or piggyback |
| RS485 DE/!RE | IO18 | `SB_RS485_DE` | HIGH = TX, LOW = RX |
| DIG_IN_1 | IO26 | `SB_DIG_IN_1` | opto-coupled, active LOW at pin |
| DIG_IN_2 | IO15 | `SB_DIG_IN_2` | opto-coupled. **Strapping pin** |
| VCC_3_EN | IO2  | `SB_VCC_3_EN` | peripherals rail. **Strapping pin** |
| Button (BTN) | IO4 | `SB_BTN` | pressed = LOW |
| BOOT | IO0 | — | shared with CP2102N RTS |
| Analog IN 1 (raw) | IO39 | `SB_ANALOG_IN_1_RAW` | shared mux |
| Analog IN 2 (raw) | IO35 | `SB_ANALOG_IN_2_RAW` | shared mux |
| VBAT monitor | IO34 | `SB_VBAT_ADC` | via divider (gated by VBAT_2_EN) |
| PV / solar monitor | IO36 | `SB_PV_ADC` | via divider |
| POUT feedback | IO32 | `SB_POUT_ADC` | switched output rail |
| VUSB monitor | IO23 | `SB_VUSB_ADC` | |

Strapping-pin notes:
- IO0 (BOOT), IO2, IO5, IO12, IO15 must satisfy the ESP32 strapping
  requirements at reset. In this board they are handled by the CP2102N
  auto-boot circuit and pull networks.

## PCAL6416A expander (I²C 0x20)

Access exclusively via the `SensBlueMonarch` library.

| Expander bit | Define | Signal | Notes |
|---|---|---|---|
| P0_0 | — | — | reserved |
| P0_1 | `SB_EXP_DIG_OUT_EN_1` | DIG_OUT_EN_1 | 1 = OUT1 conducting |
| P0_2 | `SB_EXP_DIG_OUT_EN_2` | DIG_OUT_EN_2 | 1 = OUT2 conducting |
| P0_3 | `SB_EXP_BOOST_EN` | BOOST_EN | +12 V boost for 4-20 mA loop |
| P0_4 | `SB_EXP_VBAT_2_EN` | VBAT_2_EN | enables VBAT divider bias |
| P0_5 | `SB_EXP_POWERKEY` | POWERKEY | BG95 modem PWRKEY |
| P0_6 | `SB_EXP_VCC_2_EN` | VCC_2_EN | secondary rail |
| P0_7 | `SB_EXP_WP` | WP | EEPROM write-protect |
| P1_0 | `SB_EXP_P1_0` | IOEXP_P1_0 | piggyback |
| P1_1 | `SB_EXP_P1_1` | IOEXP_P1_1 | piggyback |
| P1_2 | — | — | reserved |
| P1_3 | `SB_EXP_INPUT_IV_SEL` | INPUT_IV_SEL | 0 = 0-10 V, 1 = 4-20 mA |
| P1_4 | `SB_EXP_USART_SEL` | USART_SEL | 0 = RS485, 1 = piggyback |
| P1_5 | `SB_EXP_LED_B` | LED_B | active LOW at expander |
| P1_6 | `SB_EXP_LED_G` | LED_G | active LOW at expander |
| P1_7 | `SB_EXP_LED_R` | LED_R | active LOW at expander |

`!INT` and `!RESET` of the PCAL6416A are pulled high by R53 and not
wired to the ESP32. Interrupt-driven reads are not available.

## I²C addresses (defaults)

| Address | Define | Device |
|---|---|---|
| 0x20 | `SB_EXP_ADDR` | PCAL6416A expander |
| 0x50 | `SB_I2C_EEPROM_ADDR` | AT24C02 |
| 0x76 | `SB_I2C_BME280_ADDR` | BME280 (SDO tied low — verify) |
| 0x3C / 0x3D | — | OLED (P2 header, module-dependent) |
