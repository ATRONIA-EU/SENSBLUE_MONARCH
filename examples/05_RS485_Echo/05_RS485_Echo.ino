/*
 * 05_RS485_Echo — echoes bytes between USB serial and RS485.
 */

#include <SensBlueMonarch.h>

static constexpr uint32_t RS485_BAUD = 9600;
static constexpr uint32_t CHAR_BITS  = 10;
static constexpr uint32_t SETTLE_US  = (1000000UL * CHAR_BITS) / RS485_BAUD;

void setup() {
    Serial.begin(115200);
    if (!SensBlueMonarch.begin()) {
        Serial.println("Expander init failed.");
        while (true) delay(1000);
    }

    SensBlueMonarch.selectUart2(SB_UART2_RS485);
    Serial2.begin(RS485_BAUD, SERIAL_8N1, SB_SERIAL2_RX, SB_SERIAL2_TX);
    SensBlueMonarch.rs485EndTx();
    Serial.println("RS485 echo ready.");
}

void loop() {
    if (Serial.available()) {
        SensBlueMonarch.rs485BeginTx();
        while (Serial.available()) Serial2.write(Serial.read());
        Serial2.flush();
        delayMicroseconds(SETTLE_US);
        SensBlueMonarch.rs485EndTx();
    }
    while (Serial2.available()) Serial.write(Serial2.read());
}
