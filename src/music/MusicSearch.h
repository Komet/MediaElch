#pragma once

#include "data/MovieScraperInterface.h"
#include "globals/Globals.h"

#include <QDialog>

namespace Ui {
class MusicSearch;
}

class MusicSearch : public QDialog
{
    Q_OBJECT

public:
    explicit MusicSearch(QWidget *parent = nullptr);
    ~MusicSearch() override;

public slots:
    int exec() override;
    int exec(QString type, QString searchString, QString artistName = QString());
    static MusicSearch *instance(QWidget *parent = nullptr);
    int scraperNo();
    QString scraperId();
    QString scraperId2();
    QList<MusicScraperInfos> infosToLoad();

private:
    Ui::MusicSearch *ui;
};
