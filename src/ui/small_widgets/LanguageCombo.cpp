#include "ui/small_widgets/LanguageCombo.h"

#include "log/Log.h"
#include "utils/Meta.h"

#include <tuple>
#include <vector>

static QString LANG_INVALID_STATE = QStringLiteral("error");

LanguageCombo::LanguageCombo(QWidget* parent) : QComboBox(parent)
{
    const auto indexChanged = elchOverload<int>(&QComboBox::currentIndexChanged);
    connect(this, indexChanged, this, &LanguageCombo::onIndexChanged);
}

void LanguageCombo::setupLanguages(const QVector<mediaelch::Locale>& locales, const mediaelch::Locale& selected)
{
    blockSignals(true);
    clear(); // Clear old entries (if there are any)

    std::vector<std::pair<QString, QString>> entries;
    entries.reserve(static_cast<std::size_t>(locales.size()));
    for (const mediaelch::Locale& lang : locales) {
        entries.emplace_back(lang.languageTranslated(), lang.toString());
    }

    // sort translated strings of languages
    std::sort(entries.begin(),
        entries.end(), //
        [](const std::pair<QString, QString>& lhs, const std::pair<QString, QString>& rhs) {
            return QString::localeAwareCompare(lhs.first, rhs.first) < 0;
        });

    for (const std::pair<QString, QString>& pair : entries) {
        addItem(pair.first, pair.second);
    }

    const int index = findData(selected.toString(), Qt::UserRole);
    if (index < 0) {
        qCWarning(generic) << "[LanguageCombo] Selected language not found in provided languages:" << selected;
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
