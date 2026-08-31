/*
 * 06_Modem_Info — powers on the BG95 and queries firmware / IMEI / signal
 *                 via raw AT commands over Serial1.
 *
 * For a full network stack (TCP / MQTT / HTTP) install TinyGSM and pass
 * Serial1 to it — SensBlueMonarch only manages power and pins here.
 */

#include <SensBlueMonarch.h>

static void sendAT(const char *cmd) {
    Serial1.print(cmd);
    Serial1.print("\r\n");
}

static void readAndPrintForMs(uint32_t ms) {
    uint32_t deadline = millis() + ms;
    while (millis() < deadline) {
        while (Serial1.available()) Serial.write(Serial1.read());
    }
}

void setup() {
    Serial.begin(115200);
    if (!SensBlueMonarch.begin()) {
        Serial.println("Expander init failed.");
        while (true) delay(1000);
    }

    Serial.println("Powering on modem...");
    SensBlueMonarch.modemPowerOn();
    delay(5000);

    Serial1.begin(115200, SERIAL_8N1, SB_SERIAL1_RX, SB_SERIAL1_TX);
    delay(500);
    readAndPrintForMs(500);

    sendAT("AT");                 readAndPrintForMs(500);
    sendAT("ATE0");               readAndPrintForMs(500);
    sendAT("AT+CGMM");            readAndPrintForMs(500);
    sendAT("AT+CGMR");            readAndPrintForMs(500);
    sendAT("AT+CGSN");            readAndPrintForMs(500);
    sendAT("AT+CSQ");             readAndPrintForMs(500);
    sendAT("AT+CPIN?");           readAndPrintForMs(500);
}

void loop() {
    while (Serial1.available()) Serial.write(Serial1.read());
    while (Serial.available())  Serial1.write(Serial.read());
}
