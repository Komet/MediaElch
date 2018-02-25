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
    explicit ConcertSearch(QWidget *parent = nullptr);
    ~ConcertSearch();

public slots:
    int exec();
    int exec(QString searchString);
    static ConcertSearch *instance(QWidget *parent = nullptr);
    int scraperNo();
    QString scraperId();
    QList<int> infosToLoad();

private:
    Ui::ConcertSearch *ui;
};

#endif // CONCERTSEARCH_H
