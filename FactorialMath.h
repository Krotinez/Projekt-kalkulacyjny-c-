#pragma once
#include <cmath>
#include <stdexcept>

// Silnia jest okreslona w kalkulatorze tylko dla calkowitych n >= 0.
// Wynik jest przechowywany jako double, dlatego 170! to najwieksza
// silnia mieszczaca sie w skonczonym zakresie tego typu.
inline double calculateFactorial(double value) {
    if (!std::isfinite(value) || value < 0.0 || std::trunc(value) != value)
        throw std::runtime_error("Silnia wymaga nieujemnej liczby calkowitej");
    if (value > 170.0)
        throw std::runtime_error("Silnia poza zakresem obliczen (maks. 170!)");
    double result = 1.0;
    for (int n = 2; n <= static_cast<int>(value); ++n)
        result *= static_cast<double>(n);
    return result;
}
