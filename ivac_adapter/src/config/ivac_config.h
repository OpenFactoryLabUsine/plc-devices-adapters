#pragma once
#include <Arduino.h>
#include <array>

namespace IVACConfig {
    namespace Network {
        inline constexpr uint8_t     IP[4]      = {10, 68, 14, 15};
    }

    namespace OpcUa {
        inline constexpr const char* NAMESPACE_URI  = "lab-usine";
        inline constexpr const char* DEVICE_NAME    = "IVAC";

        inline constexpr std::array<const char*, 6> VARIABLE_NAMES = {
            "A1ToolPlus", "A2ToolPlus", "A3ToolPlus",
            "A1BlastGate", "A2BlastGate", "A3BlastGate"
        };

        inline constexpr std::array<const char*, 2> METHOD_NAMES = {
            "BuzzerControl",
            "SimulationMode"
        };
    }

    namespace Pins {
        inline constexpr std::array<pin_size_t, 3> TOOLPLUS = {A3, A4, A5};
        inline constexpr std::array<pin_size_t, 3> GATES    = {A0, A1, A2};
        inline constexpr pin_size_t LED_NORMAL  = LED_D0;
        inline constexpr pin_size_t LED_FAULT   = LED_D1;
        inline constexpr pin_size_t LED_WARNING = LED_D2;
    }

    namespace Timing {
        inline constexpr uint16_t HEARTBEAT_MS   = 10000;
        inline constexpr uint16_t TRANSITION_MS  = 3000;
    }
}