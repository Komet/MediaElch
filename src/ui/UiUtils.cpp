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

void applyStyle(QWidget* widget, bool removeFocus, bool /*isTable*/)
{
    if (removeFocus) {
        removeFocusRect(widget);
    }

    QStringList styleSheet = QStringList()
                             << "QLabel {"
                             << "    color: #666666;"
#ifndef Q_OS_WIN
                             << "    font-family: \"Helvetica Neue\";"
#    ifndef Q_OS_MACX
                             << "    font-size: 12px;"
#    endif
#endif
                             << "}"

                             << "QLineEdit, QSpinBox, QDateTimeEdit, QTextEdit, QComboBox, QDoubleSpinBox, QCheckBox {"
                             << "    border: 0;"
                             << "    border-bottom: 1px dotted #e0e0e0;"
                             << "}"

                             << "QComboBox::down-arrow {"
                             << "    image: url(':/img/ui_select.png');"
                             << "    width: 16px;"
                             << "    height: 16px;"
                             << "}";

    styleSheet
        << "QCheckBox::indicator:unchecked {"
        << "    image: url(':/img/ui_uncheck.png');"
        << "    width: 16px;"
        << "    height: 16px;"
        << "}"

        << "QCheckBox::indicator:checked {"
        << "    image: url(':/img/ui_check.png');"
        << "    width: 16px;"
        << "    height: 16px;"
        << "}"

        << "QComboBox::drop-down {"
        << "    background-color: #ffffff;"
        << "}"

        << "QTabWidget::pane {"
        << "    border-top: 1px solid #dddddd;"
        << "    margin-top: -1px;"
        << "}"

        << "QTabBar::tab {"
        << "    padding: 8px;"
        << "    color: #777777;"
        << "    border: 0;"
        << "}"

        << "QTabBar::tab:selected {"
        << "    border: 1px solid #dddddd;"
        << "    border-bottom: 1px solid #ffffff;"
        << "    border-top: 2px solid #43a9e4;"
        << "    border-top-left-radius: 4px;"
        << "    border-top-right-radius: 4px;"
        << "}"

        << "QTabBar::tab:first:!selected {"
        << "    border-left: none;"
        << "}"

        << "QTableWidget {"
        << "    border: none;"
        << "    selection-background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #4185b6, stop:1 "
           "#1b6aa5);"
        << "    alternate-background-color: #f9f9f9;"
        << "    selection-color: #ffffff;"
        << "}"

        << "QTableWidget QHeaderView::section {"
        << "    background-color: #f9f9f9;"
        << "    color: rgb(27, 105, 165);"
        << "    border: none;"
        << "    border-left: 1px solid #f0f0f0;"
        << "    font-weight: normal;"
        << "    padding-top: 4px;"
        << "    padding-bottom: 4px;"
        << "    margin-top: 1px;"
        << "    margin-bottom: 1px;"
        << "}"

        << "QTableWidget QHeaderView::section:first {"
        << "    border: none;"
        << "}"

        << "QPushButton {"
        << "    background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #428BCA, stop:1 #3071A9);"
        << "    color: #ffffff;"
        << "    border: 1px solid #2D6CA2;"
        << "    border-radius: 4px;"
        << "    padding: 4px;"
#if defined(Q_OS_MAC)
        << "    font-size: 11px;"
#endif
        << "}"

        << "QPushButton::pressed {"
        << "    background-color: #3071A9;"
        << "}"

        << "QPushButton::disabled {"
        << "    background-color: #83b1d9;"
        << "    border: 1px solid #7ca7cb;"
        << "}"

        << ";";

    for (QTabWidget* tabWidget : widget->findChildren<QTabWidget*>()) {
        QFont font = tabWidget->font();
        font.setFamily("Helvetica Neue");
#ifdef Q_OS_MAC
        font.setPointSize(13);
#else
        font.setPixelSize(12);
#endif
        font.setWeight(QFont::DemiBold);
        tabWidget->setFont(font);
    }

    widget->setStyleSheet(widget->styleSheet() + styleSheet.join("\n"));

    for (QPushButton* button : widget->findChildren<QPushButton*>()) {
        QString styleType = button->property("styleType").toString();
        if (styleType.isEmpty()) {
            continue;
        }

        if (styleType == "danger") {
            helper::setButtonStyle(button, helper::ButtonDanger);
        } else if (styleType == "info") {
            helper::setButtonStyle(button, helper::ButtonInfo);
        } else if (styleType == "primary") {
            helper::setButtonStyle(button, helper::ButtonPrimary);
        } else if (styleType == "success") {
            helper::setButtonStyle(button, helper::ButtonSuccess);
        } else if (styleType == "warning") {
            helper::setButtonStyle(button, helper::ButtonWarning);
        }
    }
}

void applyEffect(QWidget* parent)
{
    for (QPushButton* button : parent->findChildren<QPushButton*>()) {
        if (button->property("dropShadow").toBool() && helper::devicePixelRatio(button) == 1) {
            auto* effect = new QGraphicsDropShadowEffect(parent);
            effect->setColor(QColor(0, 0, 0, 30));
            effect->setOffset(2);
            effect->setBlurRadius(4);
            button->setGraphicsEffect(effect);
        }
    }
}


} // namespace ui
} // namespace mediaelch
