#ifndef MOVIESEARCH_H
#define MOVIESEARCH_H

#include "globals/Globals.h"

#include <QDialog>
#include <QList>
#include <QMap>
#include <QString>
#include <QTableWidgetItem>

namespace Ui {
class MovieSearch;
}

class ScraperInterface;

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
    int exec(QString searchString, QString id, QString tmdbId);
    static MovieSearch *instance(QWidget *parent = nullptr);
    QString scraperId();
    QString scraperMovieId();
    QList<MovieScraperInfos> infosToLoad();
    QMap<ScraperInterface *, QString> customScraperIds();

private:
    Ui::MovieSearch *ui;
};

#endif // MOVIESEARCH_H
