#ifndef MUSICSEARCH_H
#define MUSICSEARCH_H

#include <QDialog>

#include "data/ScraperInterface.h"
#include "globals/Globals.h"

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
    QList<int> infosToLoad();

private:
    Ui::MusicSearch *ui;
};

#endif // MUSICSEARCH_H
