#include "Expression.h"
#include "../GUI/Button.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <optional>
#include <limits>
#include <cmath>

void runArytmetyka(sf::RenderWindow& window, sf::Font& font, sf::Font& mathFont)
{
    Expression expression;
    std::string resultText = "";

    enum class NumberSystem {
        BIN,
        OCT,
        DEC,
        HEX
    };

    NumberSystem currentSystem = NumberSystem::DEC;

    // =========================
    // NAGŁÓWEK I POLE TEKSTOWE
    // =========================
    sf::Text title(font, "Arytmetyka", 40);
    title.setFillColor(sf::Color::White);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin({titleBounds.position.x + titleBounds.size.x / 2.f, titleBounds.position.y + titleBounds.size.y / 2.f});
    title.setPosition({600.f, 50.f});

    sf::RectangleShape expressionBox(sf::Vector2f(780.f, 210.f));
    expressionBox.setFillColor(sf::Color(45, 45, 45));
    expressionBox.setOutlineColor(sf::Color(100, 100, 100));
    expressionBox.setOutlineThickness(2.f);
    expressionBox.setPosition({220.f, 180.f});

    sf::Text expressionText(mathFont, "", 28);
    expressionText.setFillColor(sf::Color::White);
    expressionText.setPosition({240.f, 200.f});

    sf::Text resText(mathFont, "", 24);
    resText.setFillColor(sf::Color(0, 255, 128));
    resText.setPosition({240.f, 340.f});

    // =========================
    // DEFINICJA PRZYCISKÓW GŁÓWNYCH
    // =========================
    struct ButtonDef {
        std::string label;
        sf::Vector2f pos;
        ButtonType type;
        std::optional<sf::Keyboard::Key> shortcut = std::nullopt;
    };

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
        { "C",   {928.f, 589.f}, ButtonType::Medium },

        // Nawiasy
        { "(", {300.f, 430.f}, ButtonType::Small }, { ")", {368.f, 430.f}, ButtonType::Small },
        { "[", {300.f, 483.f}, ButtonType::Small }, { "]", {368.f, 483.f}, ButtonType::Small },
        { "{", {300.f, 536.f}, ButtonType::Small }, { "}", {368.f, 536.f}, ButtonType::Small },
        { "|", {300.f, 589.f}, ButtonType::Small }, { "|", {368.f, 589.f}, ButtonType::Small },

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
        { "1/2", {708.f, 642.f}, ButtonType::Small }
    };

    std::vector<Button> buttons;
    buttons.reserve(defs.size());
    for (const auto& d : defs) {
        buttons.emplace_back(font, mathFont, d.label, d.pos, d.type, d.shortcut);
    }

    // =========================
    // TABELA WYNIKI Z PRZYCISKÓW ButtonType::Small
    // =========================
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

    // Nawigacja klawiaturą. Na początku nic nie jest zaznaczone.
    std::optional<std::size_t> focusedIndex;

    auto clearFocus = [&]()
    {
        if (focusedIndex)
        {
            buttons[*focusedIndex].setSelected(false);
            focusedIndex.reset();
        }
    };

    auto setFocus = [&](std::size_t newIndex)
    {
        if (focusedIndex && *focusedIndex != newIndex)
        {
            buttons[*focusedIndex].setSelected(false);
        }

        focusedIndex = newIndex;
        buttons[newIndex].setSelected(true);
    };

    auto buttonCenter = [&](std::size_t index)
    {
        const sf::Vector2f pos = buttons[index].getPosition();
        const sf::Vector2f size = buttons[index].getSize();
        return sf::Vector2f(pos.x + size.x / 2.f, pos.y + size.y / 2.f);
    };

    auto nearestInDirection = [&](std::size_t fromIndex, sf::Keyboard::Key direction) -> std::optional<std::size_t>
    {
        const sf::Vector2f from = buttonCenter(fromIndex);
        std::optional<std::size_t> best;
        float bestDistance = std::numeric_limits<float>::max();

        for (std::size_t i = 0; i < buttons.size(); ++i)
        {
            if (i == fromIndex)
                continue;

            const sf::Vector2f to = buttonCenter(i);
            const float dx = to.x - from.x;
            const float dy = to.y - from.y;

            bool valid = false;

            if (direction == sf::Keyboard::Key::Right) valid = dx > 0.f;
            if (direction == sf::Keyboard::Key::Left)  valid = dx < 0.f;
            if (direction == sf::Keyboard::Key::Down)  valid = dy > 0.f;
            if (direction == sf::Keyboard::Key::Up)    valid = dy < 0.f;

            if (!valid)
                continue;

            // Najpierw szukamy przycisku możliwie najbardziej w jednej linii
            // z aktualnym przyciskiem. Dopiero potem liczy się odległość.
            // Dzięki temu np. HEX -> W, a ÷ -> 1/x, zamiast skoku do 7.
            float primaryDistance = 0.f;
            float secondaryDistance = 0.f;

            if (direction == sf::Keyboard::Key::Right ||
                direction == sf::Keyboard::Key::Left)
            {
                primaryDistance = std::abs(dx);
                secondaryDistance = std::abs(dy);
            }
            else
            {
                primaryDistance = std::abs(dy);
                secondaryDistance = std::abs(dx);
            }

            // Duża kara za odchylenie w poprzek kierunku.
            const float distance = primaryDistance + secondaryDistance * 10.f;

            if (distance < bestDistance)
            {
                bestDistance = distance;
                best = i;
            }
        }

        return best;
    };

    auto wrapInDirection = [&](std::size_t fromIndex, sf::Keyboard::Key direction) -> std::optional<std::size_t>
    {
        const sf::Vector2f from = buttonCenter(fromIndex);
        std::optional<std::size_t> best;
        float bestScore = std::numeric_limits<float>::max();

        for (std::size_t i = 0; i < buttons.size(); ++i)
        {
            if (i == fromIndex)
                continue;

            const sf::Vector2f to = buttonCenter(i);
            const float perpendicular =
                (direction == sf::Keyboard::Key::Left ||
                 direction == sf::Keyboard::Key::Right)
                    ? std::abs(to.y - from.y)
                    : std::abs(to.x - from.x);

            float edgeDistance = 0.f;

            if (direction == sf::Keyboard::Key::Down)
                edgeDistance = to.y;
            else if (direction == sf::Keyboard::Key::Up)
                edgeDistance = -to.y;
            else if (direction == sf::Keyboard::Key::Right)
                edgeDistance = to.x;
            else if (direction == sf::Keyboard::Key::Left)
                edgeDistance = -to.x;

            const float score = perpendicular * 1000.f + edgeDistance;

            if (score < bestScore)
            {
                bestScore = score;
                best = i;
            }
        }

        return best;
    };

    auto nearestToMouse = [&](sf::Vector2f mousePos) -> std::size_t
    {
        std::size_t best = 0;
        float bestDistance = std::numeric_limits<float>::max();

        for (std::size_t i = 0; i < buttons.size(); ++i)
        {
            const sf::Vector2f center = buttonCenter(i);
            const float dx = center.x - mousePos.x;
            const float dy = center.y - mousePos.y;
            const float distance = dx * dx + dy * dy;

            if (distance < bestDistance)
            {
                bestDistance = distance;
                best = i;
            }
        }

        return best;
    };

    // =========================
    // PĘTLA PROGRAMU
    // =========================
    while (window.isOpen())
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return;
            }

            if (event->is<sf::Event::MouseMoved>() ||
                event->is<sf::Event::MouseButtonPressed>())
            {
                const auto* mouseEvent =
                    event->getIf<sf::Event::MouseButtonPressed>();

                if (!event->is<sf::Event::MouseButtonPressed>() ||
                    mouseEvent->button == sf::Mouse::Button::Left)
                {
                    clearFocus();
                }
            }

            if (event->is<sf::Event::KeyPressed>())
            {
                const auto* keyEvent = event->getIf<sf::Event::KeyPressed>();

                if (keyEvent->code == sf::Keyboard::Key::Right ||
                    keyEvent->code == sf::Keyboard::Key::Left ||
                    keyEvent->code == sf::Keyboard::Key::Down ||
                    keyEvent->code == sf::Keyboard::Key::Up)
                {
                    std::size_t fromIndex;

                    if (focusedIndex)
                    {
                        fromIndex = *focusedIndex;
                    }
                    else
                    {
                        // Jeżeli nic nie było zaznaczone, zaczynamy dokładnie
                        // od przycisku, na którym znajduje się kursor.
                        // Nie od przycisku, którego środek jest najbliżej kursora.
                        std::optional<std::size_t> hoveredIndex;

                        for (std::size_t i = 0; i < buttons.size(); ++i)
                        {
                            if (buttons[i].isHovered())
                            {
                                hoveredIndex = i;
                                break;
                            }
                        }

                        if (hoveredIndex)
                        {
                            fromIndex = *hoveredIndex;
                        }
                        else
                        {
                            fromIndex = nearestToMouse(mousePos);
                        }
                    }

                    auto next = nearestInDirection(fromIndex, keyEvent->code);

                    if (next)
                    {
                        setFocus(*next);
                    }
                    else
                    {
                        // Doszliśmy do krawędzi. Przechodzimy na przeciwną
                        // stronę, zachowując możliwie tę samą linię.
                        auto wrapped = wrapInDirection(fromIndex, keyEvent->code);
                        if (wrapped)
                        {
                            setFocus(*wrapped);
                        }
                    }
                }
            }

            for (auto& btn : buttons) btn.handleEvent(*event);
            for (auto& btn : tableButtons) btn.handleEvent(*event);

            if (event->is<sf::Event::TextEntered>()) {
                char32_t unicode = event->getIf<sf::Event::TextEntered>()->unicode;
                if ((unicode >= '0' && unicode <= '9') || unicode == '+' || unicode == '-' ||
                    unicode == '*' || unicode == '/' || unicode == '(' || unicode == ')' || unicode == '.' || unicode == ',') {
                    char c = static_cast<char>(unicode);
                    if (c == '.') c = ',';
                    expression.add(std::string(1, c));
                }
            }
        }

        // Aktualizacja stanu i reakcja na kliknięcie/Enter
        for (std::size_t i = 0; i < buttons.size(); ++i) {
            buttons[i].update(mousePos);
            if (buttons[i].isActivated()) {
                std::string lbl = defs[i].label;

                
                if (lbl == "ESC") return;
                else if (lbl == "BIN") currentSystem = NumberSystem::BIN;
                else if (lbl == "OCT") currentSystem = NumberSystem::OCT;
                else if (lbl == "DEC") currentSystem = NumberSystem::DEC;
                else if (lbl == "HEX") currentSystem = NumberSystem::HEX;
                else if (lbl == "C") { expression.clear(); resultText.clear(); }
                else if (lbl == "←") expression.removeLast();
                else if (lbl == "=") {
                    std::string currentStr = expression.get();
                    if (!currentStr.empty()) {
                        try {
                            resultText = "= " + std::to_string(std::stod(currentStr));
                        } catch (...) {
                            resultText = "Blad wyrazenia!";
                        }
                    }
                } 
                else if (
                    lbl == "WYNIKI v" ||
                    lbl == "BIN" ||
                    lbl == "OCT" ||
                    lbl == "DEC" ||
                    lbl == "HEX" ||
                    lbl == "W" ||
                    lbl == "K" ||
                    lbl == "NWD" ||
                    lbl == "NWW" ||
                    lbl == "1/x" ||
                    lbl == "-x"
                ) {
                    // Te przyciski nie wpisują tekstu do wyrażenia.
                    // Ich właściwe działanie dodamy w kolejnych etapach.
                }
                else {
                    expression.add(lbl);
                }

            }
        }

        for (auto& btn : tableButtons) {
            btn.update(mousePos);
        }

        expressionText.setString(sf::String::fromUtf8(expression.get().begin(), expression.get().end()));
        resText.setString(sf::String::fromUtf8(resultText.begin(), resultText.end()));

        // Rysowanie
        window.clear(sf::Color(30, 30, 30));

        window.draw(title);
        window.draw(expressionBox);
        window.draw(expressionText);
        window.draw(resText);

        for (auto& btn : buttons) btn.draw(window);
        for (auto& btn : tableButtons) btn.draw(window);

        window.display();
    }
}
