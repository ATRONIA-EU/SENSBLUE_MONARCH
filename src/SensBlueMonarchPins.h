/*
 * SensBlueMonarchPins.h — GPIO and expander pin definitions for the
 * ATRONIA SensBlue Monarch board (ESP32-WROOM-32E-N8, hardware rev 2.41).
 *
 * Copyright (c) 2026 ATRONIA, Lda. MIT licence.
 */

#ifndef SENSBLUE_MONARCH_PINS_H
#define SENSBLUE_MONARCH_PINS_H

#include <stdint.h>

// -----------------------------------------------------------------------------
// Direct ESP32 GPIOs
// -----------------------------------------------------------------------------

#define SB_BTN                 4

// I²C bus (expander, BME280, EEPROM, OLED header, piggyback)
#define SB_SDA                 21
#define SB_SCL                 22

// SPI bus (piggyback header)
#define SB_SPI_MOSI            13
#define SB_SPI_MISO            12
#define SB_SPI_SCK             14
#define SB_SPI_SS              5

// UARTs
//   Serial   → UART0, USB via CP2102N (TX=IO1, RX=IO3)
//   Serial1  → UART1, Quectel BG95 modem.
//              Net names in schematic use MODEM's perspective:
//                TXD_GSM (modem TX) is on ESP32 IO33 → ESP32 RX
//                RXD_GSM (modem RX) is on ESP32 IO25 → ESP32 TX
//   Serial2  → UART2, muxed by USART_SEL on the expander between:
//                • RS485 transceiver (ISL83483)  — USART_SEL = 0
//                • BG95 GNSS via GPS_EN path     — USART_SEL = 1
//              Same GPIOs in both cases: TX=IO16, RX=IO17.
#define SB_SERIAL1_TX          25
#define SB_SERIAL1_RX          33
#define SB_SERIAL2_TX          17
#define SB_SERIAL2_RX          16

#define SB_RS485_DE            18

#define SB_DIG_IN_1            26
#define SB_DIG_IN_2            15

#define SB_VCC_3_EN            2

#define SB_ANALOG_IN_1_RAW     39
#define SB_ANALOG_IN_2_RAW     35

#define SB_VBAT_ADC            34
#define SB_PV_ADC              36
#define SB_POUT_ADC            32
#define SB_VUSB_ADC            23

// -----------------------------------------------------------------------------
// PCAL6416A I²C expander — bit positions
// -----------------------------------------------------------------------------
#define SB_EXP_ADDR            0x20

#define SB_EXP_GPS_EN          0x00
#define SB_EXP_DIG_OUT_EN_1    0x01
#define SB_EXP_DIG_OUT_EN_2    0x02
#define SB_EXP_BOOST_EN        0x03
#define SB_EXP_VBAT_2_EN       0x04
#define SB_EXP_POWERKEY        0x05
#define SB_EXP_VCC_2_EN        0x06
#define SB_EXP_P1_0            0x08
#define SB_EXP_P1_1            0x09
#define SB_EXP_INPUT_IV_SEL    0x0B
#define SB_EXP_USART_SEL       0x0C
#define SB_EXP_LED_B           0x0D
#define SB_EXP_LED_G           0x0E
#define SB_EXP_LED_R           0x0F

// -----------------------------------------------------------------------------
// On-board I²C peripheral addresses (defaults, hardware 2.41)
// -----------------------------------------------------------------------------
#define SB_I2C_EEPROM_ADDR     0x50
#define SB_I2C_BME280_ADDR     0x77
#define SB_I2C_OLED_ADDR       0x3C

#define SB_EEPROM_SIZE_BYTES   256
#define SB_EEPROM_PAGE_BYTES   8

#define SB_OLED_WIDTH          128
#define SB_OLED_HEIGHT         32
#define SB_OLED_RESET_PIN      -1

// -----------------------------------------------------------------------------
// Analog input divider constants
// -----------------------------------------------------------------------------
#define SB_ADC_VREF_MV         3300
#define SB_ADC_MAX_COUNT       4095
#define SB_010V_DIV_NUM        2490
#define SB_010V_DIV_DEN        12490
#define SB_420MA_SHUNT_OHM     100

#define SB_VBAT_DIV_NUM        1
#define SB_VBAT_DIV_DEN        2

#endif
