/*
 * 05_RS485_Echo — echoes bytes between USB serial and RS485.
 *
 * The UART2 pins are shared between the RS485 transceiver and the piggyback
 * header. selectUart2() moves the mux to RS485. The RS485 driver enable is
 * driven by RS485_DE (GPIO18); we hold it high while transmitting and drop
 * it as soon as the last byte has cleared the shift register.
 */

#include <SensBlueMonarch.h>

// Adjust baud, protocol and half-duplex timing to your bus.
static constexpr uint32_t RS485_BAUD = 9600;
static constexpr uint32_t CHAR_BITS  = 10;   // start + 8 + stop
static constexpr uint32_t SETTLE_US  = (1000000UL * CHAR_BITS) / RS485_BAUD;

void setup() {
    Serial.begin(115200);
    if (!SensBlueMonarch.begin()) {
        Serial.println("Expander init failed.");
        while (true) delay(1000);
    }

    SensBlueMonarch.selectUart2(UART2_RS485);
    Serial2.begin(RS485_BAUD, SERIAL_8N1, PIN_SERIAL2_RX, PIN_SERIAL2_TX);
    SensBlueMonarch.rs485EndTx();   // start in receive
    Serial.println("RS485 echo ready.");
}

void loop() {
    // Anything from USB → RS485
    if (Serial.available()) {
        SensBlueMonarch.rs485BeginTx();
        while (Serial.available()) Serial2.write(Serial.read());
        Serial2.flush();
        delayMicroseconds(SETTLE_US);
        SensBlueMonarch.rs485EndTx();
    }
    // Anything from RS485 → USB
    while (Serial2.available()) Serial.write(Serial2.read());
}
