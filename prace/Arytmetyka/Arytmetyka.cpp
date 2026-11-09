#include "Expression.h"
#include "CalculationSteps.h"
#include "ArithmeticUI.h"
#include "NumberSystem.h"
#include "NavigationRules.h"
#include "ExpressionKeyHandler.h"
#include "ExpressionTextHandler.h"
#include "ArithmeticButtonHandler.h"
#include "MouseSelectionHandler.h"
#include "ButtonFocusHandler.h"
#include "ArithmeticFrame.h"

#include "../GUI/Button.h"
#include "ArithmeticButtons.h"
#include "../GUI/ButtonNavigation.h"
#include "../GUI/ExpressionPanel.h"
#include "../GUI/FractionModel.h"
#include "../GUI/RootModel.h"
#include "../GUI/LogModel.h"
#include "../GUI/PowerVisual.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window/Clipboard.hpp>
#include <algorithm>

#include <optional>
#include <string>
#include <vector>


// Przejscie miedzy polami aktywnej struktury: spacja / prawy przycisk.
// Pozycje liczymy w znakach Unicode, nie bajtach UTF-8.
static bool advanceStructureField(Expression& expression, ExpressionPanel& panel,
                                  const std::string& action) {
    if (expression.hasSelection()) return false;
    const auto raw = expression.get();
    const auto chars = sf::String::fromUtf8(raw.begin(), raw.end());
    const auto at = expression.cursor();
    const char32_t begin = action == "FRACTION" ? FractionModel::Begin :
                           action == "ROOT" ? RootModel::Begin : LogModel::Begin;
    const char32_t separator = action == "FRACTION" ? FractionModel::Separator :
                               action == "ROOT" ? RootModel::Separator : LogModel::Separator;
    const char32_t end = action == "FRACTION" ? FractionModel::End :
                         action == "ROOT" ? RootModel::End : LogModel::End;
    struct Span { std::size_t start, sep, finish; };
    std::vector<Span> spans;
    std::vector<std::size_t> stack;
    for (std::size_t i=0; i<chars.getSize(); ++i) {
        if (chars[i] == begin) { spans.push_back({i,chars.getSize(),chars.getSize()}); stack.push_back(spans.size()-1); }
        else if (chars[i] == separator && !stack.empty()) spans[stack.back()].sep=i;
        else if (chars[i] == end && !stack.empty()) { spans[stack.back()].finish=i; stack.pop_back(); }
    }
    const Span* chosen=nullptr;
    for (const auto& span: spans) {
        if (span.sep >= span.finish || at <= span.start || at > span.finish) continue;
        if (!chosen || span.start > chosen->start) chosen=&span;
    }
    if (!chosen) return false;
    std::size_t target=at;
    if (action == "FRACTION") {
        target = at <= chosen->sep ? chosen->finish : chosen->finish+1;
    } else {
        // Pierwiastek/logarytm: argument -> stopien/podstawa -> poza strukture.
        target = at > chosen->sep ? chosen->sep : chosen->finish+1;
    }
    expression.setCursor(target);
    panel.ensureCursorVisible(expression.get(), expression.cursor());
    return true;
}

static bool advanceActiveStructure(Expression& expression, ExpressionPanel& panel,
                                   const sf::Font& font) {
    // W przypadku struktur zagniezdzonych wybieramy najglebsza aktywna.
    const auto raw=expression.get();
    const auto chars=sf::String::fromUtf8(raw.begin(),raw.end());
    const auto at=expression.cursor();
    std::vector<std::pair<std::size_t,std::string>> stack;
    for (std::size_t i=0;i<chars.getSize() && i<at;++i) {
        const auto c=chars[i];
        if (c==FractionModel::Begin || c==RootModel::Begin || c==LogModel::Begin)
            stack.push_back({i,c==FractionModel::Begin?"FRACTION":c==RootModel::Begin?"ROOT":"LOG"});
        else if (c==FractionModel::End || c==RootModel::End || c==LogModel::End) {
            if (!stack.empty()) stack.pop_back();
        }
    }
    if (!stack.empty()) return advanceStructureField(expression,panel,stack.back().second);
    // The innermost ^(...) is the active power field. Exiting it moves
    // the *real* insertion point past ')', rather than only moving the caret.
    std::vector<std::size_t> powerClosings;
    for (std::size_t i=0;i<chars.getSize();++i) {
        if (chars[i]!=U'^' || i+1>=chars.getSize() || chars[i+1]!=U'(') continue;
        std::size_t depth=0;
        for (std::size_t j=i+1;j<chars.getSize();++j) {
            if (chars[j]==U'(') ++depth;
            else if (chars[j]==U')' || chars[j]==U'\uE130') {
                if (--depth==0) {
                    if (at>=i+2 && at<=j) powerClosings.push_back(j);
                    break;
                }
            }
        }
    }
    if (!powerClosings.empty()) {
        expression.setCursor(powerClosings.back()+1);
        panel.setPowerOutsideAtEnd(false);
        panel.ensureCursorVisible(expression.get(),expression.cursor());
        return true;
    }
    return false;
}

void runArytmetyka(sf::RenderWindow& window, sf::Font& font, sf::Font& mathFont)
{
    Expression expression;
    ExpressionPanel expressionPanel(mathFont);

    std::string resultText = "";
    bool showBlue = true, showGreen = true;
    bool stepsMode = false; // W = wynik, K = kolejne przeksztalcenia
    std::string expressionAtResult;
    std::vector<ExpressionHighlight> stepHighlights;
    ArithmeticUI::EditorInteractionState interaction;
    auto selectionCopy = [&](const std::string& all) {
        const auto chars=sf::String::fromUtf8(all.begin(),all.end());
        const auto a=std::min(interaction.resultCaret,interaction.resultAnchor),b=std::max(interaction.resultCaret,interaction.resultAnchor);
        if(b>a) sf::Clipboard::setString(chars.substring(a,b-a));
    };
    auto displayedExpression = [&]() {
        return ArithmeticUI::displayedText(expression.get(), resultText);
    };
    std::optional<std::size_t> tableFocus;
    std::size_t lastVisibleCursor = 0;
    std::string lastVisibleExpression;

    NumberSystem currentSystem = NumberSystem::DEC;

    sf::Text title(font, "Arytmetyka", 40);
    title.setFillColor(sf::Color::White);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin({titleBounds.position.x + titleBounds.size.x / 2.f, titleBounds.position.y + titleBounds.size.y / 2.f});
    title.setPosition({600.f, 50.f});

    auto controls = createArithmeticButtons(font, mathFont);
    auto& defs = controls.defs;
    auto& buttons = controls.buttons;
    auto& tableButtons = controls.tableButtons;

    ButtonNavigation navigation(buttons);

    while (window.isOpen())
    {
        // Snapshot for detecting when the cursor leaves a fraction.
        const std::string fractionBefore = expression.get();
        const std::size_t fractionCursorBefore = expression.cursor();
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        while (const std::optional event = window.pollEvent())
        {
            // Prawy przycisk na przycisku struktury przechodzi do nastepnego pola.
            if (const auto* click=event->getIf<sf::Event::MouseButtonPressed>()) {
                if (click->button==sf::Mouse::Button::Right) {
                    bool handled=false;
                    for (std::size_t i=0;i<buttons.size();++i) {
                        const auto& action=defs[i].action;
                        if (action!="FRACTION" && action!="ROOT" && action!="LOG" && action!="^") continue;
                        if (!buttons[i].getGlobalBounds().contains(mousePos)) continue;
                        handled = action=="^" ? advanceActiveStructure(expression,expressionPanel,mathFont)
                            : advanceStructureField(expression,expressionPanel,action);
                        break;
                    }
                    if (handled) { interaction.resultFocused=false; interaction.editing=true; continue; }
                }
            }
            expressionPanel.handleEvent(*event, mousePos, displayedExpression());
            if (handleMouseSelectionEvent(*event, mousePos, {
                    expression, expressionPanel, navigation, resultText,
                    interaction
                })) continue;

            if (event->is<sf::Event::Closed>()) {
                window.close();
                return;
            }

            if (event->is<sf::Event::MouseMoved>() ||
                event->is<sf::Event::MouseButtonPressed>())
            {
                const auto* mouseEvent =
                    event->getIf<sf::Event::MouseButtonPressed>();

                if (!event->is<sf::Event::MouseButtonPressed>() ||
                    mouseEvent->button == sf::Mouse::Button::Left)
                {
                    navigation.clearFocus();
                    tableFocus.reset();
                    interaction.panelFocused = false;
                }
            }

            if (event->is<sf::Event::KeyPressed>())
            {
                const auto* keyEvent = event->getIf<sf::Event::KeyPressed>();

                const bool ctrl = keyEvent->control;
                const bool shift = keyEvent->shift;
                const auto key = keyEvent->code;
                // Direct keyboard access to structural buttons, including while editing.
                // TextEntered ignores Ctrl combinations, preventing duplicate insertion.
                std::string keyboardAction;
                if (ctrl && key == sf::Keyboard::Key::Slash) keyboardAction = "FRACTION";
                else if (ctrl && shift && key == sf::Keyboard::Key::Num6) keyboardAction = "ROOT";
                else if (ctrl && shift && key == sf::Keyboard::Key::Num5) {
                    expression.add("‰");
                    continue;
                }
                else if (!ctrl && shift && key == sf::Keyboard::Key::Num7) keyboardAction = "LOG";
                else if (!ctrl && key == sf::Keyboard::Key::Equal && !shift) keyboardAction = "=";
                else if (!ctrl && key == sf::Keyboard::Key::Backslash)
                    keyboardAction = shift ? "ABS_OPEN" : "ABS_CLOSE";
                if (!keyboardAction.empty()) {
                    const auto actionResult = handleArithmeticButton(keyboardAction, keyboardAction, {
                        expression, expressionPanel, mathFont, currentSystem, resultText,
                        expressionAtResult, stepHighlights, interaction, stepsMode,
                        showBlue, showGreen
                    });
                    if (actionResult == ButtonActionResult::Exit) return;
                    continue;
                }
                if (!ctrl && key==sf::Keyboard::Key::Space && interaction.editing) {
                    if (advanceActiveStructure(expression,expressionPanel,mathFont)) continue;
                }
                // F6/F7 toggle the two independent K-step overlays.
                if(key==sf::Keyboard::Key::F6 || key==sf::Keyboard::Key::F7) {
                    if(key==sf::Keyboard::Key::F6) showBlue=!showBlue;
                    else showGreen=!showGreen;
                    if(stepsMode && !resultText.empty()) {
                        const auto r=calculateSteps(expression.get(),
                            NumberSystems::base(currentSystem));
                        ArithmeticUI::makeHighlights(expression.get(),r,stepHighlights,showBlue,showGreen);
                    }
                    continue;
                }
                if (interaction.resultFocused) {
                    const std::string all=displayedExpression();
                    const std::size_t begin=ArithmeticUI::codepoints(expression.get())+1;
                    const std::size_t end=ArithmeticUI::codepoints(all);
                    if(ctrl && key==sf::Keyboard::Key::C) selectionCopy(all);
                    else if(ctrl && key==sf::Keyboard::Key::A) {interaction.resultAnchor=begin;interaction.resultCaret=end;}
                    else if(key==sf::Keyboard::Key::Escape) {interaction.resultFocused=false;interaction.panelFocused=true;}
                    else if(key==sf::Keyboard::Key::Left || key==sf::Keyboard::Key::Right ||
                        key==sf::Keyboard::Key::Up || key==sf::Keyboard::Key::Down) {
                        std::size_t next=interaction.resultCaret;
                        if(key==sf::Keyboard::Key::Left) next=next>begin?next-1:begin;
                        else if(key==sf::Keyboard::Key::Right) next=std::min(end,next+1);
                        else if(key==sf::Keyboard::Key::Up || key==sf::Keyboard::Key::Down) {
                            if(ctrl) next=expressionPanel.lineBoundary(all,interaction.resultCaret,key==sf::Keyboard::Key::Down);
                            else next=expressionPanel.verticalCursor(all,interaction.resultCaret,key==sf::Keyboard::Key::Down?1:-1);
                        }
                        if(ctrl && key==sf::Keyboard::Key::Left) next=expressionPanel.lineBoundary(all,interaction.resultCaret,false);
                        if(ctrl && key==sf::Keyboard::Key::Right) next=expressionPanel.lineBoundary(all,interaction.resultCaret,true);
                        if(next<begin && key==sf::Keyboard::Key::Up && !shift) {
                            interaction.resultFocused=false;
                            interaction.editing=true;
                            expression.setCursor(std::min(next,ArithmeticUI::codepoints(expression.get())));
                            expressionPanel.ensureCursorVisible(expression.get(),expression.cursor());
                            continue;
                        }
                        interaction.resultCaret=std::clamp(next,begin,end);
                        if(!shift) interaction.resultAnchor=interaction.resultCaret;
                        expressionPanel.ensureCursorVisible(all,interaction.resultCaret);
                    }
                    continue;
                }
                if (interaction.editing) {
                    handleExpressionKey(key, ctrl, shift, {
                        expression, expressionPanel, resultText, interaction
                    });
                    continue;
                }
                if (key == sf::Keyboard::Key::Enter &&
                    (interaction.panelFocused || (!navigation.focus() && !tableFocus))) {
                    // Wejscie do edycji z panelu lub bez aktywnego przycisku.
                    if (interaction.panelFocused) {
                        interaction.resultFocused = false;
                        interaction.panelFocused = false;
                    }
                    interaction.editing = true;
                    expression.setCursor(ArithmeticUI::codepoints(expression.get()));
                    expressionPanel.setWrapSide(false);
                    expressionPanel.ensureCursorVisible(expression.get(), expression.cursor());
                    continue;
                }
                if (keyEvent->code == sf::Keyboard::Key::Right ||
                    keyEvent->code == sf::Keyboard::Key::Left ||
                    keyEvent->code == sf::Keyboard::Key::Down ||
                    keyEvent->code == sf::Keyboard::Key::Up)
                {
                    handleButtonFocusKey(keyEvent->code, mousePos, {
                        defs, buttons, tableButtons, navigation, tableFocus, interaction
                    });
                }
            }

            for (auto& btn : buttons) btn.handleEvent(*event);
            for (auto& btn : tableButtons) btn.handleEvent(*event);

            if (const auto* entered = event->getIf<sf::Event::TextEntered>()) {
                handleExpressionText(*entered, {
                    expression, mathFont, currentSystem, interaction
                });
            }
        }

        for (std::size_t i = 0; i < buttons.size(); ++i) {
            bool enabled = NumberSystems::digitAllowed(defs[i].label, currentSystem);

            if (defs[i].action.size() == 5 &&
                defs[i].action.substr(0, 4) == "HEX_")
            {
                enabled = (currentSystem == NumberSystem::HEX);
            }

            buttons[i].setEnabled(enabled);
            buttons[i].update(mousePos);
            if (buttons[i].isActivated()) {
                const auto actionResult = handleArithmeticButton(defs[i].action, defs[i].label, {
                    expression, expressionPanel, mathFont, currentSystem, resultText,
                    expressionAtResult, stepHighlights, interaction, stepsMode,
                    showBlue, showGreen
                });
                if (actionResult == ButtonActionResult::Exit) return;
                if (actionResult == ButtonActionResult::SkipFrame) continue;
            }
        }

        updateArithmeticFrame({
            expression, expressionPanel, resultText, expressionAtResult,
            stepHighlights, interaction, stepsMode, defs, buttons,
            tableButtons, tableFocus, fractionCursorBefore, fractionBefore,
            lastVisibleCursor, lastVisibleExpression, mousePos
        });

        drawArithmeticFrame({
            window, font, title, expression, expressionPanel,
            displayedExpression(), stepHighlights, buttons, tableButtons,
            tableFocus, showBlue, showGreen, interaction
        });
    }
}