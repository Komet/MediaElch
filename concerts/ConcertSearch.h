#ifndef CONCERTSEARCH_H
#define CONCERTSEARCH_H

#include <QDialog>

#include "globals/Globals.h"

namespace Ui {
class ConcertSearch;
}

/**
 * @brief The ConcertSearch class
 */
class ConcertSearch : public QDialog
{
    Q_OBJECT
public:
    explicit ConcertSearch(QWidget *parent = 0);
    ~ConcertSearch();

public slots:
    int exec(QString searchString);
    static ConcertSearch *instance(QWidget *parent = 0);
    int scraperNo();
    QString scraperId();
    QList<int> infosToLoad();

private:
    Ui::ConcertSearch *ui;
};

#endif // CONCERTSEARCH_H
