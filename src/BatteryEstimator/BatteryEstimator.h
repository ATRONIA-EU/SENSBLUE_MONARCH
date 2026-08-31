#pragma once

#include <Arduino.h>
#include <Preferences.h>

// =========================
// Defaults / Config
// =========================
#define BATTERY_NVS_NAMESPACE            "battery"
#define BATTERY_NVS_KEY                  "state"

#define BATTERY_DEFAULT_TAU              600.0f   // seconds
#define BATTERY_DEFAULT_HYST_UP          1.0f     // %
#define BATTERY_DEFAULT_HYST_DOWN        3.0f     // %
#define BATTERY_DEFAULT_TEMP_COEFF       0.0f     // V/°C relative to 25°C

#define BATTERY_POLY_NUM_COEFFS          5

// =========================
// Battery type / Config
// =========================
#define BATTERY_TYPE_LIION   0
#define BATTERY_TYPE_LIFEPO4 1

// =========================
// Thresholds
// =========================
#define BATTERY_SWAP_THRESHOLD           0.5f     // V
#define BATTERY_DIP_THRESHOLD            0.15f    // V
#define BATTERY_CHARGE_DELTA             0.01f    // V
#define BATTERY_LOAD_COMPENSATION        0.05f    // V

// Li-ion
#define LIION_FULL_VOLTAGE    4.15f
#define LIION_EMPTY_VOLTAGE   3.25f

// LiFePO4
#define LIFEPO4_FULL_VOLTAGE  3.55f
#define LIFEPO4_EMPTY_VOLTAGE 2.90f

// =========================
// Polynomial coefficients
// =========================

// Generic Li-ion candidate:
// y = -226.024x^4 + 3120.130x^3 - 15973.264x^2 + 36008.854x - 30201.890
// =========================
#define LIION_COEFF_0   -226.02397602f
#define LIION_COEFF_1    3120.12987010f
#define LIION_COEFF_2  -15973.26423559f
#define LIION_COEFF_3   36008.85364591f
#define LIION_COEFF_4  -30201.89010948f

// Generic LiFePo4 candidate:
// y = -90x^4 + 1100x^3 - -5000x^2 + 9500x - 6000
// =========================
#define LIFEPO4_COEFF_0  -90.0f
#define LIFEPO4_COEFF_1  1100.0f
#define LIFEPO4_COEFF_2 -5000.0f
#define LIFEPO4_COEFF_3  9500.0f
#define LIFEPO4_COEFF_4 -6000.0f


typedef struct
{
    uint8_t batteryType;
    float rawVoltage;
    float filteredVoltage;
    float compensatedVoltage;
    float temperature;
    float deltaVoltage;
    float percentRaw;
    float percentFinal;
    float calibrationOffset;
    uint32_t lastTimestamp;
    bool isCharging;
} BatteryDebug_t;

typedef struct
{
    uint8_t batteryType;
    float filteredVoltage;
    float lastPercent;
    float calibrationOffset;
    float lastVoltageRaw;
    uint32_t lastTimestamp;
    bool isCharging;
    bool initialized;
} BatteryStateNVS;

class BatteryEstimator
{
public:
BatteryEstimator(uint8_t batteryType = BATTERY_TYPE_LIION,
                 float tau = BATTERY_DEFAULT_TAU,
                 float hystUp = BATTERY_DEFAULT_HYST_UP,
                 float hystDown = BATTERY_DEFAULT_HYST_DOWN,
                 float tempCoeff = BATTERY_DEFAULT_TEMP_COEFF);

    uint8_t getBatteryType() const;
    void setBatteryType(uint8_t type, float currentVoltage);

    float update(float voltage, uint32_t epoch, float temperature = 25.0f);

    void loadFromNVS(float currentVoltage);
    void saveToNVS();
    void reset(float voltage = 0.0f);

    BatteryDebug_t getDebug() const;

private:
    float computeAlpha(float deltaTime) const;
    float evaluatePolynomial(float voltage) const;
    float clampPercent(float value) const;
    void clearState(float voltage);

private:
    Preferences _prefs;

    uint8_t _batteryType;
    float _tau;
    float _hystUp;
    float _hystDown;
    float _tempCoeff;

    float _filteredVoltage;
    float _lastPercent;
    float _calibrationOffset;
    float _lastVoltageRaw;

    uint32_t _lastTimestamp;
    bool _isCharging;

    BatteryDebug_t _debug;
};