#pragma once

#include "data/ImdbId.h"
#include "data/TmdbId.h"
#include "globals/Globals.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/movie/MovieScraper.h"

#include <QMap>
#include <QString>
#include <QTableWidgetItem>
#include <QVector>
#include <QWidget>

namespace Ui {
class MovieSearchWidget;
}

namespace mediaelch {
namespace scraper {
class MovieScraper;
}
} // namespace mediaelch

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

    QString scraperId();
    QString scraperMovieId();
    QSet<MovieScraperInfo> infosToLoad();
    QHash<mediaelch::scraper::MovieScraper*, mediaelch::scraper::MovieIdentifier> customScraperIds();

signals:
    void sigResultClicked();

private slots:
    void startSearch();
    void showResults(mediaelch::scraper::MovieSearchJob* searchJob);
    void resultClicked(QTableWidgetItem* item);
    void updateInfoToLoad();
    void toggleAllInfo(bool checked);
    void onScraperChanged(int index);
    void onLanguageChanged();
    void onCustomMovieScraperSelected();

private:
    bool isCustomScrapingInProgress() { return !m_customScrapersLeft.isEmpty(); }

    void clearResults();
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

    QString m_scraperMovieId;
    QSet<MovieScraperInfo> m_infosToLoad;

    QHash<mediaelch::scraper::MovieScraper*, mediaelch::scraper::MovieIdentifier> m_customScraperIds;
    QVector<QString> m_customScrapersLeft;

    mediaelch::Locale m_currentLanguage = mediaelch::Locale::English;
    ImdbId m_imdbId;
    TmdbId m_tmdbId;
    QString m_searchString;
};
