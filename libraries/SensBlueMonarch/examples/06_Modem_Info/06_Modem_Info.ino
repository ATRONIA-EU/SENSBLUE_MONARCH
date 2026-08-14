/*
 * 06_Modem_Info — powers on the BG95 and queries its firmware / IMEI /
 *                 signal quality via raw AT commands over Serial1.
 *
 * For a full network stack (TCP / MQTT / HTTP) install TinyGSM and pass
 * Serial1 to it — the SensBlueMonarch library only manages power sequen-
 * cing and level-shifting.
 *
 * The BG95 supports a wide input voltage range but current spikes at TX
 * can exceed 2 A. Make sure your battery / supply can source that.
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

    // Bring up the modem
    Serial.println("Powering on modem...");
    SensBlueMonarch.modemPowerOn();
    delay(5000);   // BG95 boot takes ~4-5 s

    Serial1.begin(115200, SERIAL_8N1, PIN_SERIAL1_RX, PIN_SERIAL1_TX);
    delay(500);
    readAndPrintForMs(500);       // flush URCs

    sendAT("AT");                 readAndPrintForMs(500);
    sendAT("ATE0");               readAndPrintForMs(500);
    sendAT("AT+CGMM");            readAndPrintForMs(500);   // model
    sendAT("AT+CGMR");            readAndPrintForMs(500);   // firmware
    sendAT("AT+CGSN");            readAndPrintForMs(500);   // IMEI
    sendAT("AT+CSQ");             readAndPrintForMs(500);   // signal
    sendAT("AT+CPIN?");           readAndPrintForMs(500);   // SIM state
}

void loop() {
    while (Serial1.available()) Serial.write(Serial1.read());
    while (Serial.available())  Serial1.write(Serial.read());
}
