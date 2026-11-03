#pragma once
#include <cmath>
#include <stdexcept>
inline double calculateRealLog(double base, double argument) {
    if (!std::isfinite(base) || !std::isfinite(argument) || base <= 0.0 || base == 1.0 || argument <= 0.0)
        throw std::runtime_error("Nieprawidlowa podstawa lub argument logarytmu");
    const double result = std::log(argument) / std::log(base);
    if (!std::isfinite(result)) throw std::runtime_error("Wynik logarytmu poza zakresem");
    const double nearest = std::round(result);
    return std::abs(result - nearest) < 1e-12 ? nearest : result;
}
