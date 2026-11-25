#include "ui/scrapers/concert/TmdbConcertConfigurationView.h"

#include <QGridLayout>
#include <QLabel>

namespace mediaelch {
namespace scraper {


TmdbConcertConfigurationView::TmdbConcertConfigurationView(TmdbConcertConfiguration& settings) : m_settings(settings)
{
    m_languageBox = new LanguageCombo(this);
    m_languageBox->setupLanguages(m_settings.supportedLanguages(), m_settings.language());

    auto* layout = new QGridLayout(this);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_languageBox, 0, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);


    connect(m_languageBox, &LanguageCombo::languageChanged, this, [this]() {
        m_settings.setLanguage(m_languageBox->currentLocale());
    });
    connect(&m_settings, &TmdbConcertConfiguration::languageChanged, this, [this](Locale language) {
        const bool blocked = m_languageBox->blockSignals(true); // avoid triggering save-logic or infinite loop
        m_languageBox->setLanguage(language);
        m_languageBox->blockSignals(blocked);
    });
}


} // namespace scraper
} // namespace mediaelch
