#include "IVAC.h"
#include "config/ivac_config.h"
#include "io_manager/io_manager.h"

IVAC::IVAC()
    : _buzzerState(BuzzerState::WARNING), _simulationMode(false) {}

void IVAC::setBuzzer(BuzzerState state) {
    if (_simulationMode) return;

    _buzzerState = state;

    digitalWrite(LED_D0, state == BuzzerState::NORMAL  ? HIGH : LOW);
    digitalWrite(LED_D1, state == BuzzerState::FAULT   ? HIGH : LOW);
    digitalWrite(LED_D2, state == BuzzerState::WARNING ? HIGH : LOW);
}

BuzzerState IVAC::getBuzzerState() const { return _buzzerState; }
bool IVAC::isSimulationMode() const       { return _simulationMode; }

void IVAC::setSimulationMode(bool enabled) {
    _simulationMode = enabled;
}

void IVAC::updateStates() {
    for (uint8_t i = 0; i < TOOLPLUS_NB; i++)
        _toolState[i] = IoManager::digitalRead(IVACConfig::Pins::TOOLPLUS[i]) == HIGH ? "OFF" : "ON";

    for (uint8_t i = 0; i < GATE_NB; i++)
        _gateState[i] = IoManager::digitalRead(IVACConfig::Pins::GATES[i]) ? "CLOSED" : "OPEN";
}

String IVAC::getToolState(uint8_t i) const { return _toolState[i]; }
String IVAC::getGateState(uint8_t i) const { return _gateState[i]; }

BuzzerState IVAC::parseBuzzerCommand(const String& cmd) {
    if (cmd.equalsIgnoreCase("NORMAL"))  return BuzzerState::NORMAL;
    if (cmd.equalsIgnoreCase("FAULT"))   return BuzzerState::FAULT;
    if (cmd.equalsIgnoreCase("WARNING")) return BuzzerState::WARNING;
    return BuzzerState::UNKNOWN;
}

String IVAC::buzzerStateToString(BuzzerState state) {
    switch (state) {
        case BuzzerState::NORMAL:  return "NORMAL";
        case BuzzerState::FAULT:   return "FAULT";
        case BuzzerState::WARNING: return "WARNING";
        default:                   return "UNKNOWN";
    }
}