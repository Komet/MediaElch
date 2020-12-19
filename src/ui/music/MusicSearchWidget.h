#pragma once

#include "globals/Globals.h"
#include "globals/ScraperInfos.h"
#include "globals/ScraperResult.h"
#include "scrapers/movie/MovieScraper.h"

#include <QSignalMapper>
#include <QString>
#include <QTableWidgetItem>
#include <QVector>
#include <QWidget>

namespace Ui {
class MusicSearchWidget;
}

class MusicSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MusicSearchWidget(QWidget* parent = nullptr);
    ~MusicSearchWidget() override;

public slots:
    void search(QString searchString);
    int scraperNo() const;
    QString scraperId();
    QString scraperId2();
    QSet<MusicScraperInfo> infosToLoad();
    void setType(const QString& type);
    void setArtistName(const QString& artistName);

signals:
    void sigResultClicked();

private slots:
    void startSearch();
    void startSearchWithIndex(int index);
    void showResults(QVector<ScraperSearchResult> results);
    void resultClicked(QTableWidgetItem* item);
    void resultClickedRow(int row);
    void chkToggled();
    void chkAllToggled(bool toggled);

private:
    Ui::MusicSearchWidget* ui;
    int m_scraperNo = 0;
    QString m_scraperId;
    QString m_scraperId2;
    QSet<MusicScraperInfo> m_infosToLoad;
    QString m_type;
    QString m_artistName;
    QSignalMapper* m_signalMapper;

    void clear();
    void setCheckBoxesEnabled(QSet<MusicScraperInfo> scraperSupports);
};
