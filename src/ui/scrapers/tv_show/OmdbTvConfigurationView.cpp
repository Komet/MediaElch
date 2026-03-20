#include "ui/scrapers/tv_show/OmdbTvConfigurationView.h"

#include <QGridLayout>
#include <QLabel>

namespace mediaelch {
namespace scraper {

OmdbTvConfigurationView::OmdbTvConfigurationView(OmdbTvConfiguration& settings) : m_settings(settings)
{
    m_apiKeyEdit = new QLineEdit(this);
    m_apiKeyEdit->setText(m_settings.apiKey());

    auto* layout = new QGridLayout(this);
    layout->addWidget(new QLabel(tr("API key")), 0, 0);
    layout->addWidget(m_apiKeyEdit, 0, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);

    connect(m_apiKeyEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_settings.setApiKey(text);
    });
    connect(&m_settings, &OmdbTvConfiguration::apiKeyChanged, this, [this](const QString& apiKey) {
        const bool blocked = m_apiKeyEdit->blockSignals(true);
        m_apiKeyEdit->setText(apiKey);
        m_apiKeyEdit->blockSignals(blocked);
    });
}

} // namespace scraper
} // namespace mediaelch
