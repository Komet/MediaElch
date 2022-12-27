#include "ui/UiUtils.h"

#include "globals/Helper.h"

#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QGraphicsDropShadowEffect>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>

namespace mediaelch {
namespace ui {

void fillStereoModeCombo(QComboBox* box)
{
    bool blocked = box->blockSignals(true);
    box->clear();
    box->addItem("", "");
    QMap<QString, QString> modes = helper::stereoModes();
    QMapIterator<QString, QString> it(modes);
    while (it.hasNext()) {
        it.next();
        box->addItem(it.value(), it.key());
    }
    box->blockSignals(blocked);
}

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
