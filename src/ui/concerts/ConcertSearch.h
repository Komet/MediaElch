#pragma once

#include "data/TmdbId.h"
#include "globals/ScraperInfos.h"

#include <QDialog>

namespace Ui {
class ConcertSearch;
}

namespace mediaelch {
namespace scraper {
class ConcertScraper;
}
} // namespace mediaelch

class ConcertSearch : public QDialog
{
    Q_OBJECT

public:
    explicit ConcertSearch(QWidget* parent = nullptr);
    ~ConcertSearch() override;

public slots:
    int execWithSearch(QString searchString);

public:
    mediaelch::scraper::ConcertScraper* scraper();
    QString concertIdentifier();
    QSet<ConcertScraperInfo> infosToLoad();

private:
    Ui::ConcertSearch* ui;
};
