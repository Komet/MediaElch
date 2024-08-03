#pragma once

#include "scrapers/movie/aebn/AebnConfiguration.h"
#include "ui/small_widgets/LanguageCombo.h"

#include <QComboBox>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class AebnConfigurationView : public QWidget
{
    Q_OBJECT

public:
    explicit AebnConfigurationView(AebnConfiguration& settings, QWidget* parent = nullptr);
    ~AebnConfigurationView() override = default;

private:
    void setGenreId(const QString& genreId);

private:
    AebnConfiguration& m_settings;

    LanguageCombo* m_languageBox{nullptr};
    QComboBox* m_genreBox{nullptr};
};

} // namespace scraper
} // namespace mediaelch
