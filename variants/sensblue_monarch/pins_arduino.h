#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

// -----------------------------------------------------------------------------
// ATRONIA SensBlue Monarch — pin definitions
// ESP32-WROOM-32E-N8 (8 MB flash, no PSRAM)
// Ref: schematic P225201-ATRONIA-SensBlue rev 2.41
//
// IMPORTANT
//   Many "features" of the board (LEDs, digital outputs, rail enables,
//   UART mux, analog input mode, modem POWERKEY) are NOT direct GPIOs.
//   They are controlled through the on-board PCAL6416A I2C expander at
//   address 0x20. Use the SensBlueMonarch library to drive them.
//   The names below are provided as reference constants; do NOT pass
//   them to digitalWrite() unless explicitly documented as a GPIO.
// -----------------------------------------------------------------------------

#define USB_VID           0x10C4   // Silicon Labs (CP2102N) — default
#define USB_PID           0xEA60   // change if CP2102N EEPROM is customised

// -----------------------------------------------------------------------------
// Direct ESP32 GPIOs
// -----------------------------------------------------------------------------

// User button (also acts as boot-mode selector via CP2102N/DTR/RTS network)
static const uint8_t BTN         = 4;

// I2C bus (peripherals, expander, EEPROM, BME280, OLED header)
static const uint8_t SDA         = 21;
static const uint8_t SCL         = 22;

// SPI bus (piggyback header only — not used by any on-board device in rev 2.41)
static const uint8_t MOSI        = 13;
static const uint8_t MISO        = 12;   // strapping pin
static const uint8_t SCK         = 14;
static const uint8_t SS          = 5;    // strapping pin, reserved for piggyback CS

// UARTs
//   Serial   → UART0, USB via CP2102N (TX=IO1, RX=IO3)
//   Serial1  → UART1, Quectel BG95 modem (TX=IO33, RX=IO25)
//              Wired through SN74LVC1T45 level shifters. Always available.
//   Serial2  → UART2, muxed by USART_SEL on the expander between:
//                • RS485 transceiver (ISL83483)  — USART_SEL = 0
//                • Piggyback expansion header    — USART_SEL = 1
//              Same GPIOs in both cases: TX=IO16, RX=IO17.
#define PIN_SERIAL1_TX   33
#define PIN_SERIAL1_RX   25
#define PIN_SERIAL2_TX   16
#define PIN_SERIAL2_RX   17

// RS485 driver enable (DE/!RE tied together). Direct GPIO.
static const uint8_t RS485_DE    = 18;

// Isolated digital inputs (opto-coupled, EL357N). Active LOW at ESP32 pin.
static const uint8_t DIG_IN_1    = 26;
static const uint8_t DIG_IN_2    = 15;   // strapping pin

// Direct rail control
//   VCC_3_EN gates the LDO that feeds the on-board 3.3 V peripherals bus
//   (BME280, EEPROM, OLED, piggyback). Not to be confused with the CPU rail.
static const uint8_t VCC_3_EN    = 2;    // strapping pin

// Analog inputs — raw ADC pins.
//   ANALOG_IN_1_RAW / ANALOG_IN_2_RAW share their physical connector with a
//   mux (U15) selected by INPUT_IV_SEL on the expander:
//       INPUT_IV_SEL = 0  → 0-10 V mode  → read ANALOG_IN_x_V
//       INPUT_IV_SEL = 1  → 4-20 mA mode → read ANALOG_IN_x_I
//   In the schematic both modes share the ADC pin (via the switch).
//   The library exposes readAnalog(ch) that returns the correct unit.
static const uint8_t ANALOG_IN_1_RAW = 39;   // GPIO39 / VN
static const uint8_t ANALOG_IN_2_RAW = 35;   // GPIO35

// On-board voltage monitors
static const uint8_t VBAT_ADC    = 34;
static const uint8_t PV_ADC      = 36;   // GPIO36 / VP  — solar panel
static const uint8_t POUT_ADC    = 32;   // output rail feedback
static const uint8_t VUSB_ADC    = 23;

// -----------------------------------------------------------------------------
// I2C expander (PCAL6416A) — bit positions
//   Access exclusively through the SensBlueMonarch library.
//   These constants are informational.
// -----------------------------------------------------------------------------
#define SENSBLUE_EXP_ADDR         0x20

#define SENSBLUE_EXP_DIG_OUT_EN_1 0x01   // P0_1
#define SENSBLUE_EXP_DIG_OUT_EN_2 0x02   // P0_2
#define SENSBLUE_EXP_BOOST_EN     0x03   // P0_3  (+12 V boost for 4-20 mA loop)
#define SENSBLUE_EXP_VBAT_2_EN    0x04   // P0_4
#define SENSBLUE_EXP_POWERKEY     0x05   // P0_5  (BG95 PWRKEY, active pulse)
#define SENSBLUE_EXP_VCC_2_EN     0x06   // P0_6
#define SENSBLUE_EXP_WP           0x07   // P0_7  (EEPROM write-protect)
#define SENSBLUE_EXP_P1_0         0x08   // P1_0  (piggyback IO)
#define SENSBLUE_EXP_P1_1         0x09   // P1_1  (piggyback IO)
#define SENSBLUE_EXP_INPUT_IV_SEL 0x0B   // P1_3
#define SENSBLUE_EXP_USART_SEL    0x0C   // P1_4
#define SENSBLUE_EXP_LED_B        0x0D   // P1_5
#define SENSBLUE_EXP_LED_G        0x0E   // P1_6
#define SENSBLUE_EXP_LED_R        0x0F   // P1_7

// -----------------------------------------------------------------------------
// On-board I2C peripheral addresses (defaults)
// -----------------------------------------------------------------------------
#define SENSBLUE_I2C_EEPROM_ADDR  0x50   // AT24C02
#define SENSBLUE_I2C_BME280_ADDR  0x76   // SDO tied low — confirm on hardware

// -----------------------------------------------------------------------------
// Analog input divider constants (from schematic)
//   0-10 V branch:  Vin → R73/R75 divider (10 k / 2.49 k) → ADC
//                   Vadc = Vin * 2490 / (10000 + 2490) ≈ Vin * 0.1993
//   4-20 mA branch: Iin → 100 Ω shunt (R70/R72) → ADC
//                   Vadc = Iin * 100 Ω  →  20 mA yields 2.0 V
// The library uses these to convert raw counts back to engineering units.
// -----------------------------------------------------------------------------
#define SENSBLUE_ADC_VREF_MV       3300
#define SENSBLUE_ADC_MAX_COUNT     4095
#define SENSBLUE_010V_DIV_NUM      2490
#define SENSBLUE_010V_DIV_DEN      12490   // 10000 + 2490
#define SENSBLUE_420MA_SHUNT_OHM   100

// VBAT monitor divider: R14/R19 = 10 k / 10 k (not read from schematic,
// treated as 2:1 for now — verify on the physical board).
#define SENSBLUE_VBAT_DIV_NUM      1
#define SENSBLUE_VBAT_DIV_DEN      2

#endif // Pins_Arduino_h
