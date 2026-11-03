#include "ButtonFocusHandler.h"
#include "NavigationRules.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <vector>

void handleButtonFocusKey(sf::Keyboard::Key direction,
                          sf::Vector2f mousePos,
                          ButtonFocusContext context)
{
    const auto& defs = context.defs;
    auto& buttons = context.buttons;
    auto& tableButtons = context.tableButtons;
    auto& navigation = context.navigation;
    auto& tableFocus = context.tableFocus;
    auto& panelFocused = context.interaction.panelFocused;
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
        return;
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
        return;
    }

    const std::string& label = defs[fromIndex].label;
    std::optional<std::string> targetLabel;
    bool toPanel = false;
    bool toTable = false;
    bool noMove = false;

    targetLabel = NavigationRules::explicitTarget(label, defs[fromIndex].action, direction);
    if (direction == sf::Keyboard::Key::Right ||
        direction == sf::Keyboard::Key::Left) {
        if (label == "NWD" || label == "NWW") toPanel = true;
        if (direction == sf::Keyboard::Key::Left && label == "ESC") noMove = true;
    } else if (direction == sf::Keyboard::Key::Down) {
        if (label == "WYNIKI v") toTable = true;
        else if (!targetLabel && defs[fromIndex].pos.y <= 125.f && label != "ESC") toPanel = true;
    } else if (direction == sf::Keyboard::Key::Up) {
        if (label == "ESC") noMove = true;
        else if (!targetLabel && defs[fromIndex].pos.y <= 125.f) targetLabel = "ESC";
        else if (!targetLabel && defs[fromIndex].pos.y >= 425.f &&
                 defs[fromIndex].pos.y <= 435.f) toPanel = true;
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
        if (digitColumn) return;
    }
    if (noMove) return;
    if (toPanel) { focusPanel(); return; }
    if (toTable) { focusTable(0); return; }

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
        const auto explicitName = NavigationRules::explicitTarget(name, d.action, direction);
        const auto fallbackName = (!explicitName && direction == sf::Keyboard::Key::Up &&
                                   d.pos.y <= 125.f && name != "ESC")
            ? std::optional<std::string>("ESC") : std::nullopt;
        if (explicitName || fallbackName) {
            auto id = findAnyButton(explicitName ? *explicitName : *fallbackName);
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
