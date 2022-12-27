#pragma once

// Utils for MediaElch's UI.
// TODO: We should try to move these functions to better places.
//       For example create a custom combobox instead of fillStereoModeCombo().

#include <QComboBox>
#include <QPushButton>
#include <QWidget>

namespace mediaelch {
namespace ui {

enum ButtonStyle
{
    ButtonPrimary,
    ButtonInfo,
    ButtonDanger,
    ButtonSuccess,
    ButtonWarning
};

void fillStereoModeCombo(QComboBox* box);

void removeFocusRect(QWidget* widget);

void applyStyle(QWidget* widget, bool removeFocus = true);
void applyEffect(QWidget* parent);

void setButtonStyle(QPushButton* button, ButtonStyle style);

} // namespace ui
} // namespace mediaelch
