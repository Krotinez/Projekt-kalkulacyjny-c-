#pragma once
#include <SFML/System/String.hpp>
#include <cstddef>
#include <string>
#include <vector>

// Edytor tekstowy UTF-32: indeksy odnosza sie do znakow, a nie bajtow UTF-8.
class Expression {
public:
    void add(const std::string& text);
    void removeLast();
    void deleteForward();
    void clear();
    std::string get() const;
    std::size_t cursor() const { return caret; }
    std::size_t selectionStart() const { return anchor < caret ? anchor : caret; }
    std::size_t selectionEnd() const { return anchor > caret ? anchor : caret; }
    bool hasSelection() const { return anchor != caret; }
    void setCursor(std::size_t pos, bool extend = false);
    void selectAll();
    void moveHorizontal(int direction, bool byWord, bool extend);
    void moveLineBoundary(bool end, bool extend);
    void moveVertical(int direction, bool extend);
    void undo();
    void redo();
    void copy() const;
    void cut();
    void paste();
private:
    struct Snapshot { sf::String value; std::size_t caret, anchor; };
    sf::String value;
    std::size_t caret = 0, anchor = 0;
    std::vector<Snapshot> past, future;
    void save();
    void eraseSelection();
    static bool isWord(char32_t c);
};
