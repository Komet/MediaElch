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

void removeFocusRect(QWidget* widget)
{
    for (QListWidget* list : widget->findChildren<QListWidget*>()) {
        list->setAttribute(Qt::WA_MacShowFocusRect, false);
    }
    for (QLineEdit* edit : widget->findChildren<QLineEdit*>()) {
        edit->setAttribute(Qt::WA_MacShowFocusRect, false);
    }
    for (QComboBox* box : widget->findChildren<QComboBox*>()) {
        box->setAttribute(Qt::WA_MacShowFocusRect, false);
    }
    for (QSpinBox* box : widget->findChildren<QSpinBox*>()) {
        box->setAttribute(Qt::WA_MacShowFocusRect, false);
    }
    for (QDoubleSpinBox* box : widget->findChildren<QDoubleSpinBox*>()) {
        box->setAttribute(Qt::WA_MacShowFocusRect, false);
    }
    for (QDateEdit* dateEdit : widget->findChildren<QDateEdit*>()) {
        dateEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    }
    for (QDateTimeEdit* dateTimeEdit : widget->findChildren<QDateTimeEdit*>()) {
        dateTimeEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    }
}

void applyStyle(QWidget* widget, bool removeFocus)
{
    if (removeFocus) {
        removeFocusRect(widget);
    }
}

void applyEffect(QWidget* parent)
{
    for (QPushButton* button : parent->findChildren<QPushButton*>()) {
        if (button->property("dropShadow").toBool() && button->devicePixelRatio() == 1) {
            auto* effect = new QGraphicsDropShadowEffect(parent);
            effect->setColor(QColor(0, 0, 0, 30));
            effect->setOffset(2);
            effect->setBlurRadius(4);
            button->setGraphicsEffect(effect);
        }
    }
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
