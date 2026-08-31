/*
 * SensBlueMonarch.h — high-level access to the on-board peripherals of
 * the ATRONIA SensBlue Monarch board.
 *
 * Copyright (c) 2026 ATRONIA, Lda. MIT licence.
 */

#ifndef SENSBLUE_MONARCH_H
#define SENSBLUE_MONARCH_H

#include <Arduino.h>
#include <Wire.h>
#include "SensBlueMonarchPins.h"

// LED colour selectors
enum SensBlueLED {
    SB_LED_RED   = 0,
    SB_LED_GREEN = 1,
    SB_LED_BLUE  = 2
};

// Analog input mode. Applies to BOTH channels — the mux is shared.
enum SensBlueAnalogMode {
    SB_ANALOG_MODE_0_10V   = 0,   // returns volts
    SB_ANALOG_MODE_4_20MA  = 1    // returns milliamps
};

// UART2 mux target
enum SensBlueUartTarget {
    SB_UART2_RS485      = 0,
    SB_UART2_PIGGYBACK  = 1
};

class SensBlueMonarchClass {
public:
    bool begin(TwoWire &wire = Wire, uint32_t clockHz = 400000);

    // ---- LEDs ----
    void setLED(SensBlueLED colour, bool on);
    void setRGB(bool red, bool green, bool blue);
    void allLEDsOff();

    // ---- Isolated digital outputs (channels 1 or 2) ----
    void setDigitalOutput(uint8_t channel, bool state);

    // ---- Isolated digital inputs ----
    bool readDigitalInput(uint8_t channel);

    // ---- Analog inputs ----
    void setAnalogInputMode(SensBlueAnalogMode mode);
    SensBlueAnalogMode getAnalogInputMode() const { return _analogMode; }
    float readAnalog(uint8_t channel, uint8_t samples = 8);
    uint16_t readAnalogRaw(uint8_t channel);

    // ---- Power rails ----
    void enableVCC2(bool on);
    void enableVCC3(bool on);
    void enableBoost(bool on);
    void enableVBAT2(bool on);
    void setEepromWriteProtect(bool wp);

    // ---- Voltage/current monitors ----
    float readBatteryVoltage();
    float readPanelVoltage();
    float readUsbVoltage();
    float readOutputVoltage();

    // ---- UART2 mux ----
    void selectUart2(SensBlueUartTarget target);
    SensBlueUartTarget getUart2Target() const { return _uart2Target; }

    // ---- RS485 direction ----
    inline void rs485BeginTx() { digitalWrite(SB_RS485_DE, HIGH); }
    inline void rs485EndTx()   { digitalWrite(SB_RS485_DE, LOW);  }

    // ---- BG95 modem power sequencing ----
    void modemPowerOn(uint16_t pulseMs = 600);
    void modemPowerOff(uint16_t pulseMs = 700);
    void modemForceReset();

    // ---- User button ----
    inline bool readButton() { return digitalRead(SB_BTN) == LOW; }

    // ---- Piggyback IOs (via expander) ----
    void writePiggybackIO(uint8_t bit /* 0 or 1 */, bool state);

    // ---- Low-level expander access (advanced) ----
    void writeExpanderBit(uint8_t expanderBit, bool state);
    bool readExpanderInputBit(uint8_t expanderBit);

private:
    TwoWire *_wire = nullptr;
    uint16_t _outputCache = 0x0000;
    uint16_t _configCache = 0xFFFF;
    SensBlueAnalogMode _analogMode = SB_ANALOG_MODE_0_10V;
    SensBlueUartTarget _uart2Target = SB_UART2_RS485;

    bool writeExpanderRegister(uint8_t reg, uint16_t value);
    bool readExpanderRegister(uint8_t reg, uint16_t &value);
    void applyOutputCache();
    void applyConfigCache();

    float countsToVolts(uint16_t counts) const;
};

extern SensBlueMonarchClass SensBlueMonarch;

#endif // SENSBLUE_MONARCH_H
