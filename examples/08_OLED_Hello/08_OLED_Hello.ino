/*
 * 08_OLED_Hello — draws "Hello" on the OLED and demonstrates direct
 *                 access to the underlying Adafruit_SSD1306 object.
 */

#include <SensBlueMonarch.h>

void setup() {
    Serial.begin(115200);
    if (!SensBlueMonarch.begin())        { Serial.println("Expander init failed."); while (true) delay(1000); }
    if (!SensBlueMonarch.beginDisplay()) { Serial.println("OLED not found at 0x3C.");  while (true) delay(1000); }

    // 128x32 OLED — 4 lines of size-1 or 2 lines of size-2.
    SensBlueMonarch.display.clearDisplay();
    SensBlueMonarch.display.setTextSize(2);
    SensBlueMonarch.display.setCursor(0, 0);
    SensBlueMonarch.display.println("Hello!");
    SensBlueMonarch.display.setTextSize(1);
    SensBlueMonarch.display.setCursor(0, 24);
    SensBlueMonarch.display.println("SensBlue Monarch");
    SensBlueMonarch.display.display();
}

void loop() {}
