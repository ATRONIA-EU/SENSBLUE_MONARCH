/*
 * 04_Analog_Inputs — reads the two analog inputs, alternating between
 *                    0-10 V and 4-20 mA modes every 5 seconds.
 *
 * REMINDER: the analog mux is shared. Both channels are in the same mode
 * at any given time. When you switch to 4-20 mA, the on-board +12 V boost
 * is also enabled to feed the loop; the library handles that automatically.
 *
 * Divider constants live in variants/sensblue_monarch/pins_arduino.h and
 * should be validated against your specific board population before trust-
 * ing the returned engineering units.
 */

#include <SensBlueMonarch.h>

void setup() {
    Serial.begin(115200);
    if (!SensBlueMonarch.begin()) {
        Serial.println("Expander init failed.");
        while (true) delay(1000);
    }
    analogReadResolution(12);       // 0..4095 (default on ESP32, explicit here)
    // The ESP32 ADCs are notoriously non-linear near the rails; consider
    // calling analogSetPinAttenuation() and/or using esp_adc_cal for
    // production-grade measurements.
}

void loop() {
    // ---- 0-10 V mode
    SensBlueMonarch.setAnalogInputMode(ANALOG_MODE_0_10V);
    delay(50);   // let the mux settle
    Serial.printf("[0-10V] ch1=%.3f V  ch2=%.3f V\n",
                  SensBlueMonarch.readAnalog(1),
                  SensBlueMonarch.readAnalog(2));
    delay(5000);

    // ---- 4-20 mA mode (boost engages automatically)
    SensBlueMonarch.setAnalogInputMode(ANALOG_MODE_4_20MA);
    delay(50);
    Serial.printf("[4-20mA] ch1=%.3f mA ch2=%.3f mA\n",
                  SensBlueMonarch.readAnalog(1),
                  SensBlueMonarch.readAnalog(2));
    delay(5000);
}
