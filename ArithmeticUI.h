#pragma once
#include "CalculationSteps.h"
#include "../GUI/ExpressionPanel.h"
#include <cstddef>
#include <string>
#include <vector>

namespace ArithmeticUI {
// Stan interakcji z panelem wyrazenia i wyniku.
// Nie przechowuje danych obliczeniowych ani obiektow SFML.
struct EditorInteractionState {
    bool editing = false;
    bool panelFocused = false;
    bool resultFocused = false;
    bool selectingWithMouse = false;
    std::size_t resultCaret = 0;
    std::size_t resultAnchor = 0;
};

// Tekst widoczny w panelu: wyrazenie i opcjonalnie wynik.
std::string displayedText(const std::string& expression, const std::string& resultText);
std::size_t codepoints(const std::string& s);
void makeHighlights(const std::string& source, const StepCalculation& calculation,
                    std::vector<ExpressionHighlight>& highlights, bool showBlue, bool showGreen);
}
