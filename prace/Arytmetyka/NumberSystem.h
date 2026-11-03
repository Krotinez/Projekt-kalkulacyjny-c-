#pragma once

#include <string>

enum class NumberSystem : int {
    BIN = 2,
    OCT = 8,
    DEC = 10,
    HEX = 16
};

namespace NumberSystems {
inline int base(NumberSystem system) {
    return static_cast<int>(system);
}
inline NumberSystem fromBase(int value) {
    switch (value) {
        case 2: return NumberSystem::BIN;
        case 8: return NumberSystem::OCT;
        case 16: return NumberSystem::HEX;
        default: return NumberSystem::DEC;
    }
}
inline bool digitAllowed(const std::string& label, NumberSystem system) {
    if (label.size() != 1 || label[0] < '0' || label[0] > '9')
        return true;
    return label[0] - '0' < base(system);
}
}
