/*
 * 12_Battery_Percent — reads VBAT and estimates state-of-charge using
 *                      the on-board BatteryEstimator algorithm.
 *
 * The estimator persists state in ESP32 NVS (Preferences), so the
 * filtered voltage and last percent survive a reboot.
 *
 * Change the battery type in beginBattery() if you have a LiFePO4 pack:
 *   SensBlueMonarch.beginBattery(BATTERY_TYPE_LIFEPO4);
 */

#include <SensBlueMonarch.h>

void setup() {
    Serial.begin(115200);
    if (!SensBlueMonarch.begin())         { Serial.println("Expander init failed."); while (true) delay(1000); }
    SensBlueMonarch.beginBattery(BATTERY_TYPE_LIION);
    Serial.println("Battery estimator ready.");
}

void loop() {
    float v = SensBlueMonarch.readBatteryVoltage();
    float p = SensBlueMonarch.readBatteryPercentage();
    Serial.printf("VBAT=%.3f V   SoC=%.0f %%\n", v, p);

    // Save to NVS every ~1 min in a real application. Here we save
    // every 10 samples for demo purposes.
    static uint32_t n = 0;
    if (++n % 10 == 0) SensBlueMonarch.battery.saveToNVS();

    delay(2000);
}
