#pragma once
#include <Arduino.h>

enum class BuzzerState { NORMAL, FAULT, WARNING, UNKNOWN };

class DeviceManager {
public:
    DeviceManager();

    void setBuzzer(BuzzerState state);
    BuzzerState getBuzzerState() const;

    bool isSimulationMode() const;
    void setSimulationMode(bool enabled);

    String getToolState(uint8_t index) const;
    String getGateState(uint8_t index) const;
    void updateStates();

    static BuzzerState parseCommand(const String& cmd);
    static String buzzerStateToString(BuzzerState state);

    static constexpr uint8_t TOOLPLUS_NB = 3;
    static constexpr uint8_t GATE_NB     = 3;

private:
    BuzzerState _buzzerState;
    bool _simulationMode;
    String _toolState[TOOLPLUS_NB];
    String _gateState[GATE_NB];