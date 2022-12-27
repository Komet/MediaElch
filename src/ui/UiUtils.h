#pragma once

#include <QPushButton>

namespace mediaelch {
namespace ui {

enum class ButtonStyle
{
    Primary,
    Info,
    Danger,
    Success,
    Warning
};

void setButtonStyle(QPushButton* button, ButtonStyle style);

} // namespace ui
} // namespace mediaelch
