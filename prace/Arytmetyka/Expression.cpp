#include "Expression.h"
#include <SFML/Window/Clipboard.hpp>
#include <algorithm>

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
    value.insert(caret, inserted);
    caret += inserted.getSize(); anchor = caret;
}
void Expression::removeLast() {
    if (!hasSelection() && caret == 0) return;
    save();
    if (hasSelection()) eraseSelection();
    else { value.erase(caret - 1, 1); --caret; anchor = caret; }
}
void Expression::deleteForward() {
    if (!hasSelection() && caret >= value.getSize()) return;
    save();
    if (hasSelection()) eraseSelection();
    else value.erase(caret, 1);
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
