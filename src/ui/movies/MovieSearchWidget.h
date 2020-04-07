#pragma once

#include "data/ImdbId.h"
#include "data/TmdbId.h"
#include "globals/Globals.h"
#include "globals/ScraperInfos.h"
#include "scrapers/movie/MovieScraper.h"

#include <QMap>
#include <QString>
#include <QTableWidgetItem>
#include <QVector>
#include <QWidget>

namespace Ui {
class MovieSearchWidget;
}

class MovieSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MovieSearchWidget(QWidget* parent = nullptr);
    ~MovieSearchWidget() override;

    QString scraperId();
    QString scraperMovieId();
    QVector<MovieScraperInfos> infosToLoad();
    QHash<mediaelch::scraper::MovieScraper*, QString> customScraperIds();

    /// \brief Initialize the MovieSearchWidget and start searching. Called by MovieSearch.
    void search(QString searchString, ImdbId id, TmdbId tmdbId);

signals:
    void sigResultClicked();

private slots:
    void startSearch();
    void onSearchSuccess(QVector<mediaelch::scraper::MovieSearchJob::Result> results);
    void onSearchError(ScraperSearchError error);
    void resultClicked(QTableWidgetItem* item);
    void updateInfoToLoad();
    void toggleAllInfo(bool checked);
    void onScraperChanged();
    void onLanguageChanged();

private:
    Ui::MovieSearchWidget* ui = nullptr;
    QString m_scraperId;
    QString m_scraperMovieId;
    QVector<MovieScraperInfos> m_infosToLoad;
    QHash<mediaelch::scraper::MovieScraper*, QString> m_customScraperIds;
    mediaelch::scraper::MovieScraper* m_currentCustomScraper = nullptr;
    mediaelch::scraper::MovieScraper* m_currentScraper = nullptr;
    QString m_currentLocale;
    ImdbId m_imdbId;
    TmdbId m_tmdbId;
    QString m_searchString;

    void clearResults();
    void setCheckBoxesEnabled(const QVector<MovieScraperInfos>& scraperSupports);
    void setupComboBoxes();
    void setSearchText(mediaelch::scraper::MovieScraper* scraper);
    void setupScraperDropdown();
    void setupLanguageDropdown();
    void initializeCheckBoxes();
    int currentScraperIndex();
    void showError(const QString& message);
    void showSuccess(const QString& message);
};
