/*
 * SensBlueMonarch.h — high-level access to the on-board peripherals of
 * the ATRONIA SensBlue Monarch board (hardware revision 2.41).
 *
 * Copyright (c) 2026 ATRONIA, Lda. MIT licence.
 */

#ifndef SENSBLUE_MONARCH_H
#define SENSBLUE_MONARCH_H

#include <Arduino.h>
#include <Wire.h>

#include "SensBlueMonarchPins.h"
#include "BatteryEstimator/BatteryEstimator.h"

// The Adafruit sensor/display libraries and their transitive dependencies
// (Adafruit_Sensor, Adafruit_BusIO, Adafruit_GFX) are declared in
// library.properties and pulled by the Library Manager on install.
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

enum SensBlueLED {
    SB_LED_RED   = 0,
    SB_LED_GREEN = 1,
    SB_LED_BLUE  = 2
};

enum SensBlueAnalogMode {
    SB_ANALOG_MODE_0_10V   = 0,
    SB_ANALOG_MODE_4_20MA  = 1
};

enum SensBlueUartTarget {
    SB_UART2_RS485      = 0,
    SB_UART2_GNSS       = 1    // BG95 GNSS path (formerly named piggyback)
};

class SensBlueMonarchClass {
public:
    // ---- Instances exposed for advanced use ----
    // These are also driven by the high-level helpers below. Users who
    // need something the helpers don't cover can reach the underlying
    // library directly.
    Adafruit_BME280   bme;
    Adafruit_SSD1306  display;
    BatteryEstimator  battery;

    SensBlueMonarchClass()
      : display(SB_OLED_WIDTH, SB_OLED_HEIGHT, &Wire, SB_OLED_RESET_PIN),
        battery(BATTERY_TYPE_LIION) {}

    // ---- Core init ----
    // Sets up I²C, the on-board expander, discrete GPIOs. Does NOT touch
    // BME280, OLED, EEPROM or the modem — those are opt-in via their own
    // begin* methods so a user who doesn't need them pays nothing.
    bool begin(TwoWire &wire = Wire, uint32_t clockHz = 100000);

    // ---- LEDs ----
    void setLED(SensBlueLED colour, bool on);
    void setRGB(bool red, bool green, bool blue);
    void allLEDsOff();

    // ---- Isolated digital I/O ----
    void setDigitalOutput(uint8_t channel, bool state);
    bool readDigitalInput(uint8_t channel);

    // ---- Analog inputs ----
    void setAnalogInputMode(SensBlueAnalogMode mode);
    SensBlueAnalogMode getAnalogInputMode() const { return _analogMode; }
    float    readAnalog(uint8_t channel, uint8_t samples = 8);
    uint16_t readAnalogRaw(uint8_t channel);

    // ---- Power rails ----
    void enableVCC2(bool on);
    void enableVCC3(bool on);
    // 12 V boost — feeds the 4-20 mA current loop and is also exposed
    // externally as "12V OUT". After enabling, the FW waits ~1 s for the
    // rail to stabilise; enableBoost() does the same.
    // Also available as enable12V() for clarity.
    void enableBoost(bool on);
    inline void enable12V(bool on) { enableBoost(on); }
    void enableVBAT2(bool on);

    // ---- Voltage monitors ----
    float readBatteryVoltage();
    float readPanelVoltage();
    float readUsbVoltage();
    float readOutputVoltage();

    // ---- UART2 mux / RS485 ----
    void selectUart2(SensBlueUartTarget target);
    SensBlueUartTarget getUart2Target() const { return _uart2Target; }
    inline void rs485BeginTx() { digitalWrite(SB_RS485_DE, HIGH); }
    inline void rs485EndTx()   { digitalWrite(SB_RS485_DE, LOW);  }

    // ---- BG95 modem ----
    // powerOn: enables VBAT_2, waits 1 s, then pulses POWERKEY high for
    //          pulseMs (default 1100 ms — matches the ATRONIA FW).
    //          Call Serial1.begin() BEFORE this — as the FW does.
    // powerOff: pulses POWERKEY for pulseMs (default 700 ms), then cuts
    //          VBAT_2.
    void modemPowerOn(uint16_t pulseMs = 1100);
    void modemPowerOff(uint16_t pulseMs = 700);
    void modemForceReset();

    // ---- GNSS (BG95 built-in receiver) ----
    // Enables the GNSS power path and routes UART2 through the GNSS mux.
    // Users read NMEA from Serial2 after configuring it themselves, or
    // via the high-level readGnssLine() helper.
    void enableGNSS(bool on);

    // ---- Button ----
    inline bool readButton() { return digitalRead(SB_BTN) == LOW; }

    // ---- Piggyback IOs (via expander) ----
    void writePiggybackIO(uint8_t bit, bool state);

    // ---- Low-level expander access (advanced) ----
    void writeExpanderBit(uint8_t expanderBit, bool state);
    bool readExpanderInputBit(uint8_t expanderBit);

    // -------- Environmental sensor (BME280) --------
    bool beginEnvSensor();       // must be called after begin()
    float readTemperature();     // °C, NAN on failure
    float readHumidity();        // %RH, NAN on failure
    float readPressure();        // hPa, NAN on failure
    bool envSensorReady() const { return _bmeReady; }

    // -------- OLED (SSD1306 128x32 @ 0x3C) --------
    bool beginDisplay();
    bool displayReady() const { return _oledReady; }
    // Draws a pre-formatted status screen using whatever peripherals have
    // been initialised so far (battery %, environment, uptime). Call
    // display.display() yourself after adding extra content, or call
    // showStatusScreen() and it will refresh the display for you.
    void showStatusScreen(const char *header = nullptr);

    // -------- EEPROM (AT24C02, 256 bytes) --------
    bool beginEeprom();
    // Byte-oriented raw access. addr is 0..255. Returns true on success.
    bool eepromWriteByte(uint8_t addr, uint8_t value);
    bool eepromReadByte (uint8_t addr, uint8_t &value);
    // Multi-byte helpers. Handles page boundaries and write cycles.
    bool eepromWrite(uint8_t addr, const uint8_t *data, size_t len);
    bool eepromRead (uint8_t addr,       uint8_t *data, size_t len);
    bool eepromReady() const { return _eepromReady; }

    // -------- Battery percentage estimation --------
    // Wraps the on-board estimator. Voltage is auto-read from VBAT_ADC.
    // Timestamp defaults to millis()/1000 — pass epoch seconds if you
    // have real time from NTP for better dt calculation across resets.
    bool  beginBattery(uint8_t type = BATTERY_TYPE_LIION);
    float readBatteryPercentage(uint32_t epoch = 0, float temperatureC = 25.0f);

private:
    TwoWire *_wire = nullptr;
    uint16_t _outputCache = 0x0000;
    uint16_t _configCache = 0xFFFF;
    SensBlueAnalogMode _analogMode = SB_ANALOG_MODE_0_10V;
    SensBlueUartTarget _uart2Target = SB_UART2_RS485;

    bool _bmeReady    = false;
    bool _oledReady   = false;
    bool _eepromReady = false;
    bool _batteryReady= false;

    bool writeExpanderRegister(uint8_t reg, uint16_t value);
    bool readExpanderRegister (uint8_t reg, uint16_t &value);
    void applyOutputCache();
    void applyConfigCache();

    float countsToVolts(uint16_t counts) const;
};

extern SensBlueMonarchClass SensBlueMonarch;

#endif
