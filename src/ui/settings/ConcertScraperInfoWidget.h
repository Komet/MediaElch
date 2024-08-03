#pragma once

#include "scrapers/concert/ConcertScraper.h"

#include <QWidget>

namespace Ui {
class ConcertScraperInfoWidget;
}

class ConcertScraperInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConcertScraperInfoWidget(mediaelch::scraper::ConcertScraper& scraper, QWidget* parent = nullptr);
    ~ConcertScraperInfoWidget() override;

private:
    void setupScraperDetails();

private:
    Ui::ConcertScraperInfoWidget* ui = nullptr;
    mediaelch::scraper::ConcertScraper& m_scraper;
};
