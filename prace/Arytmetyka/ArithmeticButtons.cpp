#include "ArithmeticButtons.h"
#include <utility>

ArithmeticButtons createArithmeticButtons(sf::Font& font, sf::Font& mathFont)
{
    std::vector<ButtonDef> defs = {
        // Górne przyciski
        { "ESC", {20.f, 20.f}, ButtonType::Medium, sf::Keyboard::Key::Escape },
        { "BIN", {120.f, 120.f}, ButtonType::Medium },
        { "OCT", {220.f, 120.f}, ButtonType::Medium },
        { "DEC", {320.f, 120.f}, ButtonType::Medium },
        { "HEX", {420.f, 120.f}, ButtonType::Medium },
        { "W",   {810.f, 120.f}, ButtonType::Medium },
        { "K",   {910.f, 120.f}, ButtonType::Medium },

        // Przycisk WYNIKI (Dropdown)
        { "WYNIKI v", {1010.f, 120.f}, ButtonType::Dropdown },

        // Lewe funkcje
        { "NWD", {120.f, 180.f}, ButtonType::Medium },
        { "NWW", {120.f, 235.f}, ButtonType::Medium },

        // Prawe funkcje (wyrównane do prawej krawędzi pola tekstowego X = 928)
        { "1/x", {928.f, 430.f}, ButtonType::Medium },
        { "-x",  {928.f, 483.f}, ButtonType::Medium },
        { "←",   {928.f, 536.f}, ButtonType::Medium, sf::Keyboard::Key::Backspace },
        { "C", {928.f, 589.f}, ButtonType::Medium, std::nullopt, "CLEAR" },

        // Nawiasy
        { "(", {300.f, 430.f}, ButtonType::Small }, { ")", {368.f, 430.f}, ButtonType::Small },
        { "[", {300.f, 483.f}, ButtonType::Small }, { "]", {368.f, 483.f}, ButtonType::Small },
        { "{", {300.f, 536.f}, ButtonType::Small }, { "}", {368.f, 536.f}, ButtonType::Small },
        { "|", {300.f, 589.f}, ButtonType::Small, std::nullopt, "ABS_OPEN" }, { "|", {368.f, 589.f}, ButtonType::Small, std::nullopt, "ABS_CLOSE" },

        // Cyfry
        { "7", {436.f, 430.f}, ButtonType::Small, sf::Keyboard::Key::Num7 },
        { "8", {504.f, 430.f}, ButtonType::Small, sf::Keyboard::Key::Num8 },
        { "9", {572.f, 430.f}, ButtonType::Small, sf::Keyboard::Key::Num9 },
        { "4", {436.f, 483.f}, ButtonType::Small, sf::Keyboard::Key::Num4 },
        { "5", {504.f, 483.f}, ButtonType::Small, sf::Keyboard::Key::Num5 },
        { "6", {572.f, 483.f}, ButtonType::Small, sf::Keyboard::Key::Num6 },
        { "1", {436.f, 536.f}, ButtonType::Small, sf::Keyboard::Key::Num1 },
        { "2", {504.f, 536.f}, ButtonType::Small, sf::Keyboard::Key::Num2 },
        { "3", {572.f, 536.f}, ButtonType::Small, sf::Keyboard::Key::Num3 },
        { "0", {436.f, 589.f}, ButtonType::Small, sf::Keyboard::Key::Num0 },
        { ",", {504.f, 589.f}, ButtonType::Small },
        { "=", {572.f, 589.f}, ButtonType::Small, sf::Keyboard::Key::Enter },

        // Operatory podstawowe
        { "÷", {640.f, 430.f}, ButtonType::Small },
        { "×", {640.f, 483.f}, ButtonType::Small },
        { "-", {640.f, 536.f}, ButtonType::Small },
        { "+", {640.f, 589.f}, ButtonType::Small },

        // Zaawansowane operatory (1/2 po prawej stronie log)
        { "%",   {300.f, 642.f}, ButtonType::Small },
        { "‰",   {368.f, 642.f}, ButtonType::Small },
        { "!",   {436.f, 642.f}, ButtonType::Small },
        { "^",   {504.f, 642.f}, ButtonType::Small },
        { "√",   {572.f, 642.f}, ButtonType::Small },
        { "log", {640.f, 642.f}, ButtonType::Small },
        { "1/2", {708.f, 642.f}, ButtonType::Small },

        // Cyfry szesnastkowe A-F
        { "A", {300.f, 695.f}, ButtonType::Small },
        { "B", {368.f, 695.f}, ButtonType::Small },
        { "C", {436.f, 695.f}, ButtonType::Small, std::nullopt, "HEX_C" },
        { "D", {504.f, 695.f}, ButtonType::Small },
        { "E", {572.f, 695.f}, ButtonType::Small },
        { "F", {640.f, 695.f}, ButtonType::Small }
    };

    for (auto& d : defs)
    {
        if (!d.action.empty())
        {
            continue;
        }
        else if (d.label == "←")
            d.action = "BACKSPACE";
        else if (d.label == "×")
            d.action = "MULTIPLY";
        else if (d.label == "÷")
            d.action = "DIVIDE";
        else if (d.label == "√")
            d.action = "ROOT";
        else if (d.label == "1/2")
            d.action = "FRACTION";
        else if (d.label == "BIN")
            d.action = "SYSTEM_BIN";
        else if (d.label == "OCT")
            d.action = "SYSTEM_OCT";
        else if (d.label == "DEC")
            d.action = "SYSTEM_DEC";
        else if (d.label == "HEX")
            d.action = "SYSTEM_HEX";
        else if (d.label.size() == 1 &&
                d.label[0] >= 'A' && d.label[0] <= 'F')
            d.action = "HEX_" + d.label;
        else
            d.action = d.label;
    }

    std::vector<Button> buttons;
    buttons.reserve(defs.size());
    for (const auto& d : defs) {
        buttons.emplace_back(font, mathFont, d.label, d.pos, d.type, d.shortcut);
    }

    const float tableX = 1010.f;
    const float tableY = 175.f;
    const float cellWidth = 60.f;
    const float cellHeight = 45.f;

    const std::vector<std::string> col1 = {"0", "1", "2", "3"};
    const std::vector<std::string> col2 = {"1", "2", "3", "4"};
    const std::vector<std::string> col3 = {"1/3", "√", "r", "0,( )"};

    std::vector<Button> tableButtons;
    for (std::size_t row = 0; row < 4; ++row) {
        tableButtons.emplace_back(font, mathFont, col1[row], sf::Vector2f(tableX, tableY + row * cellHeight), ButtonType::Small);
        tableButtons.emplace_back(font, mathFont, col2[row], sf::Vector2f(tableX + cellWidth, tableY + row * cellHeight), ButtonType::Small);
        tableButtons.emplace_back(font, mathFont, col3[row], sf::Vector2f(tableX + cellWidth * 2.f, tableY + row * cellHeight), ButtonType::Small);
    }
    
    return {std::move(defs), std::move(buttons), std::move(tableButtons)};
}
