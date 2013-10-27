#include "StyledPushButton.h"

StyledPushButton::StyledPushButton(QWidget *parent) :
    QPushButton(parent)
{
}

void StyledPushButton::setButtonStyle(StyledPushButton::ButtonStyle style)
{
    QString styleSheet;

    styleSheet.append("QPushButton {");
    styleSheet.append("padding: 4px 4px; padding-bottom: 4px;");
    styleSheet.append("margin: 4px;");
    styleSheet.append("border-radius: 4px;");
    styleSheet.append("font-size: 11px;");
    styleSheet.append("}");

    if (style == StyleRed) {
        styleSheet.append("QPushButton { color: #ffffff; background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(238, 95, 91, 255), stop:1 rgba(189, 53, 47, 255)); border: 1px solid #C12E2A;}");
        styleSheet.append("QPushButton::pressed { background-color: rgb(189, 53, 47); }");
        styleSheet.append("QPushButton::disabled { background-color: rgb(213, 125, 120); }");
        styleSheet.append("margin-bottom: 2px;");
    } else if (style == StyleBlue) {
        styleSheet.append("QPushButton { color: #ffffff; background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(66, 139, 202, 255), stop:1 rgba(48, 113, 169, 255)); border: 1px solid #2D6CA2; }");
        styleSheet.append("QPushButton::pressed { background-color: rgb(48, 113, 169); }");
        styleSheet.append("QPushButton::disabled { background-color: rgb(66, 139, 202); }");
        styleSheet.append("margin-bottom: 2px;");
    } else if (style == StyleLightBlue) {
        styleSheet.append("QPushButton { color: #ffffff; background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #5BC0DE, stop:1 #31B0D5); border: 1px solid #2AABD2; }");
        styleSheet.append("QPushButton::pressed { background-color: #31B0D5; }");
        styleSheet.append("QPushButton::disabled { background-color: #79cce4; }");
        styleSheet.append("margin-bottom: 2px;");
    } else if (style == StyleYellow) {
        styleSheet.append("QPushButton { color: #ffffff; background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(251, 180, 80, 255), stop:1 rgba(248, 148, 6, 255)); border: 1px solid #EB9316; }");
        styleSheet.append("QPushButton::pressed { background-color: rgb(248, 148, 6); }");
        styleSheet.append("QPushButton::disabled { background-color: rgb(247, 177, 79); }");
        styleSheet.append("margin-bottom: 2px;");
    } else if (style == StyleGreen) {
        styleSheet.append("QPushButton { color: #ffffff; background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(98, 196, 98, 255), stop:1 rgba(81, 163, 81, 255)); border: 1px solid #419641; }");
        styleSheet.append("QPushButton::pressed { background-color: rgb(81, 163, 81); }");
        styleSheet.append("QPushButton::disabled { background-color: rgb(142, 196, 142); }");
        styleSheet.append("margin-bottom: 2px;");
    } else {
        styleSheet = "";
    }
    setStyleSheet(styleSheet);
}
