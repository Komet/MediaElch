#include "ui/UiUtils.h"

#include <QString>
#include <QVariant>

namespace mediaelch {
namespace ui {

void setButtonStyle(QPushButton* button, ButtonStyle style)
{
    switch (style) {
    case ButtonStyle::Danger: button->setProperty("buttonstyle", QStringLiteral("danger")); break;
    case ButtonStyle::Primary: button->setProperty("buttonstyle", QStringLiteral("primary")); break;
    case ButtonStyle::Info: button->setProperty("buttonstyle", QStringLiteral("info")); break;
    case ButtonStyle::Warning: button->setProperty("buttonstyle", QStringLiteral("warning")); break;
    case ButtonStyle::Success: button->setProperty("buttonstyle", QStringLiteral("success")); break;
    }
}

} // namespace ui
} // namespace mediaelch
