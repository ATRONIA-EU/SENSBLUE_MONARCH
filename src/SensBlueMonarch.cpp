/*
 * SensBlueMonarch.cpp — implementation.
 *
 * PCAL6416A register map:
 *   0x00 Input Port 0        0x01 Input Port 1
 *   0x02 Output Port 0       0x03 Output Port 1   (POR = 0xFF, all high)
 *   0x04 Polarity Inversion 0/1
 *   0x06 Configuration Port 0/1 (POR = 0xFF, all inputs)
 * 16-bit "registers" here are (port1 << 8) | port0, using auto-increment.
 */

#include "SensBlueMonarch.h"

#define PCAL_REG_INPUT   0x00
#define PCAL_REG_OUTPUT  0x02
#define PCAL_REG_POL     0x04
#define PCAL_REG_CONFIG  0x06

static constexpr uint16_t OUTPUT_MASK =
      (1u << SB_EXP_DIG_OUT_EN_1)
    | (1u << SB_EXP_DIG_OUT_EN_2)
    | (1u << SB_EXP_BOOST_EN)
    | (1u << SB_EXP_VBAT_2_EN)
    | (1u << SB_EXP_POWERKEY)
    | (1u << SB_EXP_VCC_2_EN)
    | (1u << SB_EXP_WP)
    | (1u << SB_EXP_P1_0)
    | (1u << SB_EXP_P1_1)
    | (1u << SB_EXP_INPUT_IV_SEL)
    | (1u << SB_EXP_USART_SEL)
    | (1u << SB_EXP_LED_B)
    | (1u << SB_EXP_LED_G)
    | (1u << SB_EXP_LED_R);

// LEDs are active-LOW at the expander.
static inline uint8_t ledBit(SensBlueLED colour) {
    switch (colour) {
        case SB_LED_RED:   return SB_EXP_LED_R;
        case SB_LED_GREEN: return SB_EXP_LED_G;
        case SB_LED_BLUE:  return SB_EXP_LED_B;
    }
    return SB_EXP_LED_R;
}

SensBlueMonarchClass SensBlueMonarch;

// ---------------------------------------------------------------------------

bool SensBlueMonarchClass::begin(TwoWire &wire, uint32_t clockHz) {
    _wire = &wire;
    _wire->begin(SB_SDA, SB_SCL, clockHz);

    pinMode(SB_VCC_3_EN, OUTPUT);
    digitalWrite(SB_VCC_3_EN, HIGH);

    pinMode(SB_BTN, INPUT_PULLUP);
    pinMode(SB_DIG_IN_1, INPUT);
    pinMode(SB_DIG_IN_2, INPUT);
    pinMode(SB_RS485_DE, OUTPUT);
    digitalWrite(SB_RS485_DE, LOW);

    _wire->beginTransmission(SB_EXP_ADDR);
    if (_wire->endTransmission() != 0) {
        return false;
    }

    _outputCache = 0;
    _outputCache |= (1u << SB_EXP_LED_R);
    _outputCache |= (1u << SB_EXP_LED_G);
    _outputCache |= (1u << SB_EXP_LED_B);
    _outputCache |= (1u << SB_EXP_WP);
    applyOutputCache();

    _configCache = static_cast<uint16_t>(~OUTPUT_MASK);
    applyConfigCache();

    return true;
}

// ---------------------------------------------------------------------------
// Expander I/O primitives
// ---------------------------------------------------------------------------

bool SensBlueMonarchClass::writeExpanderRegister(uint8_t reg, uint16_t value) {
    _wire->beginTransmission(SB_EXP_ADDR);
    _wire->write(reg);
    _wire->write(static_cast<uint8_t>(value & 0xFF));
    _wire->write(static_cast<uint8_t>((value >> 8) & 0xFF));
    return _wire->endTransmission() == 0;
}

bool SensBlueMonarchClass::readExpanderRegister(uint8_t reg, uint16_t &value) {
    _wire->beginTransmission(SB_EXP_ADDR);
    _wire->write(reg);
    if (_wire->endTransmission(false) != 0) return false;
    if (_wire->requestFrom((uint8_t)SB_EXP_ADDR, (uint8_t)2) != 2) return false;
    uint8_t p0 = _wire->read();
    uint8_t p1 = _wire->read();
    value = (uint16_t)p1 << 8 | p0;
    return true;
}

void SensBlueMonarchClass::applyOutputCache() { writeExpanderRegister(PCAL_REG_OUTPUT, _outputCache); }
void SensBlueMonarchClass::applyConfigCache() { writeExpanderRegister(PCAL_REG_CONFIG, _configCache); }

void SensBlueMonarchClass::writeExpanderBit(uint8_t expanderBit, bool state) {
    if (expanderBit > 15) return;
    if (state) _outputCache |=  (1u << expanderBit);
    else       _outputCache &= ~(1u << expanderBit);
    applyOutputCache();
}

bool SensBlueMonarchClass::readExpanderInputBit(uint8_t expanderBit) {
    uint16_t v = 0;
    if (!readExpanderRegister(PCAL_REG_INPUT, v)) return false;
    return (v >> expanderBit) & 0x1;
}

// ---------------------------------------------------------------------------
// LEDs
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::setLED(SensBlueLED colour, bool on) {
    writeExpanderBit(ledBit(colour), !on);
}

void SensBlueMonarchClass::setRGB(bool red, bool green, bool blue) {
    if (red)   _outputCache &= ~(1u << SB_EXP_LED_R);
    else       _outputCache |=  (1u << SB_EXP_LED_R);
    if (green) _outputCache &= ~(1u << SB_EXP_LED_G);
    else       _outputCache |=  (1u << SB_EXP_LED_G);
    if (blue)  _outputCache &= ~(1u << SB_EXP_LED_B);
    else       _outputCache |=  (1u << SB_EXP_LED_B);
    applyOutputCache();
}

void SensBlueMonarchClass::allLEDsOff() { setRGB(false, false, false); }

// ---------------------------------------------------------------------------
// Digital outputs
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::setDigitalOutput(uint8_t channel, bool state) {
    if (channel == 1) writeExpanderBit(SB_EXP_DIG_OUT_EN_1, state);
    else if (channel == 2) writeExpanderBit(SB_EXP_DIG_OUT_EN_2, state);
}

bool SensBlueMonarchClass::readDigitalInput(uint8_t channel) {
    if (channel == 1) return digitalRead(SB_DIG_IN_1) == LOW;
    if (channel == 2) return digitalRead(SB_DIG_IN_2) == LOW;
    return false;
}

// ---------------------------------------------------------------------------
// Analog inputs
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::setAnalogInputMode(SensBlueAnalogMode mode) {
    _analogMode = mode;
    writeExpanderBit(SB_EXP_INPUT_IV_SEL, mode == SB_ANALOG_MODE_4_20MA);
    enableBoost(mode == SB_ANALOG_MODE_4_20MA);
}

uint16_t SensBlueMonarchClass::readAnalogRaw(uint8_t channel) {
    uint8_t pin = (channel == 1) ? SB_ANALOG_IN_1_RAW : SB_ANALOG_IN_2_RAW;
    return analogRead(pin);
}

float SensBlueMonarchClass::countsToVolts(uint16_t counts) const {
    return (counts * (float)SB_ADC_VREF_MV) /
           ((float)SB_ADC_MAX_COUNT * 1000.0f);
}

float SensBlueMonarchClass::readAnalog(uint8_t channel, uint8_t samples) {
    if (samples == 0) samples = 1;
    uint32_t acc = 0;
    for (uint8_t i = 0; i < samples; i++) acc += readAnalogRaw(channel);
    float vAtAdc = countsToVolts(acc / samples);

    if (_analogMode == SB_ANALOG_MODE_0_10V) {
        return vAtAdc * ((float)SB_010V_DIV_DEN / (float)SB_010V_DIV_NUM);
    } else {
        return (vAtAdc * 1000.0f) / (float)SB_420MA_SHUNT_OHM;
    }
}

// ---------------------------------------------------------------------------
// Rails
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::enableVCC2(bool on)  { writeExpanderBit(SB_EXP_VCC_2_EN, on);  }
void SensBlueMonarchClass::enableVCC3(bool on)  { digitalWrite(SB_VCC_3_EN, on ? HIGH : LOW); }
void SensBlueMonarchClass::enableBoost(bool on) { writeExpanderBit(SB_EXP_BOOST_EN, on);  }
void SensBlueMonarchClass::enableVBAT2(bool on) { writeExpanderBit(SB_EXP_VBAT_2_EN, on); }
void SensBlueMonarchClass::setEepromWriteProtect(bool wp) {
    writeExpanderBit(SB_EXP_WP, wp);
}

// ---------------------------------------------------------------------------
// Monitors
// ---------------------------------------------------------------------------

float SensBlueMonarchClass::readBatteryVoltage() {
    enableVBAT2(true);
    delay(2);
    uint16_t raw = analogRead(SB_VBAT_ADC);
    float vAdc = countsToVolts(raw);
    return vAdc * (float)SB_VBAT_DIV_DEN / (float)SB_VBAT_DIV_NUM;
}

float SensBlueMonarchClass::readPanelVoltage() {
    return countsToVolts(analogRead(SB_PV_ADC)) * 2.0f;
}

float SensBlueMonarchClass::readUsbVoltage() {
    return countsToVolts(analogRead(SB_VUSB_ADC)) * 2.0f;
}

float SensBlueMonarchClass::readOutputVoltage() {
    return countsToVolts(analogRead(SB_POUT_ADC)) * 2.0f;
}

// ---------------------------------------------------------------------------
// UART2 mux
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::selectUart2(SensBlueUartTarget target) {
    _uart2Target = target;
    writeExpanderBit(SB_EXP_USART_SEL, target == SB_UART2_PIGGYBACK);
}

// ---------------------------------------------------------------------------
// Modem
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::modemPowerOn(uint16_t pulseMs) {
    writeExpanderBit(SB_EXP_POWERKEY, true);
    delay(pulseMs);
    writeExpanderBit(SB_EXP_POWERKEY, false);
}

void SensBlueMonarchClass::modemPowerOff(uint16_t pulseMs) {
    writeExpanderBit(SB_EXP_POWERKEY, true);
    delay(pulseMs);
    writeExpanderBit(SB_EXP_POWERKEY, false);
}

void SensBlueMonarchClass::modemForceReset() {
    writeExpanderBit(SB_EXP_POWERKEY, true);
    delay(3200);
    writeExpanderBit(SB_EXP_POWERKEY, false);
}

// ---------------------------------------------------------------------------
// Piggyback
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::writePiggybackIO(uint8_t bit, bool state) {
    if (bit == 0) writeExpanderBit(SB_EXP_P1_0, state);
    else if (bit == 1) writeExpanderBit(SB_EXP_P1_1, state);
}
