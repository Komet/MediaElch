#ifndef MOVIESEARCH_H
#define MOVIESEARCH_H

#include "data/ImdbId.h"
#include "data/TmdbId.h"
#include "globals/Globals.h"

#include <QDialog>
#include <QList>
#include <QMap>
#include <QString>
#include <QTableWidgetItem>

namespace Ui {
class MovieSearch;
}

class MovieScraperInterface;

/**
 * @brief The MovieSearch class
 */
class MovieSearch : public QDialog
{
    Q_OBJECT
public:
    explicit MovieSearch(QWidget *parent = nullptr);
    ~MovieSearch() override;

public slots:
    int exec() override;
    int exec(QString searchString, ImdbId id, TmdbId tmdbId);
    static MovieSearch *instance(QWidget *parent = nullptr);
    QString scraperId();
    QString scraperMovieId();
    QList<MovieScraperInfos> infosToLoad();
    QMap<MovieScraperInterface *, QString> customScraperIds();

private:
    Ui::MovieSearch *ui;
};

#endif // MOVIESEARCH_H
