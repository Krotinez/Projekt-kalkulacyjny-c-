#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

// Layout wizualny ^ bez zmiany tekstu przekazywanego do silnika.
// Pozycje indeksowane znakami Unicode (jak kursor edytora).
struct PowerVisual {
    std::vector<float> x;
    std::vector<float> rise; // Wysokosc RYSOWANYCH znakow.
    std::vector<float> caretRise; // Wysokosc kursora na granicy indeksu.
    std::vector<unsigned> size;
    float width = 0.f;
    struct Exponent { std::size_t start, end; int level; };
    std::vector<Exponent> exponents;
};
PowerVisual makePowerVisual(const std::string& line, const sf::Font& font, unsigned normalSize);
void drawPowerVisual(sf::RenderWindow& window, const std::string& line,
    const sf::Font& font, unsigned normalSize, sf::Vector2f origin, sf::Color color);
