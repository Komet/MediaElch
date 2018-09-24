#ifndef MOVIESEARCHWIDGET_H
#define MOVIESEARCHWIDGET_H

#include "data/ImdbId.h"
#include "data/TmdbId.h"
#include "globals/Globals.h"

#include <QList>
#include <QMap>
#include <QString>
#include <QTableWidgetItem>
#include <QWidget>

namespace Ui {
class MovieSearchWidget;
}

class ScraperInterface;

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
    void search(QString searchString, ImdbId id, TmdbId tmdbId);

signals:
    void sigResultClicked();

private slots:
    void startSearch();
    void showResults(QList<ScraperSearchResult> results);
    void resultClicked(QTableWidgetItem *item);
    void updateInfoToLoad();
    void toggleAllInfo(bool checked);
    void onScraperChanged();
    void onLanguageChanged();

private:
    Ui::MovieSearchWidget *ui;
    // QString m_scraperId;
    QString m_scraperMovieId;
    QList<MovieScraperInfos> m_infosToLoad;
    QMap<ScraperInterface *, QString> m_customScraperIds;
    ScraperInterface *m_currentCustomScraper;
    ScraperInterface *m_currentScraper;
    QString m_currentLanguage;
    ImdbId m_imdbId;
    TmdbId m_tmdbId;
    QString m_searchString;

    void clearResults();
    void setCheckBoxesEnabled(QList<MovieScraperInfos> scraperSupports);
    void setupComboBoxes();
    void setSearchText(ScraperInterface *scraper);
    void setupScraperDropdown();
    void setupLanguageDropdown();
    void initializeCheckBoxes();
    int currentScraperIndex();
};

#endif // MOVIESEARCHWIDGET_H
