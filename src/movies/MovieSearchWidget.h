#ifndef MOVIESEARCHWIDGET_H
#define MOVIESEARCHWIDGET_H

#include <QTableWidgetItem>
#include <QWidget>

#include "data/ScraperInterface.h"
#include "globals/Globals.h"

namespace Ui {
class MovieSearchWidget;
}

class MovieSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MovieSearchWidget(QWidget *parent = nullptr);
    ~MovieSearchWidget() override;

public slots:
    QString scraperId();
    QString scraperMovieId();
    QList<MovieScraperInfos> infosToLoad();
    QMap<ScraperInterface *, QString> customScraperIds();
    void search(QString searchString, QString id, QString tmdbId);

signals:
    void sigResultClicked();

private slots:
    void search();
    void showResults(QList<ScraperSearchResult> results);
    void resultClicked(QTableWidgetItem *item);
    void chkToggled();
    void chkAllToggled(bool toggled);
    void onUpdateSearchString();
    void onStoreSearchString(QString searchString);

private:
    Ui::MovieSearchWidget *ui;
    QString m_scraperId;
    QString m_scraperMovieId;
    QList<MovieScraperInfos> m_infosToLoad;
    QMap<ScraperInterface *, QString> m_customScraperIds;
    ScraperInterface *m_currentCustomScraper;
    QString m_id;
    QString m_tmdbId;
    QString m_searchString;

    void clear();
    void setChkBoxesEnabled(QList<MovieScraperInfos> scraperSupports);
    void setupScrapers();
};

#endif // MOVIESEARCHWIDGET_H
