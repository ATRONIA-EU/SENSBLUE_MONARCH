/*
 * 03_Digital_IO — mirrors isolated digital inputs on the isolated
 *                 digital outputs, and reports state changes on Serial.
 */

#include <SensBlueMonarch.h>

void setup() {
    Serial.begin(115200);
    if (!SensBlueMonarch.begin()) {
        Serial.println("Expander init failed.");
        while (true) delay(1000);
    }
}

void loop() {
    bool in1 = SensBlueMonarch.readDigitalInput(1);
    bool in2 = SensBlueMonarch.readDigitalInput(2);

    SensBlueMonarch.setDigitalOutput(1, in1);
    SensBlueMonarch.setDigitalOutput(2, in2);

    static bool prev1 = false, prev2 = false;
    if (in1 != prev1 || in2 != prev2) {
        Serial.printf("IN1=%d IN2=%d\n", in1, in2);
        prev1 = in1; prev2 = in2;
    }
    delay(20);
}
