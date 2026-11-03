#include "ArithmeticOperations.h"
#include "RootMath.h"
#include "LogMath.h"
#include "FactorialMath.h"
#include <cmath>
#include <stdexcept>

namespace ArithmeticOperations {
double unaryMinus(double value) { return -value; }
double absolute(double value) { return std::fabs(value); }
double percent(double value) { return value / 100.0; }
double permille(double value) { return value / 1000.0; }
double factorial(double value) { return calculateFactorial(value); }
double power(double base, double exponent) {
    const double result = std::pow(base, exponent);
    if (!std::isfinite(result))
        throw std::runtime_error("Nieprawidlowa potega lub wynik poza zakresem");
    return result;
}
double divide(double numerator, double denominator) {
    if (denominator == 0.0) throw std::runtime_error("Dzielenie przez zero");
    return numerator / denominator;
}
double root(double degree, double radicand) { return calculateRealRoot(degree, radicand); }
double logarithm(double base, double argument) { return calculateRealLog(base, argument); }
}
