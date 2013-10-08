#ifndef STYLEDPUSHBUTTON_H
#define STYLEDPUSHBUTTON_H

#include <QPushButton>

class StyledPushButton : public QPushButton
{
    Q_OBJECT
public:
    enum ButtonStyle {
        StyleBlue, StyleLightBlue, StyleRed, StyleGreen, StyleYellow, Unstyled
    };

    explicit StyledPushButton(QWidget *parent = 0);
    void setButtonStyle(StyledPushButton::ButtonStyle style);
};

#endif // STYLEDPUSHBUTTON_H
