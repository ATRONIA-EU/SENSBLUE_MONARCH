/*
 * SensBlueMonarch.h — high-level access to the on-board peripherals of the
 * ATRONIA SensBlue Monarch board.
 *
 * Copyright (c) 2026 ATRONIA, Lda. MIT licence.
 */

#ifndef SENSBLUE_MONARCH_H
#define SENSBLUE_MONARCH_H

#include <Arduino.h>
#include <Wire.h>

// LED colour selectors
enum SensBlueLED {
    LED_RED   = 0,
    LED_GREEN = 1,
    LED_BLUE  = 2
};

// Analog input mode. Applies to BOTH channels — the mux is shared.
enum SensBlueAnalogMode {
    ANALOG_MODE_0_10V   = 0,   // returns volts
    ANALOG_MODE_4_20MA  = 1    // returns milliamps
};

// UART2 mux target
enum SensBlueUartTarget {
    UART2_RS485      = 0,
    UART2_PIGGYBACK  = 1
};

class SensBlueMonarchClass {
public:
    // Initialises Wire (unless already begun by the user) at 400 kHz,
    // resets the PCAL6416A to a known state (all outputs low, INPUT_IV_SEL
    // driving 0-10 V, USART_SEL driving RS485). Returns false if the
    // expander does not ACK on the I2C bus.
    bool begin(TwoWire &wire = Wire, uint32_t clockHz = 400000);

    // ---- LEDs (active LOW at the expander drives the LED anode network) ----
    void setLED(SensBlueLED colour, bool on);
    void setRGB(bool red, bool green, bool blue);
    void allLEDsOff();

    // ---- Isolated digital outputs (channels 1 or 2) ----
    // 'state' true = output ON (high-side driver conducts).
    void setDigitalOutput(uint8_t channel, bool state);

    // ---- Isolated digital inputs ----
    // The opto-coupler pulls the ESP32 pin low when the external input is
    // energised, so we invert here to give the intuitive "true = active".
    bool readDigitalInput(uint8_t channel);

    // ---- Analog inputs ----
    // Sets the mode for BOTH channels (hardware mux is shared). Also
    // toggles the +12 V boost that feeds the 4-20 mA loop.
    void setAnalogInputMode(SensBlueAnalogMode mode);
    SensBlueAnalogMode getAnalogInputMode() const { return _analogMode; }

    // Returns V (0.0–10.0) or mA (0.0–~24.0) according to the current mode.
    // Averages 'samples' ADC reads (default 8) to reduce noise.
    float readAnalog(uint8_t channel, uint8_t samples = 8);
    uint16_t readAnalogRaw(uint8_t channel);

    // ---- Power rails ----
    void enableVCC2(bool on);   // via expander (P0_6)
    void enableVCC3(bool on);   // direct GPIO IO2
    void enableBoost(bool on);  // +12 V boost (also toggled by setAnalogInputMode)
    void enableVBAT2(bool on);
    void setEepromWriteProtect(bool wp);

    // ---- Voltage/current monitors ----
    float readBatteryVoltage();  // volts
    float readPanelVoltage();    // volts, on the solar / V_PANEL input
    float readUsbVoltage();      // volts
    float readOutputVoltage();   // volts, feedback of the switched output rail

    // ---- UART2 mux ----
    void selectUart2(SensBlueUartTarget target);
    SensBlueUartTarget getUart2Target() const { return _uart2Target; }

    // ---- RS485 direction ----
    inline void rs485BeginTx() { digitalWrite(RS485_DE, HIGH); }
    inline void rs485EndTx()   { digitalWrite(RS485_DE, LOW);  }

    // ---- BG95 modem power sequencing ----
    // powerOn() drives POWERKEY low for ~500 ms via the expander. Datasheet
    // says minimum 500 ms. After that, wait ≥5 s before sending AT commands.
    // NOT tested against production silicon by ATRONIA — see README.
    void modemPowerOn(uint16_t pulseMs = 600);
    void modemPowerOff(uint16_t pulseMs = 700);
    void modemForceReset();  // pulses POWERKEY for ≥3 s

    // ---- User button ----
    inline bool readButton() { return digitalRead(BTN) == LOW; }

    // ---- Piggyback IOs (via expander) ----
    void writePiggybackIO(uint8_t bit /* 0 or 1 */, bool state);

    // ---- Low-level expander access (advanced) ----
    // Read or write an individual bit of the PCAL6416A output register.
    // 'expanderBit' is one of the SENSBLUE_EXP_* constants from pins_arduino.h.
    void writeExpanderBit(uint8_t expanderBit, bool state);
    bool readExpanderInputBit(uint8_t expanderBit);

private:
    TwoWire *_wire = nullptr;
    uint16_t _outputCache = 0x0000;   // shadow of PCAL6416A output port
    uint16_t _configCache = 0xFFFF;   // 1 = input; we drive most as outputs
    SensBlueAnalogMode _analogMode = ANALOG_MODE_0_10V;
    SensBlueUartTarget _uart2Target = UART2_RS485;

    bool writeExpanderRegister(uint8_t reg, uint16_t value);
    bool readExpanderRegister(uint8_t reg, uint16_t &value);
    void applyOutputCache();
    void applyConfigCache();

    float countsToVolts(uint16_t counts) const;
};

extern SensBlueMonarchClass SensBlueMonarch;

#endif // SENSBLUE_MONARCH_H
