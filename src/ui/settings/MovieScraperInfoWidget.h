#pragma once

#include "scrapers/movie/MovieScraper.h"

#include <QWidget>

namespace Ui {
class MovieScraperInfoWidget;
}

class MovieScraperInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MovieScraperInfoWidget(mediaelch::scraper::MovieScraper& scraper, QWidget* parent = nullptr);
    ~MovieScraperInfoWidget() override;

private:
    void setupScraperDetails();

private:
    Ui::MovieScraperInfoWidget* ui = nullptr;
    mediaelch::scraper::MovieScraper& m_scraper;
};
