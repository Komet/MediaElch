#include "StyledPushButton.h"

StyledPushButton::StyledPushButton(QWidget *parent) :
    QPushButton(parent)
{
}

void StyledPushButton::setButtonStyle(StyledPushButton::ButtonStyle style)
{
    QString styleSheet;

    styleSheet.append("QPushButton {");
    styleSheet.append("border: 1px solid rgba(0, 0, 0, 50); margin-bottom: 2px;");
    styleSheet.append("padding: 2px 4px; padding-bottom: 4px;");
    styleSheet.append("border-radius: 5px;");
    styleSheet.append("}");

    if (style == StyleRed) {
        styleSheet.append("QPushButton { color: #ffffff; background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(238, 95, 91, 255), stop:1 rgba(189, 53, 47, 255)); }");
        styleSheet.append("QPushButton::pressed { background-color: rgb(189, 53, 47); }");
        styleSheet.append("QPushButton::disabled { background-color: rgb(213, 125, 120); }");
    } else if (style == StyleYellow) {
        styleSheet.append("QPushButton { color: #ffffff; background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(251, 180, 80, 255), stop:1 rgba(248, 148, 6, 255)); }");
        styleSheet.append("QPushButton::pressed { background-color: rgb(248, 148, 6); }");
        styleSheet.append("QPushButton::disabled { background-color: rgb(247, 177, 79); }");
    } else if (style == StyleGreen) {
        styleSheet.append("QPushButton { color: #ffffff; background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(98, 196, 98, 255), stop:1 rgba(81, 163, 81, 255)); }");
        styleSheet.append("QPushButton::pressed { background-color: rgb(81, 163, 81); }");
        styleSheet.append("QPushButton::disabled { background-color: rgb(142, 196, 142); }");
    } else {
        styleSheet = "";
    }
    setStyleSheet(styleSheet);
}
