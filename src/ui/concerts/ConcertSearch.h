#pragma once

#include "data/TmdbId.h"
#include "globals/ScraperInfos.h"

#include <QDialog>

namespace Ui {
class ConcertSearch;
}

class ConcertSearch : public QDialog
{
    Q_OBJECT

public:
    explicit ConcertSearch(QWidget* parent = nullptr);
    ~ConcertSearch() override;

public slots:
    int execWithSearch(QString searchString);

public:
    int scraperNo();
    TmdbId scraperId();
    QSet<ConcertScraperInfo> infosToLoad();

private:
    Ui::ConcertSearch* ui;
};
