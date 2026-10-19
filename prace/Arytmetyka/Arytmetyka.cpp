#include "Expression.h"

#include "../GUI/Button.h"
#include "ArithmeticButtons.h"
#include "../GUI/ButtonNavigation.h"
#include "../GUI/ExpressionPanel.h"

#include <SFML/Graphics.hpp>

#include <cctype>
#include <optional>
#include <string>
#include <vector>


void runArytmetyka(sf::RenderWindow& window, sf::Font& font, sf::Font& mathFont)
{
    Expression expression;
    ExpressionPanel expressionPanel(mathFont);

    std::string resultText = "";
    bool editing = false;
    bool selectingWithMouse = false;
    bool panelFocused = false;
    std::optional<std::size_t> tableFocus;
    std::size_t lastVisibleCursor = 0;
    std::string lastVisibleExpression;

    enum class NumberSystem {
        BIN,
        OCT,
        DEC,
        HEX
    };

    NumberSystem currentSystem = NumberSystem::DEC;

    auto isDigitAllowed = [&](const std::string& label) -> bool
    {
        if (label.size() != 1 || label[0] < '0' || label[0] > '9')
            return true;

        int digit = label[0] - '0';

        if (currentSystem == NumberSystem::BIN)
            return digit <= 1;

        if (currentSystem == NumberSystem::OCT)
            return digit <= 7;

        return true;
    };

    sf::Text title(font, "Arytmetyka", 40);
    title.setFillColor(sf::Color::White);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin({titleBounds.position.x + titleBounds.size.x / 2.f, titleBounds.position.y + titleBounds.size.y / 2.f});
    title.setPosition({600.f, 50.f});

    auto controls = createArithmeticButtons(font, mathFont);
    auto& defs = controls.defs;
    auto& buttons = controls.buttons;
    auto& tableButtons = controls.tableButtons;

    ButtonNavigation navigation(buttons);

    while (window.isOpen())
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        while (const std::optional event = window.pollEvent())
        {
            expressionPanel.handleEvent(*event, mousePos, expression.get());
            if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouse->button == sf::Mouse::Button::Left) {
                    if (expressionPanel.contains(mousePos)) {
                        editing = true;
                        selectingWithMouse = true;
                        expression.setCursor(expressionPanel.cursorAt(mousePos, expression.get()));
                        expressionPanel.setWrapSideFromClick(mousePos, expression.get(), expression.cursor());
                        navigation.clearFocus();
                        panelFocused = false;
                    } else {
                        panelFocused = false;
                        selectingWithMouse = false;
                        editing = false;
                    }
                }
            }
            if (const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouse->button == sf::Mouse::Button::Left) selectingWithMouse = false;
            }
            if (event->is<sf::Event::MouseMoved>() && selectingWithMouse)
                {
                    expression.setCursor(expressionPanel.cursorAt(mousePos, expression.get()), true);
                    expressionPanel.setWrapSideFromClick(mousePos, expression.get(), expression.cursor());
                }


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
                    navigation.clearFocus();
                    tableFocus.reset();
                    panelFocused = false;
                }
            }

            if (event->is<sf::Event::KeyPressed>())
            {
                const auto* keyEvent = event->getIf<sf::Event::KeyPressed>();

                const bool ctrl = keyEvent->control;
                const bool shift = keyEvent->shift;
                const auto key = keyEvent->code;
                if (editing) {
                    if (ctrl && key == sf::Keyboard::Key::A) expression.selectAll();
                    else if (ctrl && key == sf::Keyboard::Key::C) expression.copy();
                    else if (ctrl && key == sf::Keyboard::Key::X) expression.cut();
                    else if (ctrl && key == sf::Keyboard::Key::V) expression.paste();
                    else if (ctrl && key == sf::Keyboard::Key::Z) expression.undo();
                    else if (ctrl && key == sf::Keyboard::Key::Y) expression.redo();
                    else if (key == sf::Keyboard::Key::Left || key == sf::Keyboard::Key::Right) {
                        const bool right = key == sf::Keyboard::Key::Right;
                        const bool boundary = expressionPanel.isWrapBoundary(expression.get(), expression.cursor());
                        // Pierwsza strzalka na granicy zawijania zmienia tylko wizualna strone kursora.
                        if (!ctrl && boundary && ((right && expressionPanel.wrapSideAtEnd()) || (!right && !expressionPanel.wrapSideAtEnd()))) {
                            expressionPanel.setWrapSide(!right);
                        } else {
                            expression.moveHorizontal(right ? 1 : -1, ctrl, shift);
                            expressionPanel.setWrapSide(right);
                        }
                        expressionPanel.ensureCursorVisible(expression.get(), expression.cursor());
                    }
                    else if (key == sf::Keyboard::Key::Up) {
                        if (ctrl) expression.setCursor(expressionPanel.lineBoundary(expression.get(), expression.cursor(), false), shift);
                        else expression.setCursor(expressionPanel.verticalCursor(expression.get(), expression.cursor(), -1), shift);
                        expressionPanel.setWrapSide(!ctrl);
                    }
                    else if (key == sf::Keyboard::Key::Down) {
                        if (ctrl) expression.setCursor(expressionPanel.lineBoundary(expression.get(), expression.cursor(), true), shift);
                        else expression.setCursor(expressionPanel.verticalCursor(expression.get(), expression.cursor(), 1), shift);
                        expressionPanel.setWrapSide(ctrl);
                    }
                    else if (key == sf::Keyboard::Key::Backspace) expression.removeLast();
                    else if (key == sf::Keyboard::Key::Delete) expression.deleteForward();
                    else if (key == sf::Keyboard::Key::Enter) expression.add("\n");
                    else if (key == sf::Keyboard::Key::Escape) { editing = false; panelFocused = true; }
                    continue;
                }
                if (key == sf::Keyboard::Key::Enter && panelFocused) {
                    editing = true;
                    panelFocused = false;
                    const std::string currentExpression = expression.get();
                    expression.setCursor(sf::String::fromUtf8(currentExpression.begin(), currentExpression.end()).getSize());
                    expressionPanel.setWrapSide(false);
                    expressionPanel.ensureCursorVisible(expression.get(), expression.cursor());
                    continue;
                }
                if (key == sf::Keyboard::Key::Enter && !navigation.focus() && !tableFocus) {
                    editing = true;
                    const std::string currentExpression = expression.get();
                    expression.setCursor(sf::String::fromUtf8(currentExpression.begin(), currentExpression.end()).getSize());
                    expressionPanel.setWrapSide(false);
                    expressionPanel.ensureCursorVisible(expression.get(), expression.cursor());
                    continue;
                }
                if (keyEvent->code == sf::Keyboard::Key::Right ||
                    keyEvent->code == sf::Keyboard::Key::Left ||
                    keyEvent->code == sf::Keyboard::Key::Down ||
                    keyEvent->code == sf::Keyboard::Key::Up)
                {
                    std::size_t fromIndex;

                    if (navigation.focus())
                    {
                        fromIndex = *navigation.focus();
                    }
                    else
                    {
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
                            fromIndex = navigation.nearestToMouse(mousePos);
                        }
                    }

                    // Jawne przejscia miedzy przyciskami; panel nie jest
                    // automatycznym celem dla kazdego przycisku ponizej.
                    const auto direction = keyEvent->code;
                    auto findButton = [&](const std::string& name) -> std::optional<std::size_t> {
                        for (std::size_t i = 0; i < defs.size(); ++i)
                            if (defs[i].label == name && buttons[i].isEnabled()) return i;
                        return std::nullopt;
                    };
                    auto focusButton = [&](std::optional<std::size_t> target) {
                        if (!target) return;
                        panelFocused = false;
                        tableFocus.reset();
                        navigation.setFocus(*target);
                    };
                    auto focusPanel = [&]() {
                        navigation.clearFocus();
                        tableFocus.reset();
                        panelFocused = true;
                    };
                    auto focusTable = [&](std::size_t index) {
                        if (index >= tableButtons.size() || !tableButtons[index].isEnabled()) return;
                        navigation.clearFocus();
                        panelFocused = false;
                        tableFocus = index;
                    };

                    if (tableFocus) {
                        const std::size_t old = *tableFocus;
                        const std::size_t row = old / 3;
                        const std::size_t col = old % 3;
                        if (direction == sf::Keyboard::Key::Right)
                            focusTable(row * 3 + (col + 1) % 3);
                        else if (direction == sf::Keyboard::Key::Left)
                            focusTable(row * 3 + (col + 2) % 3);
                        else if (direction == sf::Keyboard::Key::Down)
                            focusTable(((row + 1) % 4) * 3 + col);
                        else if (direction == sf::Keyboard::Key::Up) {
                            if (row == 0) focusButton(findButton("WYNIKI v"));
                            else focusTable((row - 1) * 3 + col);
                        }
                        continue;
                    }
                    if (panelFocused) {
                        if (direction == sf::Keyboard::Key::Right) focusButton(findButton("NWD"));
                        else if (direction == sf::Keyboard::Key::Left) focusButton(findButton("NWD"));
                        else if (direction == sf::Keyboard::Key::Down) {
                            // Kolumna prawej krawedzi pola: 9 -> 6 -> 3 -> =.
                            // Nie przechodz przez wylaczone cyfry do HEX.
                            for (const char* candidate : {"9", "6", "3", "="}) {
                                auto id = findButton(candidate);
                                if (id) { focusButton(id); break; }
                            }
                        }
                        else if (direction == sf::Keyboard::Key::Up) focusButton(findButton("W"));
                        continue;
                    }

                    const std::string& label = defs[fromIndex].label;
                    std::optional<std::string> targetLabel;
                    bool toPanel = false;
                    bool toTable = false;
                    bool noMove = false;

                    if (direction == sf::Keyboard::Key::Right) {
                        if (label == "WYNIKI v") targetLabel = "BIN";
                        else if (label == "NWD" || label == "NWW") toPanel = true;
                        else if (label == "1/x") targetLabel = "(";
                        else if (label == "-x") targetLabel = "[";
                        else if (label == "←") targetLabel = "{";
                        else if (label == "C" && defs[fromIndex].action == "CLEAR") targetLabel = "|";
                        else if (label == "1/2") targetLabel = "%";
                        else if (label == "F") targetLabel = "A";
                        else if (label == "ESC") targetLabel = "BIN";
                    } else if (direction == sf::Keyboard::Key::Left) {
                        if (label == "ESC") noMove = true;
                        else if (label == "BIN") targetLabel = "WYNIKI v";
                        else if (label == "NWD" || label == "NWW") toPanel = true;
                        else if (label == "1/x") targetLabel = "÷";
                        else if (label == "-x") targetLabel = "×";
                        else if (label == "←") targetLabel = "-";
                        else if (label == "C" && defs[fromIndex].action == "CLEAR") targetLabel = "+";
                        else if (label == "(") targetLabel = "1/x";
                        else if (label == "[") targetLabel = "-x";
                        else if (label == "{") targetLabel = "←";
                        else if (label == "|" && defs[fromIndex].action == "ABS_OPEN") targetLabel = "C";
                        else if (label == "%") targetLabel = "1/2";
                        else if (label == "A") targetLabel = "F";
                    } else if (direction == sf::Keyboard::Key::Down) {
                        if (label == "BIN") targetLabel = "NWD";
                        else if (label == "NWD") targetLabel = "NWW";
                        else if (label == "NWW") targetLabel = "(";
                        else if (label == "1/2" || (label == "C" && defs[fromIndex].action == "CLEAR")) targetLabel = "1/x";
                        else if (label == "WYNIKI v") toTable = true;
                        else if (label.size() == 1 && label[0] >= 'A' && label[0] <= 'F' && defs[fromIndex].action.rfind("HEX_", 0) == 0) {
                            const char* targets[] = {"(", ")", "7", "8", "9", "÷"};
                            targetLabel = targets[label[0] - 'A'];
                        }
                        else if (defs[fromIndex].pos.y <= 125.f && label != "ESC") toPanel = true;
                    } else if (direction == sf::Keyboard::Key::Up) {
                        if (label == "ESC") noMove = true;
                        else if (label == "1/2") targetLabel = "C";
                        else if (defs[fromIndex].pos.y <= 125.f) targetLabel = "ESC";
                        else if (defs[fromIndex].pos.y >= 425.f && defs[fromIndex].pos.y <= 435.f) toPanel = true;
                    }

                    // Cyfry i dolny rzad maja pionowe kolumny. Jesli nad
                    // nimi nie ma juz aktywnego przycisku w tej kolumnie,
                    // przechodzimy na pole, nigdy na rzad HEX.
                    if (direction == sf::Keyboard::Key::Up) {
                        const std::vector<std::vector<std::string>> columns = {
                            {"0", "1", "4", "7"},
                            {",", "2", "5", "8"},
                            {"=", "3", "6", "9"}
                        };
                        bool digitColumn = false;
                        for (const auto& column : columns) {
                            for (std::size_t row = 0; row < column.size(); ++row) {
                                if (label != column[row]) continue;
                                digitColumn = true;
                                std::optional<std::size_t> above;
                                for (std::size_t nextRow = row + 1; nextRow < column.size(); ++nextRow) {
                                    above = findButton(column[nextRow]);
                                    if (above) break;
                                }
                                if (above) { focusButton(above); }
                                else focusPanel();
                                break;
                            }
                            if (digitColumn) break;
                        }
                        if (digitColumn) continue;
                    }
                    if (noMove) continue;
                    if (toPanel) { focusPanel(); continue; }
                    if (toTable) { focusTable(0); continue; }

                    // Traktuj nieaktywne przyciski jak wirtualne przystanki:
                    // przechodzimy przez nie w tym samym kierunku, zachowujac
                    // ich indywidualne polaczenia, zamiast wybierac najblizszy.
                    auto findAnyButton = [&](const std::string& name) -> std::optional<std::size_t> {
                        for (std::size_t i = 0; i < defs.size(); ++i)
                            if (defs[i].label == name) return i;
                        return std::nullopt;
                    };
                    auto nextVirtual = [&](std::size_t index) -> std::optional<std::size_t> {
                        const auto& d = defs[index];
                        const auto& name = d.label;
                        std::optional<std::string> explicitName;
                        if (direction == sf::Keyboard::Key::Right) {
                            if (name == "WYNIKI v") explicitName = "BIN";
                            else if (name == "1/x") explicitName = "(";
                            else if (name == "-x") explicitName = "[";
                            else if (name == "←") explicitName = "{";
                            else if (name == "C" && d.action == "CLEAR") explicitName = "|";
                            else if (name == "1/2") explicitName = "%";
                            else if (name == "F") explicitName = "A";
                            else if (name == "ESC") explicitName = "BIN";
                        } else if (direction == sf::Keyboard::Key::Left) {
                            if (name == "BIN") explicitName = "WYNIKI v";
                            else if (name == "1/x") explicitName = "÷";
                            else if (name == "-x") explicitName = "×";
                            else if (name == "←") explicitName = "-";
                            else if (name == "C" && d.action == "CLEAR") explicitName = "+";
                            else if (name == "(") explicitName = "1/x";
                            else if (name == "[") explicitName = "-x";
                            else if (name == "{") explicitName = "←";
                            else if (name == "|" && d.action == "ABS_OPEN") explicitName = "C";
                            else if (name == "%") explicitName = "1/2";
                            else if (name == "A") explicitName = "F";
                        } else if (direction == sf::Keyboard::Key::Down) {
                            if (name == "BIN") explicitName = "NWD";
                            else if (name == "NWD") explicitName = "NWW";
                            else if (name == "NWW") explicitName = "(";
                            else if (name == "1/2" || (name == "C" && d.action == "CLEAR")) explicitName = "1/x";
                            else if (name.size() == 1 && name[0] >= 'A' && name[0] <= 'F' && d.action.rfind("HEX_", 0) == 0) {
                                const char* targets[] = {"(", ")", "7", "8", "9", "÷"};
                                explicitName = targets[name[0] - 'A'];
                            }
                        } else if (direction == sf::Keyboard::Key::Up) {
                            if (name == "1/2") explicitName = "C";
                            else if (d.pos.y <= 125.f && name != "ESC") explicitName = "ESC";
                        }
                        if (explicitName) {
                            auto id = findAnyButton(*explicitName);
                            if (id) return id;
                        }
                        // Geometria z uwzglednieniem wylaczonych przyciskow.
                        // Nastepny przycisk szukamy wsrod WSZYSTKICH, nie tylko aktywnych.
                        const auto p0 = d.pos;
                        float bestScore = 1e30f;
                        std::optional<std::size_t> best;
                        for (std::size_t j = 0; j < defs.size(); ++j) {
                            if (j == index) continue;
                            const auto p1 = defs[j].pos;
                            const float dx = p1.x - p0.x, dy = p1.y - p0.y;
                            const bool horizontal = direction == sf::Keyboard::Key::Right || direction == sf::Keyboard::Key::Left;
                            const float primary = horizontal ? dx : dy;
                            const float secondary = horizontal ? dy : dx;
                            if ((direction == sf::Keyboard::Key::Right || direction == sf::Keyboard::Key::Down) ? primary <= 0.f : primary >= 0.f) continue;
                            const float score = std::abs(primary) + std::abs(secondary) * 10.f;
                            if (score < bestScore) { bestScore = score; best = j; }
                        }
                        return best;
                    };
                    std::optional<std::size_t> next = targetLabel ? findAnyButton(*targetLabel) : nextVirtual(fromIndex);
                    std::vector<bool> visited(buttons.size(), false);
                    while (next && !buttons[*next].isEnabled() && !visited[*next]) {
                        visited[*next] = true;
                        next = nextVirtual(*next);
                    }
                    if (next && buttons[*next].isEnabled()) focusButton(next);
                }
            }

            for (auto& btn : buttons) btn.handleEvent(*event);
            for (auto& btn : tableButtons) btn.handleEvent(*event);

            if (event->is<sf::Event::TextEntered>()) {
                char32_t unicode = event->getIf<sf::Event::TextEntered>()->unicode;
                if (editing && (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl))) continue;
                if ((unicode >= '0' && unicode <= '9') ||
                    (currentSystem == NumberSystem::HEX &&
                    ((unicode >= 'A' && unicode <= 'F') ||
                    (unicode >= 'a' && unicode <= 'f'))) ||
                    unicode == '+' || unicode == '-' ||
                    unicode == '*' || unicode == '/' ||
                    unicode == '(' || unicode == ')' ||
                    unicode == '.' || unicode == ',') {
                    char c = static_cast<char>(unicode);
                    if (c == '.') c = ',';

                    if (c >= 'a' && c <= 'f')
                    {
                        c = c - 'a' + 'A';
                    }

                    std::string znak(1, c);

                    if (isDigitAllowed(znak))
                    {
                        expression.add(znak);
                    }
                }
            }
        }

        for (std::size_t i = 0; i < buttons.size(); ++i) {
            bool enabled = isDigitAllowed(defs[i].label);

            if (defs[i].action.size() == 5 &&
                defs[i].action.substr(0, 4) == "HEX_")
            {
                enabled = (currentSystem == NumberSystem::HEX);
            }

            buttons[i].setEnabled(enabled);
            buttons[i].update(mousePos);
            if (buttons[i].isActivated()) {
                std::string lbl = defs[i].label;
                std::string action = defs[i].action;

                
                if (action == "ESC") return;
                else if (action == "SYSTEM_BIN") currentSystem = NumberSystem::BIN;
                else if (action == "SYSTEM_OCT") currentSystem = NumberSystem::OCT;
                else if (action == "SYSTEM_DEC") currentSystem = NumberSystem::DEC;
                else if (action == "SYSTEM_HEX") currentSystem = NumberSystem::HEX;
                else if (action == "CLEAR")
                {
                    expression.clear();
                    resultText.clear();

                    expressionPanel.resetScroll();
                }
                else if (action == "BACKSPACE") expression.removeLast();
                else if (action == "ABS_OPEN")
                    expression.add("<");

                else if (action == "ABS_CLOSE")
                    expression.add(">");
                else if (
                    action == "HEX_A" ||
                    action == "HEX_B" ||
                    action == "HEX_C" ||
                    action == "HEX_D" ||
                    action == "HEX_E" ||
                    action == "HEX_F"
                )
                {
                    expression.add(action.substr(4));
                }
                else if (action == "=") {
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
                action == "WYNIKI v" ||
                action == "SYSTEM_BIN" ||
                action == "SYSTEM_OCT" ||
                action == "SYSTEM_DEC" ||
                action == "SYSTEM_HEX" ||
                action == "W" ||
                action == "K" ||
                action == "NWD" ||
                action == "NWW" ||
                action == "1/x" ||
                action == "-x"
            ){
                    // Te przyciski nie wpisują tekstu do wyrażenia.
                    // Ich właściwe działanie dodamy w kolejnych etapach.
                }
                else {
                    expression.add(lbl);
                }

            }
        }

        for (std::size_t i = 0; i < tableButtons.size(); ++i) {
            tableButtons[i].update(mousePos);
            tableButtons[i].setSelected(tableFocus && *tableFocus == i);
        }

        if (lastVisibleCursor != expression.cursor() ||
            lastVisibleExpression != expression.get())
            expressionPanel.ensureCursorVisible(expression.get(), expression.cursor());
        lastVisibleCursor = expression.cursor();
        lastVisibleExpression = expression.get();

        window.clear(sf::Color(30, 30, 30));
        window.draw(title);
        expressionPanel.setKeyboardFocus(panelFocused && !editing);
        expressionPanel.draw(window, expression.get());
        expressionPanel.drawEditor(window, expression.get(), expression.cursor(),
            expression.selectionStart(), expression.selectionEnd(), editing);

        for (auto& btn : buttons) btn.draw(window);
        for (std::size_t i = 0; i < tableButtons.size(); ++i)
            if (!tableFocus || *tableFocus != i) tableButtons[i].draw(window);
        if (tableFocus && *tableFocus < tableButtons.size())
            tableButtons[*tableFocus].draw(window);

        window.display();
    }
}