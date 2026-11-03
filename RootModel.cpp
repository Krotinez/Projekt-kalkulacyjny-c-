#include "RootModel.h"
#include <SFML/System/String.hpp>
#include <vector>
#include <functional>
namespace RootModel {
std::string emptyRoot() {
    sf::String text;
    text += Begin;
    text += Separator;
    text += End;
    auto bytes = text.toUtf8();
    return std::string(bytes.begin(), bytes.end());
}
namespace {
struct Span { std::size_t begin, separator, end; };
std::vector<Span> spans(const std::string& expression) {
    const auto chars = sf::String::fromUtf8(expression.begin(), expression.end());
    std::vector<Span> result;
    std::vector<std::size_t> opens;
    for (std::size_t i=0; i<chars.getSize(); ++i) {
        if (chars[i] == Begin) opens.push_back(i);
        else if (chars[i] == Separator && !opens.empty()) {
            const auto begin=opens.back();
            result.push_back({begin,i,chars.getSize()});
        } else if (chars[i] == End && !opens.empty()) {
            const auto begin=opens.back();
            for (auto it=result.rbegin(); it!=result.rend(); ++it) {
                if (it->begin == begin && it->end == chars.getSize()) {
                    it->end=i;
                    break;
                }
            }
            opens.pop_back();
        }
    }
    return result;
}
}
bool horizontalFieldCursor(const std::string& expression, std::size_t cursor,
                           bool right, std::size_t& target) {
    const auto all=spans(expression);
    for (auto it=all.rbegin(); it!=all.rend(); ++it) {
        if (right && cursor == it->separator) {
            target=it->separator+1;
            return true;
        }
        if (!right && cursor == it->separator+1) {
            target=it->separator;
            return true;
        }
    }
    return false;
}
bool inDegree(const std::string& expression, std::size_t cursor) {
    for (const auto& s : spans(expression))
        if (cursor > s.begin && cursor <= s.separator) return true;
    return false;
}

bool toArithmetic(const std::string& expression, std::string& arithmetic) {
    const auto chars = sf::String::fromUtf8(expression.begin(), expression.end());
    const std::size_t n = chars.getSize();
    auto utf8 = [](const sf::String& value) {
        const auto bytes = value.toUtf8();
        return std::string(bytes.begin(), bytes.end());
    };
    // Recursive parser of the root markers; ordinary characters, including
    // graphical fraction markers, pass through unchanged.
    std::function<bool(std::size_t&, sf::String&, bool)> parse;
    parse = [&](std::size_t& i, sf::String& out, bool inside) -> bool {
        while (i < n) {
            const char32_t ch = chars[i];
            if (ch == End || ch == Separator) {
                if (!inside) return false;
                return true;
            }
            if (ch != Begin) { out += ch; ++i; continue; }
            ++i;
            sf::String degree, radicand;
            if (!parse(i, degree, true) || i >= n || chars[i] != Separator) return false;
            ++i;
            if (!parse(i, radicand, true) || i >= n || chars[i] != End) return false;
            ++i;
            if (radicand.isEmpty()) return false;
            // Square root when the degree field is empty.
            if (degree.isEmpty()) degree += U'2';
            const auto d = utf8(degree), r = utf8(radicand);
            const auto replacement = "root((" + d + ");(" + r + "))";
            out += sf::String::fromUtf8(replacement.begin(), replacement.end());
        }
        return !inside;
    };
    std::size_t i = 0;
    sf::String result;
    if (!parse(i, result, false) || i != n) return false;
    arithmetic = utf8(result);
    return true;
}

}
