#include "ui/UiUtils.h"

namespace mediaelch {
namespace ui {

void setButtonStyle(QPushButton* button, ButtonStyle style)
{
    switch (style) {
    case ButtonStyle::ButtonDanger: button->setProperty("buttonstyle", "danger"); break;
    case ButtonStyle::ButtonPrimary: button->setProperty("buttonstyle", "primary"); break;
    case ButtonStyle::ButtonInfo: button->setProperty("buttonstyle", "info"); break;
    case ButtonStyle::ButtonWarning: button->setProperty("buttonstyle", "warning"); break;
    case ButtonStyle::ButtonSuccess: button->setProperty("buttonstyle", "success"); break;
    }
}

} // namespace ui
} // namespace mediaelch
