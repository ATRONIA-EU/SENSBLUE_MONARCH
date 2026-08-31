/*
 * 10_EEPROM_Storage — writes and reads back a small struct to the
 *                     on-board AT24C02 (256 bytes) at address 0.
 *
 * Increment counter persists across reboots.
 */

#include <SensBlueMonarch.h>

struct Config {
    uint32_t magic;
    uint32_t bootCount;
    char     name[16];
};

const uint32_t MAGIC = 0x5EB1AE01;

void setup() {
    Serial.begin(115200);
    delay(200);
    if (!SensBlueMonarch.begin())      { Serial.println("Expander init failed."); while (true) delay(1000); }
    if (!SensBlueMonarch.beginEeprom()) { Serial.println("EEPROM not responding."); while (true) delay(1000); }

    Config cfg{};
    SensBlueMonarch.eepromRead(0, (uint8_t *)&cfg, sizeof(cfg));

    if (cfg.magic != MAGIC) {
        Serial.println("First boot — initialising EEPROM.");
        cfg.magic = MAGIC;
        cfg.bootCount = 0;
        strncpy(cfg.name, "monarch-01", sizeof(cfg.name));
    }

    cfg.bootCount++;
    SensBlueMonarch.eepromWrite(0, (const uint8_t *)&cfg, sizeof(cfg));

    Serial.printf("Name: %s  bootCount: %u\n", cfg.name, cfg.bootCount);
}

void loop() {}
