#pragma once

#include "data/ImdbId.h"
#include "globals/ScraperInfos.h"
#include "scrapers/movie/IMDB.h"
#include "scrapers/tv_show/TvScraperInterface.h"
#include "tv_shows/EpisodeNumber.h"
#include "tv_shows/SeasonNumber.h"

#include <QComboBox>
#include <QDomElement>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <functional>

/**
 * @brief The TheTvDb class
 */
class TheTvDb : public TvScraperInterface
{
    Q_OBJECT

public:
    explicit TheTvDb(QObject* parent = nullptr);
    static constexpr const char* scraperIdentifier = "TheTvDb";

    // clang-format off
    QString name()            const override { return QStringLiteral("The TV DB");        }
    QString identifier()      const override { return scraperIdentifier;                  }
    QString apiKey()          const          { return QStringLiteral("A0BB9A0F6762942B"); }
    QString language()        const          { return m_language;                         }
    bool     hasSettings()    const override { return true;                               }
    QWidget* settingsWidget() const override { return m_widget;                           }
    // clang-format on

    void search(QString searchStr) override;
    void loadTvShowData(TvDbId id,
        TvShow* show,
        TvShowUpdateType updateType,
        QVector<TvShowScraperInfos> infosToLoad) override;
    void loadTvShowEpisodeData(TvDbId id, TvShowEpisode* episode, QVector<TvShowScraperInfos> infosToLoad) override;

    void fillDatabaseWithAllEpisodes(TvShow& show, std::function<void()> callback);

    void loadSettings(const ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;

signals:
    void sigLoadProgress(TvShow*, int episodesLoaded, int episodesToLoad);

private slots:
    void onImdbSeasonLoaded();
    void onImdbEpisodeLoaded();
    void onEpisodesImdbEpisodeLoaded();
    void onEpisodesImdbSeasonLoaded();

private:
    QString m_language{"en"};
    QNetworkAccessManager m_qnam;

    // UI
    QComboBox* m_languageComboBox = nullptr;
    QWidget* m_widget = nullptr;
    IMDB* m_imdb = nullptr;
    Movie* m_dummyMovie = nullptr;

    QVector<MovieScraperInfos> m_movieInfos;

    void setupLanguages();
    void setupLayout();

    void parseAndAssignImdbInfos(const QString& html,
        TvShow& show,
        TvShowUpdateType updateType,
        QVector<TvShowScraperInfos> infosToLoad);
    void parseAndAssignImdbInfos(const QString& html, TvShowEpisode& episode, QVector<TvShowScraperInfos> infosToLoad);

    void loadEpisodesFromImdb(TvShow& show, QVector<TvShowEpisode*> episodes, QVector<TvShowScraperInfos> infosToLoad);
    void loadShowFromImdb(TvShow& show,
        const QVector<TvShowScraperInfos>& infosToLoad,
        TvShowUpdateType updateType,
        QVector<TvShowEpisode*> episodesToLoad);

    ImdbId getImdbIdForEpisode(const QString& html, EpisodeNumber episodeNumber);

    bool shouldLoadImdb(QVector<TvShowScraperInfos> infosToLoad) const;
    bool shouldLoadFromImdb(TvShowScraperInfos info, QVector<TvShowScraperInfos> infosToLoad);
};
