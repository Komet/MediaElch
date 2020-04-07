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
    int exec() override;

    /// \brief Executes the search dialog
    /// \param searchString Movie name/search string
    /// \return Result of QDialog::exec
    int exec(QString searchString, ImdbId imdbId, TmdbId tmdbId);

public:
    static MovieSearch* instance(QWidget* parent = nullptr);

    /// \brief Current scraper Id
    QString scraperId();

    /// \brief Get the scraper id of the movie last clicked in result table.
    QString scraperMovieId();

    /// \brief List of infos to load from the scraper
    QVector<MovieScraperInfos> infosToLoad();

    QHash<mediaelch::scraper::MovieScraper*, QString> customScraperIds();

private:
    Ui::MovieSearch* ui;
};
