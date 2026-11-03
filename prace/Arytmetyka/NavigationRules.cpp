#include "NavigationRules.h"

namespace NavigationRules {
std::optional<std::string> explicitTarget(const std::string& label,
                                          const std::string& action,
                                          sf::Keyboard::Key direction) {
    using Key = sf::Keyboard::Key;
    if (direction == Key::Right) {
        if (label == "WYNIKI v") return "BIN";
        if (label == "1/x") return "(";
        if (label == "-x") return "[";
        if (label == "←") return "{";
        if (label == "C" && action == "CLEAR") return "|";
        if (label == "1/2") return "%";
        if (label == "F") return "A";
        if (label == "ESC") return "BIN";
    } else if (direction == Key::Left) {
        if (label == "BIN") return "WYNIKI v";
        if (label == "1/x") return "÷";
        if (label == "-x") return "×";
        if (label == "←") return "-";
        if (label == "C" && action == "CLEAR") return "+";
        if (label == "(") return "1/x";
        if (label == "[") return "-x";
        if (label == "{") return "←";
        if (label == "|" && action == "ABS_OPEN") return "C";
        if (label == "%") return "1/2";
        if (label == "A") return "F";
    } else if (direction == Key::Down) {
        if (label == "BIN") return "NWD";
        if (label == "NWD") return "NWW";
        if (label == "NWW") return "(";
        if (label == "1/2" || (label == "C" && action == "CLEAR")) return "1/x";
        if (label.size() == 1 && label[0] >= 'A' && label[0] <= 'F' &&
            action.rfind("HEX_", 0) == 0) {
            const char* targets[] = {"(", ")", "7", "8", "9", "÷"};
            return targets[label[0] - 'A'];
        }
    } else if (direction == Key::Up) {
        if (label == "1/2") return "C";
    }
    return std::nullopt;
}
}
