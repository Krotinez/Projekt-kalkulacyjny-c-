#include "LogModel.h"
#include <SFML/System/String.hpp>
#include <vector>
#include <functional>
namespace LogModel {
std::string emptyLog() {
    sf::String s;
    s += Begin; s += Separator; s += End;
    const auto bytes=s.toUtf8();
    return std::string(bytes.begin(),bytes.end());
}
namespace {
struct Span { std::size_t begin, separator, end; };
std::vector<Span> spans(const std::string& expression) {
    const auto chars = sf::String::fromUtf8(expression.begin(), expression.end());
    std::vector<Span> result;
    std::vector<std::size_t> opens;
    for (std::size_t i = 0; i < chars.getSize(); ++i) {
        if (chars[i] == Begin) opens.push_back(i);
        else if (chars[i] == Separator && !opens.empty()) {
            result.push_back({opens.back(), i, chars.getSize()});
        } else if (chars[i] == End && !opens.empty()) {
            const auto begin = opens.back();
            for (auto it = result.rbegin(); it != result.rend(); ++it) {
                if (it->begin == begin && it->end == chars.getSize()) {
                    it->end = i;
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
    const auto all = spans(expression);
    for (auto it = all.rbegin(); it != all.rend(); ++it) {
        if (right && cursor == it->separator) {
            target = it->separator + 1;
            return true;
        }
        if (!right && cursor == it->separator + 1) {
            target = it->separator;
            return true;
        }
    }
    return false;
}
bool inBase(const std::string& expression, std::size_t cursor) {
    for (const auto& span : spans(expression)) {
        if (cursor > span.begin && cursor <= span.separator) return true;
    }
    return false;
}
bool toArithmetic(const std::string& expression, std::string& arithmetic) {
    const auto chars = sf::String::fromUtf8(expression.begin(), expression.end());
    const auto utf8 = [](const sf::String& value) {
        const auto bytes = value.toUtf8();
        return std::string(bytes.begin(), bytes.end());
    };
    std::function<bool(std::size_t&, sf::String&, bool)> parse;
    parse = [&](std::size_t& i, sf::String& out, bool inside) -> bool {
        while (i < chars.getSize()) {
            const auto ch = chars[i];
            if (ch == Separator || ch == End) return inside;
            if (ch != Begin) { out += ch; ++i; continue; }
            ++i;
            sf::String base, argument;
            if (!parse(i, base, true) || i >= chars.getSize() || chars[i] != Separator) return false;
            ++i;
            if (!parse(i, argument, true) || i >= chars.getSize() || chars[i] != End) return false;
            ++i;
            if (argument.isEmpty()) return false;
            if (base.isEmpty()) base += U'1', base += U'0';
            const std::string replacement = "log((" + utf8(base) + ");(" + utf8(argument) + "))";
            out += sf::String::fromUtf8(replacement.begin(), replacement.end());
        }
        return !inside;
    };
    std::size_t i = 0;
    sf::String output;
    if (!parse(i, output, false) || i != chars.getSize()) return false;
    arithmetic = utf8(output);
    return true;
}

}
