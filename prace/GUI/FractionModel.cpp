#include "FractionModel.h"
#include <SFML/System/String.hpp>
#include <algorithm>
#include <vector>

namespace FractionModel {
namespace {
std::string utf8(const sf::String& value) {
    const auto bytes = value.toUtf8();
    return std::string(bytes.begin(), bytes.end());
}
}
std::string emptyFraction() {
    sf::String s;
    s += Begin; s += Separator; s += End;
    return utf8(s);
}
std::vector<Span> spans(const std::string& expression) {
    const auto chars = sf::String::fromUtf8(expression.begin(), expression.end());
    std::vector<Span> result;
    std::vector<std::size_t> stack;
    for (std::size_t i=0; i<chars.getSize(); ++i) {
        if (chars[i] == Begin) {
            if (stack.size() >= 2) continue;
            result.push_back({i, i, i, static_cast<int>(stack.size())+1});
            stack.push_back(result.size()-1);
        } else if (chars[i] == Separator && !stack.empty()) {
            result[stack.back()].separator = i;
        } else if (chars[i] == End && !stack.empty()) {
            result[stack.back()].end = i;
            stack.pop_back();
        }
    }
    result.erase(std::remove_if(result.begin(), result.end(), [](const Span& s) {
        return s.separator <= s.begin || s.end <= s.separator;
    }), result.end());
    return result;
}
int nestingAt(const std::string& expression, std::size_t cursor) {
    int depth = 0;
    for (const auto& span : spans(expression))
        if (span.begin < cursor && cursor <= span.end) depth = std::max(depth, span.level);
    return depth;
}
bool horizontalFieldCursor(const std::string& expression, std::size_t cursor,
                           bool right, std::size_t& destination) {
    const auto all = spans(expression);
    const Span* selected = nullptr;
    std::size_t target = cursor;
    // Innermost matching fraction wins when structures are nested.
    for (const auto& f : all) {
        bool matched = true;
        std::size_t next = cursor;
        if (right) {
            if (cursor == f.begin) next = f.begin + 1;          // 1 -> 2
            else if (cursor == f.separator) next = f.end + 1;  // 3 -> 6
            else if (cursor == f.end) next = f.end + 1;        // 5 -> 6
            else matched = false;
        } else {
            if (cursor == f.begin + 1) next = f.begin;        // 2 -> 1
            else if (cursor == f.separator + 1) next = f.begin; // 4 -> 1
            else if (cursor == f.end + 1) next = f.separator; // 6 -> 3
            else matched = false;
        }
        if (matched && (!selected || f.level > selected->level)) {
            selected = &f;
            target = next;
        }
    }
    if (!selected) return false;
    destination = target;
    return true;
}
bool verticalFieldCursor(const std::string& expression, std::size_t cursor,
                         bool down, std::size_t& destination) {
    const auto all = spans(expression);
    const Span* selected = nullptr;
    for (const auto& span : all) {
        // Cursor positions are between Unicode codepoints. The numerator
        // occupies (begin, separator], denominator (separator, end].
        const bool inNumerator = cursor > span.begin && cursor <= span.separator;
        const bool inDenominator = cursor > span.separator && cursor <= span.end;
        if ((down && inNumerator) || (!down && inDenominator)) {
            if (!selected || span.level > selected->level) selected = &span;
        }
    }
    if (!selected) return false;
    const std::size_t sourceStart = down ? selected->begin + 1 : selected->separator + 1;
    const std::size_t targetStart = down ? selected->separator + 1 : selected->begin + 1;
    const std::size_t targetEnd = down ? selected->end : selected->separator;
    const std::size_t offset = cursor > sourceStart ? cursor - sourceStart : 0;
    destination = std::min(targetStart + offset, targetEnd);
    return true;
}
bool toArithmetic(const std::string& expression, std::string& arithmetic) {
    const auto chars = sf::String::fromUtf8(expression.begin(), expression.end());
    sf::String output;
    std::vector<bool> hasSeparator;
    for (std::size_t i=0; i<chars.getSize(); ++i) {
        const auto ch=chars[i];
        if (ch == Begin) {
            if (hasSeparator.size() >= 2) return false;
            output += U'('; output += U'(';
            hasSeparator.push_back(false);
        } else if (ch == Separator) {
            if (hasSeparator.empty() || hasSeparator.back()) return false;
            hasSeparator.back()=true;
            output += U')'; output += U'/'; output += U'(';
            if (i+1 < chars.getSize() && chars[i+1] == End) output += U'1';
        } else if (ch == End) {
            if (hasSeparator.empty() || !hasSeparator.back()) return false;
            output += U')'; output += U')';
            hasSeparator.pop_back();
        } else output += ch;
    }
    if (!hasSeparator.empty()) return false;
    arithmetic=utf8(output);
    return true;
}
}
