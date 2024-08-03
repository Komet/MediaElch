#pragma once

#include "scrapers/movie/imdb/ImdbMovieConfiguration.h"

#include <QCheckBox>
#include <QPointer>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class ImdbMovieConfigurationView : public QWidget
{
    Q_OBJECT

public:
    explicit ImdbMovieConfigurationView(ImdbMovieConfiguration& settings, QWidget* parent = nullptr);
    ~ImdbMovieConfigurationView() override = default;

private:
    ImdbMovieConfiguration& m_settings;

    QCheckBox* m_chkAllTags = nullptr;
};

} // namespace scraper
} // namespace mediaelch
