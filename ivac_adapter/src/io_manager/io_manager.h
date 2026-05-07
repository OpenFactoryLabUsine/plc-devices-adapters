#pragma once
#include <Arduino.h>

class IoManager {
public:
    static float analogReadVoltage(pin_size_t pin);
    static PinStatus digitalRead(pin_size_t pin);
    static void initPins();
};