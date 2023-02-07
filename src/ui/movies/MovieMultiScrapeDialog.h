#pragma once

#include "data/movie/Movie.h"
#include "scrapers/ScraperResult.h"
#include "scrapers/movie/MovieIdentifier.h"
#include "scrapers/movie/MovieSearchJob.h"

#include <QDialog>
#include <QPointer>
#include <QQueue>

namespace Ui {
class MovieMultiScrapeDialog;
}

namespace mediaelch {
namespace scraper {
class MovieSearchJob;
}
} // namespace mediaelch

/// \brief Dialog for scraping multiple movies at once.
class MovieMultiScrapeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MovieMultiScrapeDialog(QWidget* parent = nullptr);
    ~MovieMultiScrapeDialog() override;

    /// \brief Set the movies that should be scraped.
    void setMovies(QVector<Movie*> movies);

public slots:
    int exec() override;
    void reject() override;
    void accept() override;

private slots:
    void onStartScraping();
    void onScrapingFinished();
    void onSearchFinished(mediaelch::scraper::MovieSearchJob* searchJob);
    void onCustomScraperSearchFinished(mediaelch::scraper::MovieSearchJob* searchJob);
    void scrapeNext();
    void onProgress(Movie* movie, int current, int maximum);
    void updateInfoToLoad();
    void toggleAllInfo(bool checked);
    void onScraperChanged(int index);
    void onLanguageChanged();

private:
    void setupLanguageDropdown();
    void setupScraperDropdown();
    void showError(const QString& message);
    void setCheckBoxesForCurrentScraper();
    int storedScraperIndex();

private:
    Ui::MovieMultiScrapeDialog* ui{nullptr};
    QVector<Movie*> m_movies;
    QQueue<Movie*> m_queue;
    QPointer<Movie> m_currentMovie{nullptr};

    mediaelch::scraper::MovieScraper* m_currentScraper{nullptr};
    QHash<mediaelch::scraper::MovieScraper*, mediaelch::scraper::MovieIdentifier> m_currentIds;
    mediaelch::Locale m_currentLanguage = mediaelch::Locale::English;
    /// \brief   List of scrapers that need to be search if the custom movie scraper is selected.
    /// \details The list depends on the details to be loaded.
    QVector<mediaelch::scraper::MovieScraper*> customScrapersToUse;

    bool m_executed{false};
    QSet<MovieScraperInfo> m_infosToLoad;

    void loadMovieData(Movie* movie, const ImdbId& id);
    void loadMovieData(Movie* movie, const TmdbId& id);
    bool isExecuted() const;
    void initializeCheckBoxes();
    void startNextCustomScraperSearch();
    QVector<mediaelch::scraper::MovieScraper*> remainingScrapersForCustomScraperSearch() const;
};
