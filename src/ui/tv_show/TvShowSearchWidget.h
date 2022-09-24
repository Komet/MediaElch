#pragma once

#include "data/tv_show/SeasonOrder.h"
#include "globals/Globals.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/ScraperResult.h"
#include "scrapers/tv_show/TvScraper.h"

#include <QTableWidgetItem>
#include <QWidget>

namespace Ui {
class TvShowSearchWidget;
}

class TvShowSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowSearchWidget(QWidget* parent = nullptr);
    ~TvShowSearchWidget() override;

    void setSearchType(TvShowType type);

    QString showIdentifier() const;
    mediaelch::scraper::TvScraper* scraper();

    /// \brief Returns the selected locale.
    /// \note Do not call this method "locale" as it refers to QWidget's locale().
    const mediaelch::Locale& scraperLocale() const;
    SeasonOrder seasonOrder() const;
    const QSet<ShowScraperInfo>& showDetailsToLoad() const;
    const QSet<EpisodeScraperInfo>& episodeDetailsToLoad() const;
    TvShowUpdateType updateType() const;

public slots:
    void search(QString searchString);

signals:
    void sigResultClicked();

private slots:
    /// \brief Initializes the current scraper if necessary and start the TV show search.
    void initializeAndStartSearch();
    /// \brief Starts the TV show search with the selected _initialized_ scraper.
    void startSearch();
    void onShowResults(mediaelch::scraper::ShowSearchJob* searchJob);
    /// \brief Stores the clicked id and accepts the dialog.
    /// \param item Item which was clicked
    void onResultClicked(QTableWidgetItem* item);
    void onShowInfoToggled();
    void onEpisodeInfoToggled();
    void onChkAllShowInfosToggled();
    void onChkAllEpisodeInfosToggled();
    void onUpdateTypeChanged(int index);
    void onSeasonOrderChanged(int index);
    void onScraperChanged(int index);
    void onLanguageChanged();

private:
    void setupSeasonOrderComboBox();
    void setupScraperDropdown();
    void setupLanguageDropdown();
    void showError(const QString& message);
    void showSuccess(const QString& message);
    void clearResultTable();
    void updateCheckBoxes();

private:
    Ui::TvShowSearchWidget* ui = nullptr;
    TvShowType m_searchType = TvShowType::None;

    QString m_showIdentifier;
    QSet<ShowScraperInfo> m_showDetailsToLoad;
    QSet<EpisodeScraperInfo> m_episodeDetailsToLoad;
    SeasonOrder m_seasonOrder = SeasonOrder::Aired;
    TvShowUpdateType m_updateType = TvShowUpdateType::Show;

    mediaelch::scraper::TvScraper* m_currentScraper = nullptr;
    mediaelch::Locale m_currentLanguage = mediaelch::Locale::English;
};
