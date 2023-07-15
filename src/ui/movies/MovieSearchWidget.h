#pragma once

#include "data/ImdbId.h"
#include "data/TmdbId.h"
#include "globals/Globals.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/MovieSearchJob.h"

#include <QMap>
#include <QPointer>
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

    /// \brief Returns the selected locale.
    /// \note Do not call this method "locale" as it refers to QWidget's locale().
    const mediaelch::Locale& scraperLocale() const;

public slots:
    /// \brief Initialize the MovieSearchWidget and start searching. Called by MovieSearch.
    void openAndSearch(QString searchString, const ImdbId& imdbId, const TmdbId& tmdbId);

    /// \brief Used by the surrounding dialog when "scrape" button is clicked.
    /// \todo Remove; breaks dependency chain; move buttons to widget?
    void onScrapeSelectedMovie();

public:
    QString scraperId();
    QString scraperMovieId();
    QSet<MovieScraperInfo> infosToLoad();
    QHash<mediaelch::scraper::MovieScraper*, mediaelch::scraper::MovieIdentifier> customScraperIds();

signals:
    void sigResultClicked();
    /// \brief Emitted when a different movie is selected. \p isSelected is false if there is no movie selected.
    void sigMovieSelectionChanged(bool isSelected);

private slots:
    void startSearch();
    void onShowResults(mediaelch::scraper::MovieSearchJob* searchJob);

    /// \brief When the selected item changes, e.g. via click or keys.
    /// \param current Currently selected result.
    /// \param previous Previously selected result. Unused, only due to Qt API.
    void onSelectedResultChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
    /// \brief Stores the double-clicked id and accepts the dialog.
    /// \param item Item which was clicked
    void onResultDoubleClicked(QTableWidgetItem* item);

    void updateInfoToLoad();
    void toggleAllInfo(bool checked);
    void onScraperChanged(int index);
    void onLanguageChanged();
    void onCustomMovieScraperSelected();

private:
    bool isCustomScrapingInProgress() { return !m_customScrapersLeft.isEmpty(); }

    void abortAndClearResults();
    void abortCurrentJobs();
    void setCheckBoxesForCurrentScraper();
    void setupComboBoxes();
    void setSearchText();
    void setupScraperDropdown();
    void setupLanguageDropdown();
    void initializeCheckBoxes();
    int storedScraperIndex();
    /// \brief Changes the current scraper and triggers a search.
    /// \details Returns true if the scraper was selected, otherwise false.
    bool changeScraperTo(const QString& scraperId);
    void createCustomScraperListLabel();

    void showError(const QString& message);
    void showSuccess(const QString& message);

private:
    Ui::MovieSearchWidget* ui{nullptr};

    // Selected scraper or the currently used scraper for the custom movie scraper.
    mediaelch::scraper::MovieScraper* m_currentScraper{nullptr};
    QPointer<mediaelch::scraper::MovieSearchJob> m_currentSearchJob{nullptr};

    QString m_scraperMovieId;
    QSet<MovieScraperInfo> m_infosToLoad;

    QHash<mediaelch::scraper::MovieScraper*, mediaelch::scraper::MovieIdentifier> m_customScraperIds;
    QVector<QString> m_customScrapersLeft;

    mediaelch::Locale m_currentLanguage = mediaelch::Locale::English;
    ImdbId m_imdbId;
    TmdbId m_tmdbId;
    QString m_searchString;
};
