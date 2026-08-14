/*
 * 01_Blink_RGB — cycles the on-board RGB LED through primary colours.
 *
 * ATRONIA SensBlue Monarch — HW rev 2.41
 *
 * The RGB LED sits on the PCAL6416A I2C expander, not on a direct GPIO,
 * so we must call SensBlueMonarch.begin() before touching it.
 */

#include <SensBlueMonarch.h>

void setup() {
    Serial.begin(115200);
    delay(100);

    if (!SensBlueMonarch.begin()) {
        Serial.println("Expander not responding — check I2C wiring.");
        while (true) delay(1000);
    }

    Serial.println("SensBlue Monarch ready.");
}

void loop() {
    SensBlueMonarch.setRGB(true,  false, false); delay(400);   // red
    SensBlueMonarch.setRGB(false, true,  false); delay(400);   // green
    SensBlueMonarch.setRGB(false, false, true);  delay(400);   // blue
    SensBlueMonarch.setRGB(true,  true,  false); delay(400);   // yellow
    SensBlueMonarch.setRGB(false, true,  true);  delay(400);   // cyan
    SensBlueMonarch.setRGB(true,  false, true);  delay(400);   // magenta
    SensBlueMonarch.setRGB(true,  true,  true);  delay(400);   // white
    SensBlueMonarch.allLEDsOff();                delay(400);
}
