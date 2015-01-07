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
    explicit MovieSearch(QWidget *parent = 0);
    ~MovieSearch();

public slots:
    int exec();
    int exec(QString searchString, QString id, QString tmdbId);

    //! \brief Reimplemented from QDialog. Saves persitent user changes.
    void accept();

    //! \brief Reimplemented from QDialog. Saves persitent user changes.
    void reject();

    QString scraperId();
    QString scraperMovieId();
    QList<int> infosToLoad();
    QMap<ScraperInterface*, QString> customScraperIds();

private:
    Ui::MovieSearch *ui;
};

#endif // MOVIESEARCH_H
