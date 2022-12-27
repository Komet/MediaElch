#include "ui/UiUtils.h"

namespace mediaelch {
namespace ui {

void setButtonStyle(QPushButton* button, ButtonStyle style)
{
    switch (style) {
    case ButtonStyle::Danger: button->setProperty("buttonstyle", "danger"); break;
    case ButtonStyle::Primary: button->setProperty("buttonstyle", "primary"); break;
    case ButtonStyle::Info: button->setProperty("buttonstyle", "info"); break;
    case ButtonStyle::Warning: button->setProperty("buttonstyle", "warning"); break;
    case ButtonStyle::Success: button->setProperty("buttonstyle", "success"); break;
    }
}

} // namespace ui
} // namespace mediaelch
