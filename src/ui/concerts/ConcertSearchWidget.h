#pragma once

#include "data/TmdbId.h"
#include "globals/Globals.h"
#include "globals/ScraperInfos.h"
#include "globals/ScraperResult.h"

#include <QTableWidgetItem>
#include <QWidget>

namespace Ui {
class ConcertSearchWidget;
}

class ConcertSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConcertSearchWidget(QWidget* parent = nullptr);
    ~ConcertSearchWidget() override;

public slots:
    void search(QString searchString);
    int scraperNo();
    TmdbId scraperId();
    QSet<ConcertScraperInfos> infosToLoad();

signals:
    void sigResultClicked();

private slots:
    void searchByComboIndex(int comboScraperIndex);
    void showResults(QVector<ScraperSearchResult> results);
    void resultClicked(QTableWidgetItem* item);
    void chkToggled();
    void chkAllToggled(bool toggled);

private:
    Ui::ConcertSearchWidget* ui;
    int m_scraperNo = 0;
    TmdbId m_scraperId;
    QSet<ConcertScraperInfos> m_infosToLoad;

    void clear();
    void setCheckBoxesEnabled(QSet<ConcertScraperInfos> scraperSupports);
};
