#pragma once

#include "scrapers/music/MusicScraper.h"

#include <QWidget>

namespace Ui {
class MusicScraperInfoWidget;
}

class MusicScraperInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MusicScraperInfoWidget(mediaelch::scraper::MusicScraper& scraper, QWidget* parent = nullptr);
    ~MusicScraperInfoWidget() override;

private:
    void setupScraperDetails();

private:
    Ui::MusicScraperInfoWidget* ui = nullptr;
    mediaelch::scraper::MusicScraper& m_scraper;
};
