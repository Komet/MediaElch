#include "ui/scrapers/movie/ImdbMovieConfigurationView.h"
#include <QGridLayout>
#include <QLabel>

namespace mediaelch {
namespace scraper {

ImdbMovieConfigurationView::ImdbMovieConfigurationView(ImdbMovieConfiguration& settings, QWidget* parent) :
    QWidget(parent), m_settings(settings)
{
    m_languageBox = new LanguageCombo(this);
    m_languageBox->setupLanguages(m_settings.supportedLanguages(), m_settings.language());

    m_chkAllTags = new QCheckBox(tr("Load all tags"), this);
    m_chkAllTags->setObjectName("chkAllTags");

    auto* layout = new QGridLayout(this);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_languageBox, 0, 1);
    layout->addWidget(m_chkAllTags, 1, 0, 1, 2);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);

    m_chkAllTags->setChecked(m_settings.shouldLoadAllTags());

    connect(m_languageBox, &LanguageCombo::languageChanged, this, [this]() {
        m_settings.setLanguage(m_languageBox->currentLocale());
    });
    connect(&m_settings, &ImdbMovieConfiguration::languageChanged, this, [this](Locale language) {
        const bool blocked = m_languageBox->blockSignals(true);
        m_languageBox->setLanguage(language);
        m_languageBox->blockSignals(blocked);
    });
    connect(m_chkAllTags, &QCheckBox::toggled, this, [this](bool activated) { //
        m_settings.setLoadAllTags(activated);
    });
}

} // namespace scraper
} // namespace mediaelch
