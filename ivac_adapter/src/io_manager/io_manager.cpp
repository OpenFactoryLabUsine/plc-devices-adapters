#include "io_manager.h"
#include "config/ivac_config.h"
#include <Arduino_OPC_UA.h>

static constexpr float VOLTAGE_MAX   = 3.3f;
static constexpr float RESOLUTION    = 4096.0f;
static constexpr float DIVIDER       = 0.3034f;
static constexpr float DIGITAL_THRESHOLD = 5.0f;

void IoManager::initPins() {
    for (auto pin : IVACConfig::Pins::TOOLPLUS) pinMode(pin, INPUT);
    for (auto pin : IVACConfig::Pins::GATES)     pinMode(pin, INPUT);

    pinMode(IVACConfig::Pins::LED_NORMAL, OUTPUT);
    pinMode(IVACConfig::Pins::LED_FAULT, OUTPUT);
    pinMode(IVACConfig::Pins::LED_WARNING, OUTPUT);
}

float IoManager::analogReadVoltage(pin_size_t pin) {
    return analogRead(pin) * (VOLTAGE_MAX / RESOLUTION) / DIVIDER;
}

PinStatus IoManager::digitalRead(pin_size_t pin) {
    return analogReadVoltage(pin) > DIGITAL_THRESHOLD ? HIGH : LOW;
}