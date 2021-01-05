#include "ui/small_widgets/LanguageCombo.h"

#include "globals/Meta.h"

#include <QDebug>

static QString LANG_INVALID_STATE = QStringLiteral("error");

LanguageCombo::LanguageCombo(QWidget* parent) : QComboBox(parent)
{
    const auto indexChanged = elchOverload<int>(&QComboBox::currentIndexChanged);
    connect(this, indexChanged, this, &LanguageCombo::onIndexChanged);
}

void LanguageCombo::setupLanguages(const QVector<mediaelch::Locale>& locales, const mediaelch::Locale& selected)
{
    blockSignals(true);
    // Clear old entries (if there are any)
    clear();
    for (const mediaelch::Locale& lang : locales) {
        addItem(lang.languageTranslated(), lang.toString());
    }
    const int index = findData(selected.toString(), Qt::UserRole);
    if (index < 0) {
        qWarning() << "[LanguageCombo] Selected language not found in provided languages:" << selected;
    }
    setCurrentIndex(index > 0 ? index : 0);
    // If only one language is available, "make it grey"
    setEnabled(locales.size() > 1);
    blockSignals(false);
}

void LanguageCombo::setInvalid()
{
    blockSignals(true);
    clear();
    addItem(tr("No language available"), LANG_INVALID_STATE);
    blockSignals(false);
}

bool LanguageCombo::isCurrentValid()
{
    return itemData(currentIndex(), Qt::UserRole) != LANG_INVALID_STATE;
}

mediaelch::Locale LanguageCombo::currentLocale()
{
    return mediaelch::Locale(itemData(currentIndex(), Qt::UserRole).toString());
}

mediaelch::Locale LanguageCombo::localeAt(int index)
{
    return mediaelch::Locale(itemData(index, Qt::UserRole).toString());
}

void LanguageCombo::onIndexChanged(int index)
{
    Q_UNUSED(index)
    emit languageChanged();
}
