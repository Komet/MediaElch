#ifndef MUSICSEARCHWIDGET_H
#define MUSICSEARCHWIDGET_H

#include <QSignalMapper>
#include <QTableWidgetItem>
#include <QWidget>

#include "data/ScraperInterface.h"
#include "globals/Globals.h"

namespace Ui {
class MusicSearchWidget;
}

class MusicSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MusicSearchWidget(QWidget *parent = nullptr);
    ~MusicSearchWidget() override;

public slots:
    void search(QString searchString);
    int scraperNo();
    QString scraperId();
    QString scraperId2();
    QList<MusicScraperInfos> infosToLoad();
    void setType(const QString &type);
    void setArtistName(const QString &artistName);

signals:
    void sigResultClicked();

private slots:
    void search();
    void showResults(QList<ScraperSearchResult> results);
    void resultClicked(QTableWidgetItem *item);
    void resultClicked(int row);
    void chkToggled();
    void chkAllToggled(bool toggled);

private:
    Ui::MusicSearchWidget *ui;
    int m_scraperNo;
    QString m_scraperId;
    QString m_scraperId2;
    QList<MusicScraperInfos> m_infosToLoad;
    QString m_type;
    QString m_artistName;
    QSignalMapper *m_signalMapper;

    void clear();
    void setChkBoxesEnabled(QList<MusicScraperInfos> scraperSupports);
};

#endif // MUSICSEARCHWIDGET_H
