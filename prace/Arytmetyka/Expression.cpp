#include "Expression.h"
#include <SFML/Window/Clipboard.hpp>
#include <algorithm>

namespace {
char32_t ghostFor(char32_t c) {
    switch(c) { case U'(': return U'\uE130'; case U'[': return U'\uE131'; case U'{': return U'\uE132'; case U'<': return U'\uE133'; default: return 0; }
}
char32_t realFor(char32_t c) {
    switch(c) { case U')': return U'\uE130'; case U']': return U'\uE131'; case U'}': return U'\uE132'; case U'>': return U'\uE133'; default: return 0; }
}
}


bool Expression::isWord(char32_t c) {
    return (c >= U'0' && c <= U'9') || (c >= U'A' && c <= U'Z') ||
           (c >= U'a' && c <= U'z') || c == U',' || c == U'.';
}
void Expression::save() {
    past.push_back({value, caret, anchor});
    if (past.size() > 200) past.erase(past.begin());
    future.clear();
}
void Expression::eraseSelection() {
    if (!hasSelection()) return;
    const auto start = selectionStart();
    value.erase(start, selectionEnd() - start);
    caret = anchor = start;
}
void Expression::add(const std::string& text) {
    sf::String inserted = sf::String::fromUtf8(text.begin(), text.end());
    if (inserted.isEmpty()) return;
    save(); eraseSelection();
    // A power is an explicit editable field. Its closing parenthesis is
    // present from the start, so digits entered after exiting cannot extend it.
    if (inserted.getSize() == 1 && inserted[0] == U'^') {
        value.insert(caret, sf::String("^()"));
        caret += 2; // immediately before the closing parenthesis
        anchor = caret;
        return;
    }
    // Reguly wpisywania operatorow i przecinka dotycza pojedynczych znakow.
    if (inserted.getSize() == 1) {
        const char32_t c = inserted[0];
        const auto isOperator = [](char32_t ch) {
            return ch == U'+' || ch == U'-' || ch == U'*' || ch == U'/' ||
                   ch == U'×' || ch == U'÷';
        };
        const bool atStart = caret == 0 || value[caret - 1] == U'\n' ||
                             value[caret - 1] == U'(' || value[caret - 1] == U'[' ||
                             value[caret - 1] == U'{';
        if (c == U',' || c == U'.') {
            // A decimal separator is unique within a numeric literal. If the
            // immediately preceding character is a separator, replace it;
            // if the literal already has one, ignore the new separator.
            if (caret > 0 && (value[caret - 1] == U',' || value[caret - 1] == U'.')) {
                value.erase(caret - 1, 1);
                --caret;
            } else {
                std::size_t begin = caret;
                while (begin > 0 && ((value[begin - 1] >= U'0' && value[begin - 1] <= U'9') ||
                       value[begin - 1] == U',' || value[begin - 1] == U'.')) --begin;
                for (std::size_t i = begin; i < caret; ++i) {
                    if (value[i] == U',' || value[i] == U'.') {
                        anchor = caret;
                        return;
                    }
                }
            }
            if (atStart || (caret > 0 && isOperator(value[caret - 1]))) {
                value.insert(caret, sf::String("0")); ++caret;
            }
        } else if (isOperator(c)) {
            if (caret > 0 && isOperator(value[caret - 1])) {
                value.erase(caret - 1, 1); --caret;
            } else if (atStart && c != U'+' && c != U'-') {
                value.insert(caret, sf::String("0")); ++caret;
            }
        }
    }
    if (inserted.getSize() == 1) {
        const char32_t c = inserted[0];
        const char32_t ghost = ghostFor(c);
        if (ghost) {
            value.insert(caret, sf::String(c));
            value.insert(caret + 1, sf::String(ghost));
            ++caret; anchor = caret;
            return;
        }
        const char32_t matchingGhost = realFor(c);
        if (matchingGhost) {
            // Zamykamy najblizszy pasujacy widmowy nawias, ale tylko
            // gdy po drodze nie ma innego, jeszcze niezamknietego nawiasu.
            std::size_t match = value.getSize();
            for (std::size_t i = caret; i < value.getSize(); ++i) {
                if (value[i] == matchingGhost) { match = i; break; }
                if (value[i] == U'\uE130' || value[i] == U'\uE131' ||
                    value[i] == U'\uE132' || value[i] == U'\uE133') break;
            }
            if (match < value.getSize()) {
                value.erase(match, 1);
                value.insert(caret, sf::String(c));
                ++caret; anchor = caret;
                return;
            }
        }
    }
    value.insert(caret, inserted);
    caret += inserted.getSize(); anchor = caret;
}
// Structure-aware editing: a field boundary must never be removed as an
// ordinary character. Empty mathematical objects are removed atomically.
namespace {
bool isBegin(char32_t c) {
    return c == U'\uE100' || c == U'\uE110' || c == U'\uE120';
}
bool isSeparator(char32_t c) {
    return c == U'\uE101' || c == U'\uE111' || c == U'\uE121';
}
bool isPowerBoundary(const sf::String& value, std::size_t index) {
    if (index >= value.getSize()) return false;
    if (value[index] == U'^' && index + 1 < value.getSize() && value[index+1] == U'(') return true;
    if (value[index] == U'(' && index > 0 && value[index-1] == U'^') return true;
    if (value[index] != U')') return false;
    // A closing bracket belongs to a power if its matching opening bracket
    // immediately follows ^. Supports nested brackets in the exponent.
    int depth = 0;
    for (std::size_t i = index + 1; i > 0;) {
        --i;
        if (value[i] == U')') ++depth;
        else if (value[i] == U'(') {
            --depth;
            if (depth == 0) return i > 0 && value[i-1] == U'^';
        }
    }
    return false;
}
bool isEnd(char32_t c) {
    return c == U'\uE102' || c == U'\uE112' || c == U'\uE122';
}
char32_t matchingEnd(char32_t begin) {
    return begin == U'\uE100' ? U'\uE102' :
           begin == U'\uE110' ? U'\uE112' : U'\uE122';
}
char32_t matchingSeparator(char32_t begin) {
    return begin == U'\uE100' ? U'\uE101' :
           begin == U'\uE110' ? U'\uE111' : U'\uE121';
}
// Locate a complete, empty fraction/root/log enclosing or adjacent to cursor.
bool emptyStructureAt(const sf::String& value, std::size_t cursor,
                      std::size_t& start, std::size_t& length) {
    // Explicit empty power field ^() is one editable object.
    for (std::size_t i = 0; i + 2 < value.getSize(); ++i) {
        if (value[i] == U'^' && value[i+1] == U'(' && value[i+2] == U')' &&
            cursor >= i && cursor <= i+3) {
            start = i; length = 3; return true;
        }
    }
    for (std::size_t i = 0; i + 2 < value.getSize(); ++i) {
        if (!isBegin(value[i]) || value[i+1] != matchingSeparator(value[i]) ||
            value[i+2] != matchingEnd(value[i])) continue;
        if (cursor >= i && cursor <= i+3) { start = i; length = 3; return true; }
    }
    return false;
}
}
void Expression::removeLast() {
    if (hasSelection()) { save(); eraseSelection(); return; }
    std::size_t start = 0, length = 0;
    if (emptyStructureAt(value, caret, start, length) && caret > start) {
        save(); value.erase(start, length); caret = anchor = start; return;
    }
    if (caret == 0) return;
    // Never damage a populated structure by deleting a delimiter.
    if (isBegin(value[caret-1]) || isSeparator(value[caret-1]) ||
        isEnd(value[caret-1]) || isPowerBoundary(value, caret-1)) return;
    save(); value.erase(caret-1, 1); --caret; anchor = caret;
}
void Expression::deleteForward() {
    if (hasSelection()) { save(); eraseSelection(); return; }
    std::size_t start = 0, length = 0;
    if (emptyStructureAt(value, caret, start, length)) {
        save(); value.erase(start, length); caret = anchor = start; return;
    }
    if (caret >= value.getSize()) return;
    // Preserve the boundaries of nonempty mathematical structures.
    if (isBegin(value[caret]) || isSeparator(value[caret]) ||
        isEnd(value[caret]) || isPowerBoundary(value, caret)) return;
    save(); value.erase(caret, 1); anchor = caret;
}
void Expression::replaceForBaseConversion(const std::string& text, std::size_t newCursor, std::size_t newAnchor) {
    const auto converted=sf::String::fromUtf8(text.begin(),text.end());
    if(converted==value) return;
    save();
    value=converted;
    caret=std::min(newCursor,value.getSize());
    anchor=std::min(newAnchor,value.getSize());
}
void Expression::clear() {
    if (value.isEmpty()) return;
    save(); value.clear(); caret = anchor = 0;
}
std::string Expression::get() const { auto utf8 = value.toUtf8(); return std::string(utf8.begin(), utf8.end()); }
void Expression::setCursor(std::size_t pos, bool extend) {
    caret = std::min(pos, value.getSize());
    if (!extend) anchor = caret;
}
void Expression::selectAll() { anchor = 0; caret = value.getSize(); }
void Expression::moveHorizontal(int direction, bool byWord, bool extend) {
    std::size_t next = caret;
    if (direction < 0 && next > 0) {
        --next;
        if (byWord) {
            if (isWord(value[next])) while (next > 0 && isWord(value[next-1])) --next;
            else while (next > 0 && !isWord(value[next-1])) --next;
        }
    } else if (direction > 0 && next < value.getSize()) {
        if (byWord) {
            if (isWord(value[next])) while (next < value.getSize() && isWord(value[next])) ++next;
            else while (next < value.getSize() && !isWord(value[next])) ++next;
        } else ++next;
    }
    setCursor(next, extend);
}
void Expression::moveLineBoundary(bool end, bool extend) {
    std::size_t pos = caret;
    if (end) { while (pos < value.getSize() && value[pos] != U'\n') ++pos; }
    else { while (pos > 0 && value[pos-1] != U'\n') --pos; }
    setCursor(pos, extend);
}
void Expression::moveVertical(int direction, bool extend) {
    std::size_t start = caret;
    while (start > 0 && value[start-1] != U'\n') --start;
    const auto column = caret - start;
    if (direction < 0) {
        if (start == 0) { setCursor(0, extend); return; }
        std::size_t previous = start - 1, begin = previous;
        while (begin > 0 && value[begin-1] != U'\n') --begin;
        setCursor(begin + std::min(column, previous - begin), extend);
    } else {
        std::size_t end = caret;
        while (end < value.getSize() && value[end] != U'\n') ++end;
        if (end == value.getSize()) { setCursor(end, extend); return; }
        std::size_t begin = end + 1, nextEnd = begin;
        while (nextEnd < value.getSize() && value[nextEnd] != U'\n') ++nextEnd;
        setCursor(begin + std::min(column, nextEnd - begin), extend);
    }
}
void Expression::undo() {
    if (past.empty()) return;
    future.push_back({value, caret, anchor});
    const auto s = past.back(); past.pop_back();
    value = s.value; caret = s.caret; anchor = s.anchor;
}
void Expression::redo() {
    if (future.empty()) return;
    past.push_back({value, caret, anchor});
    const auto s = future.back(); future.pop_back();
    value = s.value; caret = s.caret; anchor = s.anchor;
}
void Expression::copy() const {
    if (hasSelection()) sf::Clipboard::setString(value.substring(selectionStart(), selectionEnd()-selectionStart()));
}
void Expression::cut() {
    if (!hasSelection()) return;
    copy(); save(); eraseSelection();
}
void Expression::paste() {
    auto clipboard = sf::Clipboard::getString();
    if (clipboard.isEmpty()) return;
    save(); eraseSelection(); value.insert(caret, clipboard);
    caret += clipboard.getSize(); anchor = caret;
}
