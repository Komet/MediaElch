#pragma once

#include "globals/Globals.h"
#include "globals/ScraperResult.h"

#include <QObject>

namespace mediaelch {
namespace scraper {

class TrailerProvider : public QObject
{
    Q_OBJECT

public:
    virtual QString name() = 0;

public slots:
    virtual void searchMovie(QString searchStr) = 0;
    virtual void loadMovieTrailers(QString id) = 0;

signals:
    void sigSearchDone(QVector<ScraperSearchResult>);
    void sigLoadDone(QVector<TrailerResult>);
};

} // namespace scraper
} // namespace mediaelch
