#ifndef CONCERTSEARCHWIDGET_H
#define CONCERTSEARCHWIDGET_H

#include <QTableWidgetItem>
#include <QWidget>

#include "globals/Globals.h"

namespace Ui {
class ConcertSearchWidget;
}

class ConcertSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConcertSearchWidget(QWidget *parent = 0);
    ~ConcertSearchWidget();

public slots:
    void search(QString searchString);
    int scraperNo();
    QString scraperId();
    QList<int> infosToLoad();

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
    QList<int> m_infosToLoad;

    void clear();
    void setChkBoxesEnabled(QList<int> scraperSupports);
};

#endif // CONCERTSEARCHWIDGET_H
