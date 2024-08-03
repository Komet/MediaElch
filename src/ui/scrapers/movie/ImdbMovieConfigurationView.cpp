#include "ui/scrapers/movie/ImdbMovieConfigurationView.h"
#include <QGridLayout>
#include <QLabel>

namespace mediaelch {
namespace scraper {

ImdbMovieConfigurationView::ImdbMovieConfigurationView(ImdbMovieConfiguration& settings, QWidget* parent) :
    QWidget(parent), m_settings(settings)
{
    m_chkAllTags = new QCheckBox(tr("Load all tags"), this);
    m_chkAllTags->setObjectName("chkAllTags");

    auto* layout = new QGridLayout(this);
    layout->addWidget(m_chkAllTags, 0, 0);

    m_chkAllTags->setChecked(m_settings.shouldLoadAllTags());

    connect(m_chkAllTags, &QCheckBox::toggled, this, [this](bool activated) { //
        m_settings.setLoadAllTags(activated);
    });
}

} // namespace scraper
} // namespace mediaelch
