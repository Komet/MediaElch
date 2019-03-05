#pragma once

#include <QObject>

#include "globals/Globals.h"

/**
 * @brief The ImageProviderInterface class
 */
class TrailerProvider : public QObject
{
    Q_OBJECT

public:
    virtual QString name() = 0;

public slots:
    virtual void searchMovie(QString searchStr) = 0;
    virtual void loadMovieTrailers(QString id) = 0;

signals:
    virtual void sigSearchDone(QVector<ScraperSearchResult>) = 0;
    virtual void sigLoadDone(QVector<TrailerResult>) = 0;
};
