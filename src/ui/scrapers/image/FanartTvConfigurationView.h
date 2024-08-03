#pragma once

#include "scrapers/image/FanartTvConfiguration.h"
#include "ui/small_widgets/LanguageCombo.h"

#include <QComboBox>
#include <QLineEdit>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class FanartTvConfigurationView : public QWidget
{
    Q_OBJECT

public:
    explicit FanartTvConfigurationView(FanartTvConfiguration& settings);
    ~FanartTvConfigurationView() override = default;

private:
    void setPreferredDiscType(const QString& discType);

private:
    FanartTvConfiguration& m_settings;
    LanguageCombo* m_languageBox{nullptr};
    QComboBox* m_discBox{nullptr};
    QLineEdit* m_personalApiKeyEdit{nullptr};
};

} // namespace scraper
} // namespace mediaelch
