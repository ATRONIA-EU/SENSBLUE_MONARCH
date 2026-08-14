/*
 * SensBlueMonarch.cpp — implementation.
 *
 * NOTE: PCAL6416A register map used here:
 *   0x00 Input Port 0        0x01 Input Port 1
 *   0x02 Output Port 0       0x03 Output Port 1   (POR = 0xFF, all high)
 *   0x04 Polarity Inversion 0/1
 *   0x06 Configuration Port 0/1 (POR = 0xFF, all inputs)
 * The 16-bit "registers" below are formed as (port1 << 8) | port0, and we
 * write them with the auto-increment feature of the chip.
 */

#include "SensBlueMonarch.h"
#include "pins_arduino.h"

// PCAL6416A registers
#define PCAL_REG_INPUT   0x00
#define PCAL_REG_OUTPUT  0x02
#define PCAL_REG_POL     0x04
#define PCAL_REG_CONFIG  0x06

// Which bits we drive as outputs. Everything except the piggyback IOs is
// an output for us. P0_0, P1_2 are unused — leave as inputs (safer).
// P1_0 and P1_1 (piggyback) are outputs by default too; the user can flip
// them back with the low-level API if they need to read.
static constexpr uint16_t OUTPUT_MASK =
      (1u << SENSBLUE_EXP_DIG_OUT_EN_1)
    | (1u << SENSBLUE_EXP_DIG_OUT_EN_2)
    | (1u << SENSBLUE_EXP_BOOST_EN)
    | (1u << SENSBLUE_EXP_VBAT_2_EN)
    | (1u << SENSBLUE_EXP_POWERKEY)
    | (1u << SENSBLUE_EXP_VCC_2_EN)
    | (1u << SENSBLUE_EXP_WP)
    | (1u << SENSBLUE_EXP_P1_0)
    | (1u << SENSBLUE_EXP_P1_1)
    | (1u << SENSBLUE_EXP_INPUT_IV_SEL)
    | (1u << SENSBLUE_EXP_USART_SEL)
    | (1u << SENSBLUE_EXP_LED_B)
    | (1u << SENSBLUE_EXP_LED_G)
    | (1u << SENSBLUE_EXP_LED_R);

// LEDs are driven active-LOW at the expander (cathode to expander pin, anode
// to Vcc through a resistor). We hide that inversion from the API.
static inline uint8_t ledBit(SensBlueLED colour) {
    switch (colour) {
        case LED_RED:   return SENSBLUE_EXP_LED_R;
        case LED_GREEN: return SENSBLUE_EXP_LED_G;
        case LED_BLUE:  return SENSBLUE_EXP_LED_B;
    }
    return SENSBLUE_EXP_LED_R;
}

SensBlueMonarchClass SensBlueMonarch;

// ---------------------------------------------------------------------------

bool SensBlueMonarchClass::begin(TwoWire &wire, uint32_t clockHz) {
    _wire = &wire;
    _wire->begin(SDA, SCL, clockHz);

    // VCC_3_EN direct GPIO — enable peripherals rail by default.
    pinMode(VCC_3_EN, OUTPUT);
    digitalWrite(VCC_3_EN, HIGH);

    // Discrete GPIOs used directly
    pinMode(BTN, INPUT_PULLUP);
    pinMode(DIG_IN_1, INPUT);
    pinMode(DIG_IN_2, INPUT);
    pinMode(RS485_DE, OUTPUT);
    digitalWrite(RS485_DE, LOW);

    // Probe the expander
    _wire->beginTransmission(SENSBLUE_EXP_ADDR);
    if (_wire->endTransmission() != 0) {
        return false;
    }

    // Set default output state: all zero, with LEDs off (LEDs are active-LOW
    // so "off" means the corresponding bit is 1). DIG_OUT_EN_x = 0 (off).
    // INPUT_IV_SEL = 0 (0-10 V), USART_SEL = 0 (RS485), BOOST_EN = 0.
    _outputCache = 0;
    _outputCache |= (1u << SENSBLUE_EXP_LED_R);
    _outputCache |= (1u << SENSBLUE_EXP_LED_G);
    _outputCache |= (1u << SENSBLUE_EXP_LED_B);
    // WP high = protected. Safer default.
    _outputCache |= (1u << SENSBLUE_EXP_WP);

    applyOutputCache();

    // Configure outputs (0 = output).
    _configCache = static_cast<uint16_t>(~OUTPUT_MASK);
    applyConfigCache();

    return true;
}

// ---------------------------------------------------------------------------
// Expander I/O primitives
// ---------------------------------------------------------------------------

bool SensBlueMonarchClass::writeExpanderRegister(uint8_t reg, uint16_t value) {
    _wire->beginTransmission(SENSBLUE_EXP_ADDR);
    _wire->write(reg);
    _wire->write(static_cast<uint8_t>(value & 0xFF));      // port 0
    _wire->write(static_cast<uint8_t>((value >> 8) & 0xFF)); // port 1
    return _wire->endTransmission() == 0;
}

bool SensBlueMonarchClass::readExpanderRegister(uint8_t reg, uint16_t &value) {
    _wire->beginTransmission(SENSBLUE_EXP_ADDR);
    _wire->write(reg);
    if (_wire->endTransmission(false) != 0) return false;
    if (_wire->requestFrom((uint8_t)SENSBLUE_EXP_ADDR, (uint8_t)2) != 2) return false;
    uint8_t p0 = _wire->read();
    uint8_t p1 = _wire->read();
    value = (uint16_t)p1 << 8 | p0;
    return true;
}

void SensBlueMonarchClass::applyOutputCache() {
    writeExpanderRegister(PCAL_REG_OUTPUT, _outputCache);
}

void SensBlueMonarchClass::applyConfigCache() {
    writeExpanderRegister(PCAL_REG_CONFIG, _configCache);
}

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
    // Active-LOW → on == false gives a 1 at the bit.
    writeExpanderBit(ledBit(colour), !on);
}

void SensBlueMonarchClass::setRGB(bool red, bool green, bool blue) {
    if (red)   _outputCache &= ~(1u << SENSBLUE_EXP_LED_R);
    else       _outputCache |=  (1u << SENSBLUE_EXP_LED_R);
    if (green) _outputCache &= ~(1u << SENSBLUE_EXP_LED_G);
    else       _outputCache |=  (1u << SENSBLUE_EXP_LED_G);
    if (blue)  _outputCache &= ~(1u << SENSBLUE_EXP_LED_B);
    else       _outputCache |=  (1u << SENSBLUE_EXP_LED_B);
    applyOutputCache();
}

void SensBlueMonarchClass::allLEDsOff() {
    setRGB(false, false, false);
}

// ---------------------------------------------------------------------------
// Digital outputs
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::setDigitalOutput(uint8_t channel, bool state) {
    if (channel == 1) writeExpanderBit(SENSBLUE_EXP_DIG_OUT_EN_1, state);
    else if (channel == 2) writeExpanderBit(SENSBLUE_EXP_DIG_OUT_EN_2, state);
}

bool SensBlueMonarchClass::readDigitalInput(uint8_t channel) {
    // Opto is inverting: input active pulls the pin low.
    if (channel == 1) return digitalRead(DIG_IN_1) == LOW;
    if (channel == 2) return digitalRead(DIG_IN_2) == LOW;
    return false;
}

// ---------------------------------------------------------------------------
// Analog inputs
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::setAnalogInputMode(SensBlueAnalogMode mode) {
    _analogMode = mode;
    writeExpanderBit(SENSBLUE_EXP_INPUT_IV_SEL, mode == ANALOG_MODE_4_20MA);
    // The +12 V boost is only needed for the 4-20 mA loop.
    enableBoost(mode == ANALOG_MODE_4_20MA);
}

uint16_t SensBlueMonarchClass::readAnalogRaw(uint8_t channel) {
    uint8_t pin = (channel == 1) ? ANALOG_IN_1_RAW : ANALOG_IN_2_RAW;
    return analogRead(pin);
}

float SensBlueMonarchClass::countsToVolts(uint16_t counts) const {
    return (counts * (float)SENSBLUE_ADC_VREF_MV) /
           ((float)SENSBLUE_ADC_MAX_COUNT * 1000.0f);
}

float SensBlueMonarchClass::readAnalog(uint8_t channel, uint8_t samples) {
    if (samples == 0) samples = 1;
    uint32_t acc = 0;
    for (uint8_t i = 0; i < samples; i++) acc += readAnalogRaw(channel);
    float vAtAdc = countsToVolts(acc / samples);

    if (_analogMode == ANALOG_MODE_0_10V) {
        // Undo the resistor divider
        return vAtAdc * ((float)SENSBLUE_010V_DIV_DEN /
                         (float)SENSBLUE_010V_DIV_NUM);
    } else {
        // I = V / R (in mA when V in volts and R in ohms → mA = V*1000/R)
        return (vAtAdc * 1000.0f) / (float)SENSBLUE_420MA_SHUNT_OHM;
    }
}

// ---------------------------------------------------------------------------
// Rails
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::enableVCC2(bool on)  { writeExpanderBit(SENSBLUE_EXP_VCC_2_EN, on);   }
void SensBlueMonarchClass::enableVCC3(bool on)  { digitalWrite(VCC_3_EN, on ? HIGH : LOW);       }
void SensBlueMonarchClass::enableBoost(bool on) { writeExpanderBit(SENSBLUE_EXP_BOOST_EN, on);   }
void SensBlueMonarchClass::enableVBAT2(bool on) { writeExpanderBit(SENSBLUE_EXP_VBAT_2_EN, on);  }
void SensBlueMonarchClass::setEepromWriteProtect(bool wp) {
    writeExpanderBit(SENSBLUE_EXP_WP, wp);
}

// ---------------------------------------------------------------------------
// Monitors
// ---------------------------------------------------------------------------

float SensBlueMonarchClass::readBatteryVoltage() {
    // VBAT_2_EN needs to be on for the divider to be biased.
    enableVBAT2(true);
    delay(2);
    uint16_t raw = analogRead(VBAT_ADC);
    float vAdc = countsToVolts(raw);
    return vAdc * (float)SENSBLUE_VBAT_DIV_DEN / (float)SENSBLUE_VBAT_DIV_NUM;
}

float SensBlueMonarchClass::readPanelVoltage() {
    // Assume same 1:2 divider until verified.
    uint16_t raw = analogRead(PV_ADC);
    return countsToVolts(raw) * 2.0f;
}

float SensBlueMonarchClass::readUsbVoltage() {
    uint16_t raw = analogRead(VUSB_ADC);
    return countsToVolts(raw) * 2.0f;
}

float SensBlueMonarchClass::readOutputVoltage() {
    uint16_t raw = analogRead(POUT_ADC);
    return countsToVolts(raw) * 2.0f;
}

// ---------------------------------------------------------------------------
// UART2 mux
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::selectUart2(SensBlueUartTarget target) {
    _uart2Target = target;
    writeExpanderBit(SENSBLUE_EXP_USART_SEL, target == UART2_PIGGYBACK);
}

// ---------------------------------------------------------------------------
// Modem
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::modemPowerOn(uint16_t pulseMs) {
    // BG95 PWRKEY is level shifted; expander bit high → PWRKEY pulled to
    // ground long enough to trigger power-on. (This assumes the polarity
    // matches production silicon — verify on bench before shipping.)
    writeExpanderBit(SENSBLUE_EXP_POWERKEY, true);
    delay(pulseMs);
    writeExpanderBit(SENSBLUE_EXP_POWERKEY, false);
}

void SensBlueMonarchClass::modemPowerOff(uint16_t pulseMs) {
    writeExpanderBit(SENSBLUE_EXP_POWERKEY, true);
    delay(pulseMs);
    writeExpanderBit(SENSBLUE_EXP_POWERKEY, false);
}

void SensBlueMonarchClass::modemForceReset() {
    writeExpanderBit(SENSBLUE_EXP_POWERKEY, true);
    delay(3200);
    writeExpanderBit(SENSBLUE_EXP_POWERKEY, false);
}

// ---------------------------------------------------------------------------
// Piggyback
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::writePiggybackIO(uint8_t bit, bool state) {
    if (bit == 0) writeExpanderBit(SENSBLUE_EXP_P1_0, state);
    else if (bit == 1) writeExpanderBit(SENSBLUE_EXP_P1_1, state);
}
