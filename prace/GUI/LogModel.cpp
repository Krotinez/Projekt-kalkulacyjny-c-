#include "LogModel.h"
#include <SFML/System/String.hpp>
#include <vector>
#include <algorithm>
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
// Main horizontal path: outside -> argument/radicand -> outside.
// The first encoded field (degree/base) is accessible via Up/Down only.
bool horizontalFieldCursor(const std::string& expression, std::size_t cursor,
                           bool right, std::size_t& target) {
    const auto all = spans(expression);
    const Span* selected = nullptr;
    for (const auto& s : all) {
        const auto length = sf::String::fromUtf8(expression.begin(), expression.end()).getSize();
        if (s.end >= length) continue;
        bool matched = false;
        std::size_t next = cursor;
        if (right) {
            if (cursor == s.begin) { next = s.separator + 1; matched = true; }
            else if (cursor == s.separator || cursor == s.end) { next = s.end + 1; matched = true; }
        } else {
            if (cursor == s.separator + 1 || cursor == s.begin + 1) { next = s.begin; matched = true; }
            else if (cursor == s.end + 1) { next = s.end; matched = true; }
        }
        if (matched && (!selected || s.begin >= selected->begin)) { selected = &s; target = next; }
    }
    return selected != nullptr;
}
bool verticalFieldCursor(const std::string& expression, std::size_t cursor,
                         bool down, std::size_t& target) {
    const auto all = spans(expression);
    const Span* selected = nullptr;
    for (const auto& s : all) {
        const auto length = sf::String::fromUtf8(expression.begin(), expression.end()).getSize();
        if (s.end >= length) continue;
        const bool degree = cursor > s.begin && cursor <= s.separator;
        const bool argument = cursor > s.separator && cursor <= s.end;
        // Down: argument -> base. Up: base -> argument.
        if (((down && argument) || (!down && degree)) &&
            (!selected || s.begin >= selected->begin)) selected = &s;
    }
    if (!selected) return false;
    const std::size_t source = down ? selected->separator + 1 : selected->begin + 1;
    const std::size_t destination = down ? selected->begin + 1 : selected->separator + 1;
    const std::size_t last = down ? selected->separator : selected->end;
    target = std::min(destination + (cursor - source), last);
    return true;
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
