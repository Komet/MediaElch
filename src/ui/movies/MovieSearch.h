#pragma once

#include "data/ImdbId.h"
#include "data/TmdbId.h"
#include "globals/ScraperInfos.h"

#include <QDialog>
#include <QMap>
#include <QString>
#include <QTableWidgetItem>
#include <QVector>

namespace Ui {
class MovieSearch;
}

namespace mediaelch {
namespace scraper {
class MovieScraper;
}
} // namespace mediaelch

class MovieSearch : public QDialog
{
    Q_OBJECT
public:
    explicit MovieSearch(QWidget* parent = nullptr);
    ~MovieSearch() override;

public slots:
    int execWithSearch(QString searchString, ImdbId id, TmdbId tmdbId);

public:
    QString scraperId();
    QString scraperMovieId();
    QSet<MovieScraperInfo> infosToLoad();
    QHash<mediaelch::scraper::MovieScraper*, QString> customScraperIds();

private:
    Ui::MovieSearch* ui;
};
