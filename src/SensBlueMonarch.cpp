/*
 * SensBlueMonarch.cpp — implementation for hardware revision 2.41.
 */

#include "SensBlueMonarch.h"

// PCAL6416A register map
#define PCAL_REG_INPUT   0x00
#define PCAL_REG_OUTPUT  0x02
#define PCAL_REG_POL     0x04
#define PCAL_REG_CONFIG  0x06

// AT24C02 write cycle time (datasheet: 5 ms typical, 10 ms max at 5 V)
#define AT24C02_WRITE_MS 6

// Which expander bits we drive as outputs.
static constexpr uint16_t OUTPUT_MASK =
      (1u << SB_EXP_GPS_EN)
    | (1u << SB_EXP_DIG_OUT_EN_1)
    | (1u << SB_EXP_DIG_OUT_EN_2)
    | (1u << SB_EXP_BOOST_EN)
    | (1u << SB_EXP_VBAT_2_EN)
    | (1u << SB_EXP_POWERKEY)
    | (1u << SB_EXP_VCC_2_EN)
    | (1u << SB_EXP_P1_0)
    | (1u << SB_EXP_P1_1)
    | (1u << SB_EXP_INPUT_IV_SEL)
    | (1u << SB_EXP_USART_SEL)
    | (1u << SB_EXP_LED_B)
    | (1u << SB_EXP_LED_G)
    | (1u << SB_EXP_LED_R);

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

    // LEDs off (active-LOW), everything else at 0 → safe defaults
    _outputCache = 0;
    _outputCache |= (1u << SB_EXP_LED_R);
    _outputCache |= (1u << SB_EXP_LED_G);
    _outputCache |= (1u << SB_EXP_LED_B);
    applyOutputCache();

    _configCache = static_cast<uint16_t>(~OUTPUT_MASK);
    applyConfigCache();

    return true;
}

// ---------------------------------------------------------------------------
// Expander primitives
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
// LEDs (active-LOW at expander)
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::setLED(SensBlueLED colour, bool on) {
    writeExpanderBit(ledBit(colour), !on);
}

void SensBlueMonarchClass::setRGB(bool r, bool g, bool b) {
    if (r) _outputCache &= ~(1u << SB_EXP_LED_R); else _outputCache |= (1u << SB_EXP_LED_R);
    if (g) _outputCache &= ~(1u << SB_EXP_LED_G); else _outputCache |= (1u << SB_EXP_LED_G);
    if (b) _outputCache &= ~(1u << SB_EXP_LED_B); else _outputCache |= (1u << SB_EXP_LED_B);
    applyOutputCache();
}

void SensBlueMonarchClass::allLEDsOff() { setRGB(false, false, false); }

// ---------------------------------------------------------------------------
// Digital I/O
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::setDigitalOutput(uint8_t ch, bool state) {
    if (ch == 1) writeExpanderBit(SB_EXP_DIG_OUT_EN_1, state);
    else if (ch == 2) writeExpanderBit(SB_EXP_DIG_OUT_EN_2, state);
}

bool SensBlueMonarchClass::readDigitalInput(uint8_t ch) {
    if (ch == 1) return digitalRead(SB_DIG_IN_1) == LOW;
    if (ch == 2) return digitalRead(SB_DIG_IN_2) == LOW;
    return false;
}

// ---------------------------------------------------------------------------
// Analog
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::setAnalogInputMode(SensBlueAnalogMode mode) {
    _analogMode = mode;
    writeExpanderBit(SB_EXP_INPUT_IV_SEL, mode == SB_ANALOG_MODE_4_20MA);
    enableBoost(mode == SB_ANALOG_MODE_4_20MA);
}

uint16_t SensBlueMonarchClass::readAnalogRaw(uint8_t ch) {
    uint8_t pin = (ch == 1) ? SB_ANALOG_IN_1_RAW : SB_ANALOG_IN_2_RAW;
    return analogRead(pin);
}

float SensBlueMonarchClass::countsToVolts(uint16_t counts) const {
    return (counts * (float)SB_ADC_VREF_MV) /
           ((float)SB_ADC_MAX_COUNT * 1000.0f);
}

float SensBlueMonarchClass::readAnalog(uint8_t ch, uint8_t samples) {
    if (samples == 0) samples = 1;
    uint32_t acc = 0;
    for (uint8_t i = 0; i < samples; i++) acc += readAnalogRaw(ch);
    float v = countsToVolts(acc / samples);

    if (_analogMode == SB_ANALOG_MODE_0_10V)
        return v * ((float)SB_010V_DIV_DEN / (float)SB_010V_DIV_NUM);
    return (v * 1000.0f) / (float)SB_420MA_SHUNT_OHM;
}

// ---------------------------------------------------------------------------
// Rails
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::enableVCC2(bool on)  { writeExpanderBit(SB_EXP_VCC_2_EN, on);  }
void SensBlueMonarchClass::enableVCC3(bool on)  { digitalWrite(SB_VCC_3_EN, on ? HIGH : LOW); }
void SensBlueMonarchClass::enableBoost(bool on) {
    // Mirror the ATRONIA FW Enable_BOOST() — 1 s settle time on turn-on.
    writeExpanderBit(SB_EXP_BOOST_EN, on);
    if (on) delay(1000);
}
void SensBlueMonarchClass::enableVBAT2(bool on) { writeExpanderBit(SB_EXP_VBAT_2_EN, on); }

// ---------------------------------------------------------------------------
// Monitors
// ---------------------------------------------------------------------------

float SensBlueMonarchClass::readBatteryVoltage() {
    enableVBAT2(true);
    delay(2);
    float v = countsToVolts(analogRead(SB_VBAT_ADC));
    return v * (float)SB_VBAT_DIV_DEN / (float)SB_VBAT_DIV_NUM;
}

float SensBlueMonarchClass::readPanelVoltage()  { return countsToVolts(analogRead(SB_PV_ADC))   * 2.0f; }
float SensBlueMonarchClass::readUsbVoltage()    { return countsToVolts(analogRead(SB_VUSB_ADC)) * 2.0f; }
float SensBlueMonarchClass::readOutputVoltage() { return countsToVolts(analogRead(SB_POUT_ADC)) * 2.0f; }

// ---------------------------------------------------------------------------
// UART2 mux and RS485
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::selectUart2(SensBlueUartTarget target) {
    _uart2Target = target;
    writeExpanderBit(SB_EXP_USART_SEL, target == SB_UART2_GNSS);
}

// ---------------------------------------------------------------------------
// Modem
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::modemPowerOn(uint16_t pulseMs) {
    // Mirror the ATRONIA FW GSM_TurnON() sequence exactly.
    // VBAT_2 is the modem power rail — the BG95 has no power at all
    // without it. Sequence:
    //   1. Enable VBAT_2 (modem power rail).
    //   2. Wait 1000 ms for the rail to settle.
    //   3. Pulse POWERKEY high for ~1100 ms.
    // The caller must begin() Serial1 with the modem baudrate BEFORE
    // calling this function (the FW does the same).
    enableVBAT2(true);
    delay(1000);

    writeExpanderBit(SB_EXP_POWERKEY, true);
    delay(pulseMs);
    writeExpanderBit(SB_EXP_POWERKEY, false);
}

void SensBlueMonarchClass::modemPowerOff(uint16_t pulseMs) {
    // Mirror the ATRONIA FW GSM_TurnOFF() shutdown sequence.
    //   1. Pulse POWERKEY high for ~700 ms  → asks the modem to shut down.
    //   2. Cut VBAT_2                        → capacitors discharge.
    writeExpanderBit(SB_EXP_POWERKEY, true);
    delay(pulseMs);
    writeExpanderBit(SB_EXP_POWERKEY, false);

    enableVBAT2(false);
}

void SensBlueMonarchClass::modemForceReset() {
    writeExpanderBit(SB_EXP_POWERKEY, true);
    delay(3200);
    writeExpanderBit(SB_EXP_POWERKEY, false);
}

// ---------------------------------------------------------------------------
// GNSS
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::enableGNSS(bool on) {
    writeExpanderBit(SB_EXP_GPS_EN, on);
    selectUart2(on ? SB_UART2_GNSS : SB_UART2_RS485);
}

// ---------------------------------------------------------------------------
// Piggyback
// ---------------------------------------------------------------------------

void SensBlueMonarchClass::writePiggybackIO(uint8_t bit, bool state) {
    if (bit == 0) writeExpanderBit(SB_EXP_P1_0, state);
    else if (bit == 1) writeExpanderBit(SB_EXP_P1_1, state);
}

// ===========================================================================
// BME280 wrapper
// ===========================================================================

bool SensBlueMonarchClass::beginEnvSensor() {
    // VCC_3 (peripherals rail) must be up and stable before probing the
    // BME280. Assume the user may have toggled it since begin() — enable
    // and wait a conservative time for the LDO to settle.
    enableVCC3(true);
    delay(300);

    _bmeReady = bme.begin(SB_I2C_BME280_ADDR, _wire ? _wire : &Wire);
    if (_bmeReady) {
        // Weather monitoring preset — low power, oversampling as recommended.
        bme.setSampling(Adafruit_BME280::MODE_FORCED,
                        Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::FILTER_OFF);
    }
    return _bmeReady;
}

float SensBlueMonarchClass::readTemperature() {
    if (!_bmeReady) return NAN;
    bme.takeForcedMeasurement();
    return bme.readTemperature();
}

float SensBlueMonarchClass::readHumidity() {
    if (!_bmeReady) return NAN;
    bme.takeForcedMeasurement();
    return bme.readHumidity();
}

float SensBlueMonarchClass::readPressure() {
    if (!_bmeReady) return NAN;
    bme.takeForcedMeasurement();
    return bme.readPressure() / 100.0f;
}

// ===========================================================================
// OLED (SSD1306) wrapper
// ===========================================================================

bool SensBlueMonarchClass::beginDisplay() {
    // VCC_3 (peripherals rail) must be up and stable before probing the
    // SSD1306. The Atronia FW waits 1000 ms after enabling the rail —
    // we do the same to be safe.
    enableVCC3(true);
    delay(1000);

    _oledReady = display.begin(SSD1306_SWITCHCAPVCC, SB_I2C_OLED_ADDR);
    if (_oledReady) {
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        display.setTextSize(1);
        display.display();
    }
    return _oledReady;
}

void SensBlueMonarchClass::showStatusScreen(const char *header) {
    if (!_oledReady) return;

    // 128x32 OLED — 4 lines of 8 px each with text size 1.
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(header ? header : "SensBlue Monarch");

    // Line 2: battery
    display.setCursor(0, 8);
    float vBat = readBatteryVoltage();
    display.print("Bat ");
    if (_batteryReady) {
        display.print((int)battery.update(vBat, millis() / 1000, 25.0f));
        display.print("%  ");
    }
    display.print(vBat, 2);
    display.print("V");

    // Line 3: temperature / humidity (only if BME280 is up)
    display.setCursor(0, 16);
    if (_bmeReady) {
        float t = readTemperature();
        float h = readHumidity();
        display.print("T "); display.print(t, 1);
        display.print((char)247); display.print("C  ");
        display.print("H "); display.print(h, 0); display.print("%");
    }

    // Line 4: uptime hh:mm:ss
    display.setCursor(0, 24);
    uint32_t s = millis() / 1000;
    uint32_t h = s / 3600; s %= 3600;
    uint32_t m = s / 60;   s %= 60;
    char buf[24];
    snprintf(buf, sizeof(buf), "Up %02lu:%02lu:%02lu", (unsigned long)h, (unsigned long)m, (unsigned long)s);
    display.print(buf);

    display.display();
}

// ===========================================================================
// EEPROM (AT24C02) — raw Wire access, no external library needed
// ===========================================================================

bool SensBlueMonarchClass::beginEeprom() {
    // VCC_3 rail powers the EEPROM too.
    enableVCC3(true);
    delay(200);

    // AT24C02 has no init sequence — probe with an address-only transaction.
    _wire->beginTransmission(SB_I2C_EEPROM_ADDR);
    _wire->write((uint8_t)0);
    _eepromReady = (_wire->endTransmission() == 0);
    return _eepromReady;
}

bool SensBlueMonarchClass::eepromWriteByte(uint8_t addr, uint8_t value) {
    if (!_eepromReady) return false;
    _wire->beginTransmission(SB_I2C_EEPROM_ADDR);
    _wire->write(addr);
    _wire->write(value);
    bool ok = (_wire->endTransmission() == 0);
    delay(AT24C02_WRITE_MS);
    return ok;
}

bool SensBlueMonarchClass::eepromReadByte(uint8_t addr, uint8_t &value) {
    if (!_eepromReady) return false;
    _wire->beginTransmission(SB_I2C_EEPROM_ADDR);
    _wire->write(addr);
    if (_wire->endTransmission(false) != 0) return false;
    if (_wire->requestFrom((uint8_t)SB_I2C_EEPROM_ADDR, (uint8_t)1) != 1) return false;
    value = _wire->read();
    return true;
}

bool SensBlueMonarchClass::eepromWrite(uint8_t addr, const uint8_t *data, size_t len) {
    if (!_eepromReady || data == nullptr) return false;
    if ((uint32_t)addr + len > SB_EEPROM_SIZE_BYTES) return false;

    while (len > 0) {
        // Stay within an 8-byte page.
        uint8_t pageStart = addr & ~(SB_EEPROM_PAGE_BYTES - 1);
        uint8_t roomInPage = pageStart + SB_EEPROM_PAGE_BYTES - addr;
        uint8_t chunk = (len < roomInPage) ? len : roomInPage;

        _wire->beginTransmission(SB_I2C_EEPROM_ADDR);
        _wire->write(addr);
        for (uint8_t i = 0; i < chunk; i++) _wire->write(data[i]);
        if (_wire->endTransmission() != 0) return false;
        delay(AT24C02_WRITE_MS);

        addr += chunk;
        data += chunk;
        len  -= chunk;
    }
    return true;
}

bool SensBlueMonarchClass::eepromRead(uint8_t addr, uint8_t *data, size_t len) {
    if (!_eepromReady || data == nullptr) return false;
    if ((uint32_t)addr + len > SB_EEPROM_SIZE_BYTES) return false;

    // Wire on ESP32 supports arbitrary-length reads up to I2C_BUFFER_LENGTH.
    // We chunk defensively at 32 bytes for portability.
    const size_t CHUNK = 32;
    while (len > 0) {
        size_t chunk = (len < CHUNK) ? len : CHUNK;
        _wire->beginTransmission(SB_I2C_EEPROM_ADDR);
        _wire->write(addr);
        if (_wire->endTransmission(false) != 0) return false;
        if (_wire->requestFrom((uint8_t)SB_I2C_EEPROM_ADDR, (uint8_t)chunk) != chunk) return false;
        for (size_t i = 0; i < chunk; i++) data[i] = _wire->read();

        addr += chunk;
        data += chunk;
        len  -= chunk;
    }
    return true;
}

// ===========================================================================
// Battery wrapper
// ===========================================================================

bool SensBlueMonarchClass::beginBattery(uint8_t type) {
    // Read voltage once to prime the estimator with a plausible starting state.
    float v = readBatteryVoltage();
    battery.setBatteryType(type, v);
    battery.loadFromNVS(v);
    _batteryReady = true;
    return true;
}

float SensBlueMonarchClass::readBatteryPercentage(uint32_t epoch, float temperatureC) {
    if (!_batteryReady) return NAN;
    if (epoch == 0) epoch = millis() / 1000;
    return battery.update(readBatteryVoltage(), epoch, temperatureC);
}
