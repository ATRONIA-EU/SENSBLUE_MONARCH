/*
 * 06_Modem_Info — powers on the BG95 and queries firmware/IMEI/signal
 *                 via raw AT commands over Serial1.
 *
 * Sequence mirrors the ATRONIA FW GSM_TurnON():
 *   1. Serial1.begin() with modem baudrate FIRST.
 *   2. modemPowerOn() enables VBAT_2, waits 1 s, pulses POWERKEY 1.1 s.
 *   3. Wait a few seconds for the modem to boot.
 *   4. Send AT commands.
 *
 * For a full stack (TCP/HTTP/MQTT) install TinyGSM and pass Serial1 to it.
 *
 * Hardware rev 2.41 pin mapping:
 *   ESP32 IO25 → BG95 RXD_GSM   (ESP32 TX)
 *   ESP32 IO33 ← BG95 TXD_GSM   (ESP32 RX)
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
    delay(200);

    if (!SensBlueMonarch.begin()) {
        Serial.println("Expander init failed.");
        while (true) delay(1000);
    }

    // 1. Open Serial1 to the modem BEFORE powering it up.
    Serial1.begin(115200, SERIAL_8N1, SB_SERIAL1_RX, SB_SERIAL1_TX);

    // 2. Power on: enables VBAT_2, waits, pulses POWERKEY.
    Serial.println("Powering on modem (this takes ~5 s)...");
    SensBlueMonarch.modemPowerOn();

    // 3. Give the modem time to boot and produce URCs.
    delay(5000);
    readAndPrintForMs(500);

    // 4. Basic identity queries.
    sendAT("AT");                 readAndPrintForMs(500);
    sendAT("ATE0");               readAndPrintForMs(500);
    sendAT("AT+CGMM");            readAndPrintForMs(500);   // model
    sendAT("AT+CGMR");            readAndPrintForMs(500);   // firmware
    sendAT("AT+CGSN");            readAndPrintForMs(500);   // IMEI
    sendAT("AT+CSQ");             readAndPrintForMs(500);   // signal quality
    sendAT("AT+CPIN?");           readAndPrintForMs(500);   // SIM state
}

void loop() {
    while (Serial1.available()) Serial.write(Serial1.read());
    while (Serial.available())  Serial1.write(Serial.read());
}
