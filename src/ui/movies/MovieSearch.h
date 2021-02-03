#pragma once

#include "data/ImdbId.h"
#include "data/Locale.h"
#include "data/TmdbId.h"
#include "globals/ScraperInfos.h"
#include "scrapers/movie/MovieIdentifier.h"

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
    /// \brief Returns the selected locale.
    /// \note Do not call this method "locale" as it refers to QWidget's locale().
    const mediaelch::Locale& scraperLocale() const;

    QString scraperId();
    QString scraperMovieId();
    QSet<MovieScraperInfo> infosToLoad();
    QHash<mediaelch::scraper::MovieScraper*, mediaelch::scraper::MovieIdentifier> customScraperIds();

private:
    Ui::MovieSearch* ui;
};
