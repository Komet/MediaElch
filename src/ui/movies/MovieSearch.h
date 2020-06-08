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

class MovieScraperInterface;

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
    QHash<MovieScraperInterface*, QString> customScraperIds();

private:
    Ui::MovieSearch* ui;
};
