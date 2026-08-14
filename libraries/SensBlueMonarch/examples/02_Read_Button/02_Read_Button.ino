/*
 * 02_Read_Button — lights the green LED while the on-board button is pressed.
 *
 * The button is on GPIO4 with an internal pull-up; pressing pulls the pin
 * to ground. The library's readButton() inverts that so pressed == true.
 */

#include <SensBlueMonarch.h>

void setup() {
    Serial.begin(115200);
    if (!SensBlueMonarch.begin()) {
        Serial.println("Expander init failed.");
        while (true) delay(1000);
    }
    Serial.println("Press the on-board button.");
}

void loop() {
    bool pressed = SensBlueMonarch.readButton();
    SensBlueMonarch.setLED(LED_GREEN, pressed);

    static bool wasPressed = false;
    if (pressed != wasPressed) {
        Serial.println(pressed ? "PRESSED" : "released");
        wasPressed = pressed;
    }
    delay(20);
}
