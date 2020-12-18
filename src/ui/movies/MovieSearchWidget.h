#pragma once

#include "data/ImdbId.h"
#include "data/TmdbId.h"
#include "globals/Globals.h"
#include "globals/ScraperInfos.h"
#include "scrapers/movie/MovieScraperInterface.h"

#include <QMap>
#include <QString>
#include <QTableWidgetItem>
#include <QVector>
#include <QWidget>

namespace Ui {
class MovieSearchWidget;
}

class MovieScraperInterface;

class MovieSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MovieSearchWidget(QWidget* parent = nullptr);
    ~MovieSearchWidget() override;

public slots:
    QString scraperId();
    QString scraperMovieId();
    QSet<MovieScraperInfo> infosToLoad();
    QHash<MovieScraperInterface*, QString> customScraperIds();
    void search(QString searchString, ImdbId id, TmdbId tmdbId);

signals:
    void sigResultClicked();

private slots:
    void startSearch();
    void showResults(QVector<ScraperSearchResult> results, ScraperError error);
    void resultClicked(QTableWidgetItem* item);
    void updateInfoToLoad();
    void toggleAllInfo(bool checked);
    void onScraperChanged();
    void onLanguageChanged();

private:
    Ui::MovieSearchWidget* ui = nullptr;
    // QString m_scraperId;
    QString m_scraperMovieId;
    QSet<MovieScraperInfo> m_infosToLoad;
    QHash<MovieScraperInterface*, QString> m_customScraperIds;
    MovieScraperInterface* m_currentCustomScraper = nullptr;
    MovieScraperInterface* m_currentScraper = nullptr;
    mediaelch::Locale m_currentLanguage = mediaelch::Locale::English;
    ImdbId m_imdbId;
    TmdbId m_tmdbId;
    QString m_searchString;

    void clearResults();
    void setCheckBoxesEnabled(QSet<MovieScraperInfo> scraperSupports);
    void setupComboBoxes();
    void setSearchText(MovieScraperInterface* scraper);
    void setupScraperDropdown();
    void setupLanguageDropdown();
    void initializeCheckBoxes();
    int currentScraperIndex();
    void showError(const QString& message);
    void showSuccess(const QString& message);
};
