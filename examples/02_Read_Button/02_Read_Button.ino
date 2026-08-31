/*
 * 02_Read_Button — lights the green LED while the on-board button is pressed.
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
    SensBlueMonarch.setLED(SB_LED_GREEN, pressed);

    static bool wasPressed = false;
    if (pressed != wasPressed) {
        Serial.println(pressed ? "PRESSED" : "released");
        wasPressed = pressed;
    }
    delay(20);
}
