#include "ui/scrapers/image/FanartTvConfigurationView.h"

#include <QGridLayout>
#include <QLabel>

namespace mediaelch {
namespace scraper {


FanartTvConfigurationView::FanartTvConfigurationView(FanartTvConfiguration& settings) : m_settings(settings)
{
    m_languageBox = new LanguageCombo(this);
    m_languageBox->setupLanguages(m_settings.supportedLanguages(), m_settings.language());

    m_discBox = new QComboBox(this);
    m_discBox->addItem("3D", "3D");
    m_discBox->addItem(tr("Blu-ray"), "BluRay");
    m_discBox->addItem(tr("DVD"), "DVD");
    setPreferredDiscType(m_settings.preferredDiscType());

    m_personalApiKeyEdit = new QLineEdit(this);

    auto* layout = new QGridLayout(this);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_languageBox, 0, 1);
    layout->addWidget(new QLabel(tr("Preferred Disc Type")), 1, 0);
    layout->addWidget(m_discBox, 1, 1);
    layout->addWidget(new QLabel(tr("Personal API key")), 2, 0);
    layout->addWidget(m_personalApiKeyEdit, 2, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);


    connect(m_languageBox, &LanguageCombo::languageChanged, this, [this]() {
        m_settings.setLanguage(m_languageBox->currentLocale());
    });
    connect(&m_settings, &FanartTvConfiguration::languageChanged, this, [this](Locale language) {
        const bool blocked = m_languageBox->blockSignals(true); // avoid triggering save-logic or infinite loop
        m_languageBox->setLanguage(language);
        m_languageBox->blockSignals(blocked);
    });

    connect(m_discBox, &QComboBox::activated, this, [this]() {
        QString discType = m_discBox->itemData(m_discBox->currentIndex()).toString();
        m_settings.setPreferredDiscType(discType);
    });
    connect(&m_settings, &FanartTvConfiguration::preferredDiscTypeChanged, this, [this](QString discType) {
        const bool blocked = m_discBox->blockSignals(true); // avoid triggering save-logic or infinite loop
        setPreferredDiscType(discType);
        m_discBox->blockSignals(blocked);
    });

    connect(m_personalApiKeyEdit, &QLineEdit::textChanged, this, [this](QString text) {
        m_settings.setPersonalApiKey(text);
    });
    connect(&m_settings, &FanartTvConfiguration::personalApiKeyChanged, this, [this](QString apiKey) {
        const bool blocked = m_personalApiKeyEdit->blockSignals(true); // avoid triggering save-logic
        m_personalApiKeyEdit->setText(apiKey);
        m_personalApiKeyEdit->blockSignals(blocked);
    });
}

void FanartTvConfigurationView::setPreferredDiscType(const QString& discType)
{
    for (int i = 0, n = m_discBox->count(); i < n; ++i) {
        if (m_discBox->itemData(i).toString() == discType) {
            m_discBox->setCurrentIndex(i);
        }
    }
}


} // namespace scraper
} // namespace mediaelch
