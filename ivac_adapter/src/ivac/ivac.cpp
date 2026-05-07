#include "ivac.h"
#include "io_manager/io_manager.h"

DeviceManager::DeviceManager() : _buzzerState(BuzzerState::UNKNOWN), _simulationMode(false) {
    IoManager::initPins();
}

void DeviceManager::setBuzzer(BuzzerState state) {
    _buzzerState = state;

    switch (state) {
        case BuzzerState::NORMAL:
            digitalWrite(NORMAL_PIN, HIGH);
            digitalWrite(FAULT_PIN, LOW);
            digitalWrite(WARNING_PIN, LOW);
            break;
        case BuzzerState::FAULT:
            digitalWrite(NORMAL_PIN, LOW);
            digitalWrite(FAULT_PIN, HIGH);
            digitalWrite(WARNING_PIN, LOW);
            break;
        case BuzzerState::WARNING:
            digitalWrite(NORMAL_PIN, LOW);
            digitalWrite(FAULT_PIN, LOW);
            digitalWrite(WARNING_PIN, HIGH);
            break;
        default:
            digitalWrite(NORMAL_PIN, LOW);
            digitalWrite(FAULT_PIN, LOW);
            digitalWrite(WARNING_PIN, LOW);
            break;
    }
}

BuzzerState DeviceManager::getBuzzerState() const {
    return _buzzerState;
}

bool DeviceManager::isSimulationMode() const {
    return _simulationMode;
}

void DeviceManager::setSimulationMode(bool enabled) {
    _simulationMode = enabled;
}

String DeviceManager::getToolState(uint8_t index) const {
    if (index < TOOLPLUS_NB) {
        return _toolState[index];
    }
    return "Invalid Index";
}

String DeviceManager::getGateState(uint8_t index) const {
    if (index < GATE_NB) {
        return _gateState[index];
    }
    return "Invalid Index";
}

void DeviceManager::updateStates() {
    for (uint8_t i = 0; i < TOOLPLUS_NB; i++) {
        _toolState[i] = IoManager::digitalRead(TOOLPLUS_PINS[i]) == HIGH ? "ON" : "OFF";
    }
    for (uint8_t i = 0; i < GATE_NB; i++) {
        _gateState[i] = IoManager::digitalRead(GATE_PINS[i]) == HIGH ? "OPEN" : "CLOSED";
    }
}

BuzzerState DeviceManager::parseCommand(const String& cmd) {
    if (cmd.equalsIgnoreCase("NORMAL")) {
        return BuzzerState::NORMAL;
    } else if (cmd.equalsIgnoreCase("FAULT")) {
        return BuzzerState::FAULT;
    } else if (cmd.equalsIgnoreCase("WARNING")) {
        return BuzzerState::WARNING;
    }
    return BuzzerState::UNKNOWN;
}

String DeviceManager::buzzerStateToString(BuzzerState state) {
    switch (state) {
        case BuzzerState::NORMAL: return "NORMAL";
        case BuzzerState::FAULT: return "FAULT";
        case BuzzerState::WARNING: return "WARNING";
        default: return "UNKNOWN";
    }
}