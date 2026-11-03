#include "NumberFormatting.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace NumberFormatting {
std::string format(double v) {
    // Uzywaj tych samych zasad zaokraglania dla wyniku i krokow.
    return formatStep(v, 10);
}
std::string formatBase(double v, int base) {
    if (base == 10) return format(v);
    if (!std::isfinite(v) || std::abs(v) > 9007199254740991.0)
        throw std::runtime_error("Wynik poza zakresem systemu");
    const char* digits = "0123456789ABCDEF";
    const bool negative = v < 0;
    const long double absolute = std::fabs(static_cast<long double>(v));
    auto n = static_cast<unsigned long long>(std::floor(absolute));
    std::string out;
    do { out.insert(out.begin(), digits[n % base]); n /= base; } while (n);
    long double fraction = absolute - std::floor(absolute);
    if (fraction > 1e-16L) {
        out += ',';
        constexpr int maxDigits = 64;
        for (int i = 0; i < maxDigits && fraction > 1e-16L; ++i) {
            fraction *= base;
            int d = static_cast<int>(std::floor(fraction));
            if (d >= base) d = base - 1;
            out += digits[d];
            fraction -= d;
        }
        if (fraction > 1e-16L) out += "...";
    }
    return negative ? "-" + out : out;
}
std::string formatStep(double v, int base) {
    if (base != 10) return formatBase(v, base);
    if (!std::isfinite(v)) throw std::runtime_error("Wynik poza zakresem");
    if (v == 0) return "0";
    if (std::fabs(v) < 1e-12) {
        std::ostringstream tiny;
        tiny << std::setprecision(12) << v;
        std::string result = tiny.str();
        for (char& c : result) if (c == '.') c = ',';
        return result;
    }

    // Round the display, rather than truncating a binary floating-point value
    // such as 0.03 to 0.029999999999. Keep the original value in Node::value.
    constexpr int places = 12;
    constexpr long double factor = 1000000000000.0L;
    const long double scaled = static_cast<long double>(v) * factor;
    const long double nearest = std::round(scaled);
    const long double noise = std::max(0.0001L,
        16.0L * std::numeric_limits<double>::epsilon() *
        std::fabs(static_cast<long double>(v)) * factor);
    const bool moreDigits = std::fabs(v) < 1e10L &&
        std::fabs(scaled - nearest) > noise;

    std::ostringstream out;
    out << std::fixed << std::setprecision(places) << v;
    std::string result = out.str();
    const auto dot = result.find('.');
    if (dot != std::string::npos) {
        while (!result.empty() && result.back() == '0') result.pop_back();
        if (!result.empty() && result.back() == '.') result.pop_back();
    }
    for (char& c : result) if (c == '.') c = ',';
    if (moreDigits) result += "...";
    return result;
}

}
