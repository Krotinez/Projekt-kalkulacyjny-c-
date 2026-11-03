#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <optional>
#include <string>

// Explicit transitions shared by direct navigation and traversal through disabled buttons.
namespace NavigationRules {
std::optional<std::string> explicitTarget(const std::string& label,
                                          const std::string& action,
                                          sf::Keyboard::Key direction);
}
