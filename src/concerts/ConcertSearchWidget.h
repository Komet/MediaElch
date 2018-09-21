#ifndef CONCERTSEARCHWIDGET_H
#define CONCERTSEARCHWIDGET_H

#include "globals/Globals.h"

#include <QTableWidgetItem>
#include <QWidget>

namespace Ui {
class ConcertSearchWidget;
}

class ConcertSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConcertSearchWidget(QWidget *parent = nullptr);
    ~ConcertSearchWidget() override;

public slots:
    void search(QString searchString);
    int scraperNo();
    QString scraperId();
    QList<ConcertScraperInfos> infosToLoad();

signals:
    void sigResultClicked();

private slots:
    void search();
    void showResults(QList<ScraperSearchResult> results);
    void resultClicked(QTableWidgetItem *item);
    void chkToggled();
    void chkAllToggled(bool toggled);

private:
    Ui::ConcertSearchWidget *ui;
    int m_scraperNo;
    QString m_scraperId;
    QList<ConcertScraperInfos> m_infosToLoad;

    void clear();
    void setChkBoxesEnabled(QList<ConcertScraperInfos> scraperSupports);
};

#endif // CONCERTSEARCHWIDGET_H
