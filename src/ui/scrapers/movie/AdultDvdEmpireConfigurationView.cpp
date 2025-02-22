#include "ui/scrapers/movie/AdultDvdEmpireConfigurationView.h"

#include <QGridLayout>

namespace mediaelch {
namespace scraper {


AdultDvdEmpireConfigurationView::AdultDvdEmpireConfigurationView(AdultDvdEmpireConfiguration& settings,
    QWidget* parent) :
    QWidget(parent), m_settings(settings)
{
    m_chkBackCoverAsFanart = new QCheckBox(tr("Store back cover as fanart and poster"), this);
    m_chkBackCoverAsFanart->setObjectName("chkBackCoverAsFanart");

    auto* layout = new QGridLayout(this);
    layout->addWidget(m_chkBackCoverAsFanart, 0, 0);

    m_chkBackCoverAsFanart->setChecked(m_settings.storeBackCoverAsFanart());

    connect(m_chkBackCoverAsFanart, &QCheckBox::toggled, this, [this](bool activated) { //
        m_settings.setStoreBackCoverAsFanart(activated);
    });
}


} // namespace scraper
} // namespace mediaelch
