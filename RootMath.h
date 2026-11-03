#pragma once
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

// Real-valued nth root, including negative radicands for odd integer degrees.
inline double calculateRealRoot(double degree, double radicand) {
    if (!std::isfinite(degree) || !std::isfinite(radicand))
        throw std::runtime_error("Nieprawidlowy stopien lub liczba pod pierwiastkiem");
    if (degree == 0.0)
        throw std::runtime_error("Stopien pierwiastka nie moze wynosic zero");
    if (radicand == 0.0 && degree < 0.0)
        throw std::runtime_error("Dzielenie przez zero w pierwiastku");
    double result;
    if (radicand < 0.0) {
        // Odd integer degree is required for a real negative root.
        if (std::trunc(degree) != degree || std::fmod(std::fabs(degree), 2.0) != 1.0)
            throw std::runtime_error("Ujemna liczba wymaga nieparzystego calkowitego stopnia pierwiastka");
        result = -std::pow(-radicand, 1.0 / degree);
    } else {
        result = std::pow(radicand, 1.0 / degree);
    }
    if (!std::isfinite(result)) throw std::runtime_error("Wynik pierwiastka poza zakresem");
    const double nearest = std::round(result);
    if (std::fabs(result-nearest) <= 1e-12 * std::max(1.0,std::fabs(result))) result=nearest;
    return result;
}
