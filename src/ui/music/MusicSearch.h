#pragma once

#include "globals/ScraperInfos.h"
#include "scrapers/movie/MovieScraper.h"

#include <QDialog>

namespace Ui {
class MusicSearch;
}

class MusicSearch : public QDialog
{
    Q_OBJECT

public:
    explicit MusicSearch(QWidget* parent = nullptr);
    ~MusicSearch() override;

public slots:
    int execWithSearch(QString type, QString searchString, QString artistName = QString());

    int scraperNo();
    QString scraperId();
    QString scraperId2();
    QSet<MusicScraperInfo> infosToLoad();

private:
    Ui::MusicSearch* ui;
};
