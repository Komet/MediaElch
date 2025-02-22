#pragma once

#include "scrapers/movie/adultdvdempire/AdultDvdEmpireConfiguration.h"

#include <QCheckBox>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class AdultDvdEmpireConfigurationView : public QWidget
{
    Q_OBJECT

public:
    explicit AdultDvdEmpireConfigurationView(AdultDvdEmpireConfiguration& settings, QWidget* parent = nullptr);
    ~AdultDvdEmpireConfigurationView() override = default;

private:
    AdultDvdEmpireConfiguration& m_settings;

    QCheckBox* m_chkBackCoverAsFanart = nullptr;
};

} // namespace scraper
} // namespace mediaelch
