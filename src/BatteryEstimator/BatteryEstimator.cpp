#include "BatteryEstimator.h"
#include <math.h>


BatteryEstimator::BatteryEstimator(uint8_t batteryType,
                                   float tau,
                                   float hystUp,
                                   float hystDown,
                                   float tempCoeff)
    : _batteryType(batteryType),
      _tau(tau),
      _hystUp(hystUp),
      _hystDown(hystDown),
      _tempCoeff(tempCoeff),
      _filteredVoltage(0.0f),
      _lastPercent(-1.0f),
      _calibrationOffset(0.0f),
      _lastVoltageRaw(0.0f),
      _lastTimestamp(0),
      _isCharging(false)
{
    memset(&_debug, 0, sizeof(_debug));
}

uint8_t BatteryEstimator::getBatteryType() const
{
    return _batteryType;
}

void BatteryEstimator::setBatteryType(uint8_t type, float currentVoltage)
{
    if (type == _batteryType)
        return;



    // ---- trocar tipo ----
    _batteryType = type;

    // ---- reset controlado ----
    _filteredVoltage = currentVoltage;
    _lastPercent = -1.0f;
    _calibrationOffset = 0.0f;
    _lastVoltageRaw = currentVoltage;
    _lastTimestamp = 0;
    _isCharging = false;

    // opcional: limpar debug
    memset(&_debug, 0, sizeof(_debug));
}

float BatteryEstimator::computeAlpha(float deltaTime) const
{
    if (deltaTime <= 0.0f)
        return 1.0f;

    float alpha = 1.0f - expf(-deltaTime / _tau);

    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;

    return alpha;
}

float BatteryEstimator::evaluatePolynomial(float v) const
{
    const float* coeffs;

    static const float liion[] = {
        LIION_COEFF_0,
        LIION_COEFF_1,
        LIION_COEFF_2,
        LIION_COEFF_3,
        LIION_COEFF_4
    };

    static const float lifepo4[] = {
        LIFEPO4_COEFF_0,
        LIFEPO4_COEFF_1,
        LIFEPO4_COEFF_2,
        LIFEPO4_COEFF_3,
        LIFEPO4_COEFF_4
    };

    if (_batteryType == BATTERY_TYPE_LIFEPO4)
        coeffs = lifepo4;
    else
        coeffs = liion;

    float result = coeffs[0];

    for (int i = 1; i < 5; i++)
    {
        result = result * v + coeffs[i];
    }

    return result;
}

float BatteryEstimator::clampPercent(float value) const
{
    if (value > 100.0f) return 100.0f;
    if (value < 0.0f)   return 0.0f;
    return value;
}

void BatteryEstimator::clearState(float voltage)
{
    _filteredVoltage = voltage;
    _lastPercent = -1.0f;
    _calibrationOffset = 0.0f;
    _lastVoltageRaw = voltage;
    _lastTimestamp = 0;
    _isCharging = false;
}

void BatteryEstimator::reset(float voltage)
{
    clearState(voltage);
    memset(&_debug, 0, sizeof(_debug));
}

void BatteryEstimator::loadFromNVS(float currentVoltage)
{
    BatteryStateNVS state;
    bool validState = false;

    memset(&state, 0, sizeof(state));

    _prefs.begin(BATTERY_NVS_NAMESPACE, true);

    size_t len = _prefs.getBytesLength(BATTERY_NVS_KEY);
    if (len == sizeof(BatteryStateNVS))
    {
        size_t readLen = _prefs.getBytes(BATTERY_NVS_KEY, &state, sizeof(state));
        if (readLen == sizeof(BatteryStateNVS) && state.initialized)
        {
            validState = true;
        }
    }

    _prefs.end();

    // Caso 1: NVS vazia, inválida ou corrompida
    if (!validState)
    {
        clearState(currentVoltage);
        return;
    }

    // Caso 2: provável troca de bateria
    float deltaV = fabsf(currentVoltage - state.filteredVoltage);
    bool batterySwapped =
        (deltaV > BATTERY_SWAP_THRESHOLD) &&
        (currentVoltage > state.filteredVoltage);

    if (batterySwapped)
    {
        clearState(currentVoltage);
        return;
    }

    // Regular case
    _batteryType = state.batteryType;
    if (state.batteryType > BATTERY_TYPE_LIFEPO4)
        _batteryType = BATTERY_TYPE_LIION;

    _filteredVoltage = state.filteredVoltage;
    _lastPercent = state.lastPercent;
    _calibrationOffset = state.calibrationOffset;
    _lastVoltageRaw = state.lastVoltageRaw;
    _lastTimestamp = state.lastTimestamp;
    _isCharging = state.isCharging;
}

void BatteryEstimator::saveToNVS()
{
    BatteryStateNVS state;

    state.batteryType = _batteryType;
    state.filteredVoltage = _filteredVoltage;
    state.lastPercent = _lastPercent;
    state.calibrationOffset = _calibrationOffset;
    state.lastVoltageRaw = _lastVoltageRaw;
    state.lastTimestamp = _lastTimestamp;
    state.isCharging = _isCharging;
    state.initialized = true;

    _prefs.begin(BATTERY_NVS_NAMESPACE, false);
    _prefs.putBytes(BATTERY_NVS_KEY, &state, sizeof(state));
    _prefs.end();
}

float BatteryEstimator::update(float voltage, uint32_t epoch, float temperature)
{
    _debug.rawVoltage = voltage;
    _debug.temperature = temperature;

    float dt;
    if (_lastTimestamp > 0 && epoch > _lastTimestamp)
        dt = (float)(epoch - _lastTimestamp);
    else
        dt = _tau;

    float alpha = computeAlpha(dt);

    // EMA dependente do tempo
    if (_filteredVoltage == 0.0f)
        _filteredVoltage = voltage;
    else
        _filteredVoltage = (1.0f - alpha) * _filteredVoltage + alpha * voltage;

    _debug.filteredVoltage = _filteredVoltage;

    float dv = voltage - _lastVoltageRaw;
    _debug.deltaVoltage = dv;

    // Deteção de charging/discharging
    if (dv > BATTERY_CHARGE_DELTA)
        _isCharging = true;
    else if (dv < -BATTERY_CHARGE_DELTA)
        _isCharging = false;

    _debug.isCharging = _isCharging;

    // Compensação por temperatura
    float tempCompVoltage = _filteredVoltage + (_tempCoeff * (temperature - 25.0f));

    // Load compensation
    float compensatedVoltage = tempCompVoltage;
    if (!_isCharging && fabsf(dv) > BATTERY_DIP_THRESHOLD)
    {
        compensatedVoltage += BATTERY_LOAD_COMPENSATION;
    }

    _debug.compensatedVoltage = compensatedVoltage;

    // Polinómio
    float newPercent = evaluatePolynomial(compensatedVoltage);
    _debug.percentRaw = newPercent;

    float fullV, emptyV;

    if (_batteryType == BATTERY_TYPE_LIFEPO4)
    {
        fullV = LIFEPO4_FULL_VOLTAGE;
        emptyV = LIFEPO4_EMPTY_VOLTAGE;
    }
    else
    {
        fullV = LIION_FULL_VOLTAGE;
        emptyV = LIION_EMPTY_VOLTAGE;
    }

    // Auto-calibração por eventos físicos
    if (compensatedVoltage >= fullV)
    {
        _calibrationOffset = 100.0f - newPercent;
    }
    else if (compensatedVoltage <= emptyV)
    {
        _calibrationOffset = 0.0f - newPercent;
    }

    newPercent += _calibrationOffset;
    newPercent = clampPercent(newPercent);

    // Histerese adaptativa
    if (_lastPercent < 0.0f)
    {
        _lastPercent = newPercent;
    }
    else
    {
        float diff = newPercent - _lastPercent;

        if (_isCharging)
        {
            if (diff >= _hystUp)
                _lastPercent = newPercent;
        }
        else
        {
            if (fabsf(diff) >= _hystDown)
                _lastPercent = newPercent;
        }
    }

    _lastVoltageRaw = voltage;
    _lastTimestamp = epoch;

    _debug.percentFinal = _lastPercent;
    _debug.calibrationOffset = _calibrationOffset;
    _debug.lastTimestamp = epoch;

    return _lastPercent;
}

BatteryDebug_t BatteryEstimator::getDebug() const
{
    return _debug;
}
