#include "ExpressionTextHandler.h"
#include "Expression.h"
#include "../GUI/PowerVisual.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <string>

void handleExpressionText(const sf::Event::TextEntered& entered,
                          ExpressionTextContext context)
{
            if (!context.interaction.resultFocused) {
                char32_t unicode = entered.unicode;
                if (context.interaction.editing && (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl))) return;
                if ((unicode >= '0' && unicode <= '9') ||
                    (context.currentSystem == NumberSystem::HEX &&
                    ((unicode >= 'A' && unicode <= 'F') ||
                    (unicode >= 'a' && unicode <= 'f'))) ||
                    unicode == '+' || unicode == '-' ||
                    unicode == '*' || unicode == '/' || unicode == '^' ||
                    unicode == '(' || unicode == ')' ||
                    unicode == '[' || unicode == ']' ||
                    unicode == '{' || unicode == '}' ||
                    unicode == '.' || unicode == ',') {
                    char c = static_cast<char>(unicode);
                    if (c == '.') c = ',';

                    if (c >= 'a' && c <= 'f')
                    {
                        c = c - 'a' + 'A';
                    }

                    std::string znak(1, c);

                    if (NumberSystems::digitAllowed(znak, context.currentSystem))
                    {
                        if (c == '^' && context.interaction.editing) {
                            const auto raw = context.expression.get();
                            const auto chars = sf::String::fromUtf8(raw.begin(), raw.end());
                            const auto at = context.expression.cursor();
                            // Puste pole: drugie ^ anuluje utworzony indeks.
                            if (!context.expression.hasSelection() && at > 0 && chars[at - 1] == U'^') {
                                context.expression.removeLast();
                                return;
                            }
                            // Nie dopuszczamy trzeciego poziomu indeksu.
                            const auto layout = makePowerVisual(raw, context.mathFont, 28);
                            int depth = 0;
                            for (const auto& exponent : layout.exponents)
                                if (at >= exponent.start && at <= exponent.end)
                                    depth = std::max(depth, exponent.level);
                            if (depth >= 2) return;
                        }
                        if (c == '^') {
                            const auto raw = context.expression.get();
                            const auto chars = sf::String::fromUtf8(raw.begin(), raw.end());
                            const auto at = context.expression.cursor();
                            // Gdy brak podstawy, utworz potegę liczby 1.
                            if (at == 0 || chars[at-1] == U'+' || chars[at-1] == U'-' ||
                                chars[at-1] == U'*' || chars[at-1] == U'/' ||
                                chars[at-1] == U'(' || chars[at-1] == U'[' || chars[at-1] == U'{')
                                context.expression.add("1");
                        }
                        context.expression.add(znak);
                    }
                }
            }
}
