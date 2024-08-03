#include "ui/scrapers/music/UniversalMusicConfigurationView.h"

#include <QGridLayout>
#include <QLabel>

namespace mediaelch {
namespace scraper {


UniversalMusicConfigurationView::UniversalMusicConfigurationView(UniversalMusicConfiguration& settings) :
    m_settings(settings)
{
    m_languageBox = new LanguageCombo(this);
    m_languageBox->setupLanguages(m_settings.supportedLanguages(), m_settings.language());

    m_preferredBox = new QComboBox(this);
    m_preferredBox->addItem(tr("The Audio DB"), "theaudiodb");
    m_preferredBox->addItem(tr("MusicBrainz"), "musicbrainz");
    m_preferredBox->addItem(tr("AllMusic"), "allmusic");
    m_preferredBox->addItem(tr("Discogs"), "discogs");
    setPreferredScraper(m_settings.preferredScraper());

    auto* layout = new QGridLayout(this);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_languageBox, 0, 1);
    layout->addWidget(new QLabel(tr("Prefer")), 1, 0);
    layout->addWidget(m_preferredBox, 1, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);

    connect(m_languageBox, &LanguageCombo::languageChanged, this, [this]() {
        m_settings.setLanguage(m_languageBox->currentLocale());
    });
    connect(&m_settings, &UniversalMusicConfiguration::languageChanged, this, [this](Locale language) {
        const bool blocked = m_languageBox->blockSignals(true); // avoid triggering save-logic or infinite loop
        m_languageBox->setLanguage(language);
        m_languageBox->blockSignals(blocked);
    });


    connect(m_preferredBox, &QComboBox::activated, this, [this]() {
        QString language = m_preferredBox->itemData(m_preferredBox->currentIndex()).toString();
        m_settings.setPreferredScraper(language);
    });
    connect(&m_settings, &UniversalMusicConfiguration::preferredScraperChanged, this, [this](QString preferredScraper) {
        const bool blocked = m_preferredBox->blockSignals(true); // avoid triggering save-logic or infinite loop
        setPreferredScraper(preferredScraper);
        m_preferredBox->blockSignals(blocked);
    });
}

void UniversalMusicConfigurationView::setPreferredScraper(const QString& preferred)
{
    for (int i = 0, n = m_preferredBox->count(); i < n; ++i) {
        if (m_preferredBox->itemData(i).toString() == preferred) {
            m_preferredBox->setCurrentIndex(i);
        }
    }
}


} // namespace scraper
} // namespace mediaelch
