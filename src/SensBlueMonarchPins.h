/*
 * SensBlueMonarchPins.h — GPIO and expander pin definitions for the
 * ATRONIA SensBlue Monarch board (ESP32-WROOM-32E-N8, hardware rev 2.41).
 *
 * When the ATRONIA SensBlue Monarch is officially added to the Espressif
 * arduino-esp32 core, these will become variant-provided defines and this
 * file may be simplified or deprecated.
 *
 * Copyright (c) 2026 ATRONIA, Lda. MIT licence.
 */

#ifndef SENSBLUE_MONARCH_PINS_H
#define SENSBLUE_MONARCH_PINS_H

#include <stdint.h>

// -----------------------------------------------------------------------------
// Direct ESP32 GPIOs
// -----------------------------------------------------------------------------

// User button (also acts as boot-mode selector via CP2102N/DTR/RTS network)
#define SB_BTN                 4

// I2C bus (peripherals, expander, EEPROM, BME280, OLED header)
#define SB_SDA                 21
#define SB_SCL                 22

// SPI bus (piggyback header only — not used by any on-board device in rev 2.41)
#define SB_SPI_MOSI            13
#define SB_SPI_MISO            12   // strapping pin
#define SB_SPI_SCK             14
#define SB_SPI_SS              5    // strapping pin, reserved for piggyback CS

// UARTs
//   Serial   → UART0, USB via CP2102N (TX=IO1, RX=IO3)
//   Serial1  → UART1, Quectel BG95 modem (TX=IO33, RX=IO25)
//              Wired through SN74LVC1T45 level shifters. Always available.
//   Serial2  → UART2, muxed by USART_SEL on the expander between:
//                • RS485 transceiver (ISL83483)  — USART_SEL = 0
//                • Piggyback expansion header    — USART_SEL = 1
//              Same GPIOs in both cases: TX=IO16, RX=IO17.
#define SB_SERIAL1_TX          33
#define SB_SERIAL1_RX          25
#define SB_SERIAL2_TX          16
#define SB_SERIAL2_RX          17

// RS485 driver enable (DE/!RE tied together). Direct GPIO.
#define SB_RS485_DE            18

// Isolated digital inputs (opto-coupled, EL357N). Active LOW at ESP32 pin.
#define SB_DIG_IN_1            26
#define SB_DIG_IN_2            15   // strapping pin

// Direct rail control
//   VCC_3_EN gates the LDO that feeds the on-board 3.3 V peripherals bus
//   (BME280, EEPROM, OLED, piggyback). Not to be confused with the CPU rail.
#define SB_VCC_3_EN            2    // strapping pin

// Analog inputs — raw ADC pins.
//   SB_ANALOG_IN_1_RAW / SB_ANALOG_IN_2_RAW share their physical connector
//   with a mux (U15) selected by INPUT_IV_SEL on the expander:
//       INPUT_IV_SEL = 0  → 0-10 V mode
//       INPUT_IV_SEL = 1  → 4-20 mA mode
//   The library exposes readAnalog(ch) that returns the correct unit.
#define SB_ANALOG_IN_1_RAW     39   // GPIO39 / VN
#define SB_ANALOG_IN_2_RAW     35   // GPIO35

// On-board voltage monitors
#define SB_VBAT_ADC            34
#define SB_PV_ADC              36   // GPIO36 / VP  — solar panel
#define SB_POUT_ADC            32   // output rail feedback
#define SB_VUSB_ADC            23

// -----------------------------------------------------------------------------
// I2C expander (PCAL6416A) — bit positions
//   Access exclusively through the SensBlueMonarch library.
//   These constants are informational.
// -----------------------------------------------------------------------------
#define SB_EXP_ADDR            0x20

#define SB_EXP_DIG_OUT_EN_1    0x01   // P0_1
#define SB_EXP_DIG_OUT_EN_2    0x02   // P0_2
#define SB_EXP_BOOST_EN        0x03   // P0_3  (+12 V boost for 4-20 mA loop)
#define SB_EXP_VBAT_2_EN       0x04   // P0_4
#define SB_EXP_POWERKEY        0x05   // P0_5  (BG95 PWRKEY, active pulse)
#define SB_EXP_VCC_2_EN        0x06   // P0_6
#define SB_EXP_WP              0x07   // P0_7  (EEPROM write-protect)
#define SB_EXP_P1_0            0x08   // P1_0  (piggyback IO)
#define SB_EXP_P1_1            0x09   // P1_1  (piggyback IO)
#define SB_EXP_INPUT_IV_SEL    0x0B   // P1_3
#define SB_EXP_USART_SEL       0x0C   // P1_4
#define SB_EXP_LED_B           0x0D   // P1_5
#define SB_EXP_LED_G           0x0E   // P1_6
#define SB_EXP_LED_R           0x0F   // P1_7

// -----------------------------------------------------------------------------
// On-board I2C peripheral addresses (defaults)
// -----------------------------------------------------------------------------
#define SB_I2C_EEPROM_ADDR     0x50   // AT24C02
#define SB_I2C_BME280_ADDR     0x76   // SDO tied low — confirm on hardware

// -----------------------------------------------------------------------------
// Analog input divider constants (from schematic)
//   0-10 V branch:  Vin → R73/R75 divider (10 k / 2.49 k) → ADC
//   4-20 mA branch: Iin → 100 Ω shunt (R70/R72) → ADC
// -----------------------------------------------------------------------------
#define SB_ADC_VREF_MV         3300
#define SB_ADC_MAX_COUNT       4095
#define SB_010V_DIV_NUM        2490
#define SB_010V_DIV_DEN        12490  // 10000 + 2490
#define SB_420MA_SHUNT_OHM     100

// VBAT monitor divider: assumed 2:1 — verify on the physical board.
#define SB_VBAT_DIV_NUM        1
#define SB_VBAT_DIV_DEN        2

#endif // SENSBLUE_MONARCH_PINS_H
