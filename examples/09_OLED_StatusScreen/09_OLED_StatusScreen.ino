/*
 * 09_OLED_StatusScreen — refreshes the built-in status screen every
 *                        second. Shows battery percentage/voltage,
 *                        temperature, humidity and uptime.
 *
 * If BME280 or battery estimator aren't initialised the status screen
 * silently skips those lines, so this example also works without them.
 */

#include <SensBlueMonarch.h>

void setup() {
    Serial.begin(115200);
    if (!SensBlueMonarch.begin())        { Serial.println("Expander init failed."); while (true) delay(1000); }
    if (!SensBlueMonarch.beginDisplay()) { Serial.println("OLED init failed.");     while (true) delay(1000); }

    SensBlueMonarch.beginEnvSensor();    // optional
    SensBlueMonarch.beginBattery();      // optional (Li-ion default)
}

void loop() {
    SensBlueMonarch.showStatusScreen();  // header defaults to "SensBlue Monarch"
    delay(1000);
}
