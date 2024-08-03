#include "ui/scrapers/movie/AebnConfigurationView.h"
#include <QGridLayout>
#include <QLabel>

namespace mediaelch {
namespace scraper {


AebnConfigurationView::AebnConfigurationView(AebnConfiguration& settings, QWidget* parent) :
    QWidget(parent), m_settings(settings)
{
    m_languageBox = new LanguageCombo(this);
    m_languageBox->setupLanguages(m_settings.supportedLanguages(), m_settings.language());
    m_genreBox = new QComboBox(this);


    // Genre IDs overrides URL (http://[straight|gay]...)
    m_genreBox->addItem(QObject::tr("Straight"), "101");
    m_genreBox->addItem(QObject::tr("Gay"), "102");

    // set before all event handlers are set up
    setGenreId(m_settings.genreId());

    auto* layout = new QGridLayout(this);
    layout->addWidget(new QLabel(QObject::tr("Language")), 0, 0);
    layout->addWidget(m_languageBox, 0, 1);
    layout->addWidget(new QLabel(QObject::tr("Genre")), 1, 0);
    layout->addWidget(m_genreBox, 1, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);


    connect(m_languageBox, &LanguageCombo::languageChanged, this, [this]() {
        m_settings.setLanguage(m_languageBox->currentLocale());
    });
    connect(&m_settings, &AebnConfiguration::languageChanged, this, [this](Locale language) {
        const bool blocked = m_languageBox->blockSignals(true); // avoid triggering save-logic or infinite loop
        m_languageBox->setLanguage(language);
        m_languageBox->blockSignals(blocked);
    });


    connect(m_genreBox, &QComboBox::activated, this, [this]() {
        QString genreId = m_genreBox->itemData(m_genreBox->currentIndex()).toString();
        m_settings.setGenreId(genreId);
    });
    connect(&m_settings, &AebnConfiguration::genreIdChanged, this, [this](QString genreId) {
        const bool blocked = m_genreBox->blockSignals(true); // avoid triggering save-logic
        setGenreId(genreId);
        m_genreBox->blockSignals(blocked);
    });
}

void AebnConfigurationView::setGenreId(const QString& genreId)
{
    for (int i = 0, n = m_genreBox->count(); i < n; ++i) {
        if (m_genreBox->itemData(i).toString() == genreId) {
            m_genreBox->setCurrentIndex(i);
        }
    }
}

} // namespace scraper
} // namespace mediaelch
