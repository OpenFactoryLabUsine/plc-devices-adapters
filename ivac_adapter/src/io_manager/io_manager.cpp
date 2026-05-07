#include "io_manager.h"
#include <Arduino_OPC_UA.h>

static constexpr float VOLTAGE_MAX   = 3.3f;
static constexpr float RESOLUTION    = 4096.0f;
static constexpr float DIVIDER       = 0.3034f;
static constexpr float DIGITAL_THRESHOLD = 5.0f;

static const pin_size_t TOOLPLUS_PINS[] = {A3, A4, A5};
static const pin_size_t GATE_PINS[]     = {A0, A1, A2};

void IoManager::initPins() {
    for (auto pin : TOOLPLUS_PINS) pinMode(pin, INPUT);
    for (auto pin : GATE_PINS)     pinMode(pin, INPUT);

    pinMode(LED_D0, OUTPUT);
    pinMode(LED_D1, OUTPUT);
    pinMode(LED_D2, OUTPUT);
}

float IoManager::analogReadVoltage(pin_size_t pin) {
    return analogRead(pin) * (VOLTAGE_MAX / RESOLUTION) / DIVIDER;
}

PinStatus IoManager::digitalRead(pin_size_t pin) {
    return analogReadVoltage(pin) > DIGITAL_THRESHOLD ? HIGH : LOW;
}