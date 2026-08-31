/*
 * 07_BME280_Read — reads the on-board BME280 (I2C 0x77).
 * Prints temperature, humidity, pressure every 2 seconds.
 */

#include <SensBlueMonarch.h>

void setup() {
    Serial.begin(115200);
    delay(200);

    if (!SensBlueMonarch.begin()) {
        Serial.println("Expander init failed.");
        while (true) delay(1000);
    }

    if (!SensBlueMonarch.beginEnvSensor()) {
        Serial.println("BME280 not found at 0x77. Check hardware.");
        while (true) delay(1000);
    }
    Serial.println("BME280 ready.");
}

void loop() {
    Serial.printf("T=%.2f C  H=%.1f %%  P=%.2f hPa\n",
                  SensBlueMonarch.readTemperature(),
                  SensBlueMonarch.readHumidity(),
                  SensBlueMonarch.readPressure());
    delay(2000);
}
