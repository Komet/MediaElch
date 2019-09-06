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
    int exec() override;
    int exec(QString searchString);
    static ConcertSearch* instance(QWidget* parent = nullptr);
    int scraperNo();
    TmdbId scraperId();
    QVector<ConcertScraperInfos> infosToLoad();

private:
    Ui::ConcertSearch* ui;
};
