/*
 * 04_Analog_Inputs — reads the two analog inputs, alternating between
 *                    0-10 V and 4-20 mA modes every 5 seconds.
 */

#include <SensBlueMonarch.h>

void setup() {
    Serial.begin(115200);
    if (!SensBlueMonarch.begin()) {
        Serial.println("Expander init failed.");
        while (true) delay(1000);
    }
    analogReadResolution(12);
}

void loop() {
    SensBlueMonarch.setAnalogInputMode(SB_ANALOG_MODE_0_10V);
    delay(50);
    Serial.printf("[0-10V] ch1=%.3f V  ch2=%.3f V\n",
                  SensBlueMonarch.readAnalog(1),
                  SensBlueMonarch.readAnalog(2));
    delay(5000);

    SensBlueMonarch.setAnalogInputMode(SB_ANALOG_MODE_4_20MA);
    delay(50);
    Serial.printf("[4-20mA] ch1=%.3f mA ch2=%.3f mA\n",
                  SensBlueMonarch.readAnalog(1),
                  SensBlueMonarch.readAnalog(2));
    delay(5000);
}
