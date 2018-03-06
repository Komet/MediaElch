#ifndef MOVIESEARCH_H
#define MOVIESEARCH_H

#include <QDialog>
#include <QTableWidgetItem>

#include "data/ScraperInterface.h"
#include "globals/Globals.h"

namespace Ui {
class MovieSearch;
}

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
    QList<int> infosToLoad();
    QMap<ScraperInterface *, QString> customScraperIds();

private:
    Ui::MovieSearch *ui;
};

#endif // MOVIESEARCH_H
