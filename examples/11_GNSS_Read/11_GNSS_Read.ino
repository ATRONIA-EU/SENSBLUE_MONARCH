/*
 * 11_GNSS_Read — enables the BG95 built-in GNSS receiver and prints
 *                raw NMEA to Serial.
 *
 * Steps performed:
 *   1. Boot the BG95 modem (needs to be on for GNSS to power).
 *   2. Route Serial2 through the on-board GNSS mux (GPS_EN + USART_SEL).
 *   3. Send AT+QGPS=1 to the modem UART to start GNSS.
 *   4. Read NMEA sentences from Serial2 and echo them.
 *
 * For parsed positions install TinyGPSPlus and feed it Serial2 bytes.
 */

#include <SensBlueMonarch.h>

void setup() {
    Serial.begin(115200);
    if (!SensBlueMonarch.begin()) { Serial.println("Expander init failed."); while (true) delay(1000); }

    Serial.println("Powering modem...");
    SensBlueMonarch.modemPowerOn();
    delay(5000);

    // Modem AT command port
    Serial1.begin(115200, SERIAL_8N1, SB_SERIAL1_RX, SB_SERIAL1_TX);
    delay(200);
    Serial1.println("ATE0");        delay(200);
    Serial1.println("AT+QGPS=1");   delay(500);

    // Route GNSS UART to Serial2
    SensBlueMonarch.enableGNSS(true);
    Serial2.begin(9600, SERIAL_8N1, SB_SERIAL2_RX, SB_SERIAL2_TX);

    Serial.println("Streaming NMEA from Serial2...");
}

void loop() {
    while (Serial2.available()) Serial.write(Serial2.read());
}
