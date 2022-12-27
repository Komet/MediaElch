#pragma once

#include <QPushButton>

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

void setButtonStyle(QPushButton* button, ButtonStyle style);

} // namespace ui
} // namespace mediaelch
