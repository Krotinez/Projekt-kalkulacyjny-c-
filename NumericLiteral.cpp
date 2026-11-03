#include "NumericLiteral.h"
#include <stdexcept>

namespace {
int digitValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}
}

namespace NumericLiteral {
double parse(const std::string& text, std::size_t& position, int base) {
    if (base != 2 && base != 8 && base != 10 && base != 16)
        throw std::runtime_error("Nieznany system liczbowy");
    const auto start = position;
    long double integer = 0, fraction = 0, divisor = 1;
    bool any = false;
    while (position < text.size()) {
        const int d = digitValue(text[position]);
        if (d < 0) break;
        if (d >= base) throw std::runtime_error("Cyfra niedozwolona w tym systemie");
        integer = integer * base + d;
        ++position;
        any = true;
    }
    if (position < text.size() && (text[position] == ',' || text[position] == '.')) {
        ++position;
        while (position < text.size()) {
            const int d = digitValue(text[position]);
            if (d < 0) break;
            if (d >= base) throw std::runtime_error("Cyfra niedozwolona w tym systemie");
            divisor *= base;
            fraction += d / divisor;
            ++position;
            any = true;
        }
    }
    if (!any || position == start) throw std::runtime_error("Oczekiwano liczby");
    return static_cast<double>(integer + fraction);
}
}
