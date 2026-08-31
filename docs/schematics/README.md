# Schematics — ATRONIA SensBlue Monarch (HW rev 2.41)

Project: **P225201 – ATRONIA – SensBlue**

Source: `Schematic_Prints.PDF` exported from Altium, split into
one JPG per sheet (1316 × 924, ~100 KB each).

| Sheet | File | Description |
|---|---|---|
| 1 | [01_RS485.jpg](01_RS485.jpg) | RS485 transceiver (ISL83483) and DE/!RE control |
| 2 | [02_Analog_Inputs.jpg](02_Analog_Inputs.jpg) | 0-10 V / 4-20 mA input front-end and mux (U15) |
| 3 | [03_CPU.jpg](03_CPU.jpg) | ESP32-WROOM-32E-N8 and immediate periphery |
| 4 | [04_Dig_Inputs.jpg](04_Dig_Inputs.jpg) | Isolated digital inputs (EL357N opto-couplers) |
| 5 | [05_Dig_Outputs.jpg](05_Dig_Outputs.jpg) | Isolated digital outputs (high-side drivers) |
| 6 | [06_HMI.jpg](06_HMI.jpg) | User button and RGB LED |
| 7 | [07_Overview.jpg](07_Overview.jpg) | Hierarchical block diagram and changelog |
| 8 | [08_Peripherals.jpg](08_Peripherals.jpg) | PCAL6416A expander, BME280, AT24C02, OLED header, piggyback |
| 9 | [09_Power_Supply.jpg](09_Power_Supply.jpg) | LM66200 ideal diode, buck-boost, LDOs |
| 10 | [10_RF_Modem.jpg](10_RF_Modem.jpg) | Quectel BG95-M3 modem, SIM holder, level shifters, GNSS |
| 11 | [11_USB.jpg](11_USB.jpg) | CP2102N USB-UART bridge and auto-reset/boot |

## Licence

The schematics are released under the same MIT licence as this
repository. See [`../../LICENSE`](../../LICENSE).
