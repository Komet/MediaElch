#pragma once

#include "scrapers/music/UniversalMusicConfiguration.h"
#include "ui/small_widgets/LanguageCombo.h"

#include <QComboBox>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class UniversalMusicConfigurationView : public QWidget
{
    Q_OBJECT

public:
    explicit UniversalMusicConfigurationView(UniversalMusicConfiguration& settings);
    ~UniversalMusicConfigurationView() override = default;

private:
    void setPreferredScraper(const QString& preferred);

private:
    UniversalMusicConfiguration& m_settings;
    LanguageCombo* m_languageBox{nullptr};
    QComboBox* m_preferredBox{nullptr};
};

} // namespace scraper
} // namespace mediaelch
