#pragma once

#include "data/Storage.h"
#include "globals/ScraperInfos.h"
#include "movies/Movie.h"
#include "network/NetworkManager.h"

#include <QObject>
#include <QString>

namespace mediaelch {
namespace scraper {

class ImdbMovie;
class ImdbApi;

class ImdbMovieLoader : public QObject
{
    Q_OBJECT
public:
    ImdbMovieLoader(ImdbApi& api,
        ImdbMovie& scraper,
        ImdbId imdbId,
        Movie& movie,
        QSet<MovieScraperInfo> infos,
        bool loadAllTags,
        QObject* parent = nullptr) :
        QObject(parent),
        m_api{api},
        m_scraper{scraper},
        m_imdbId{std::move(imdbId)},
        m_movie{movie},
        m_infos{std::move(infos)},
        m_loadAllTags{loadAllTags}
    {
    }

    void load();

signals:
    void sigLoadDone(Movie& movie, ImdbMovieLoader* loader);

private:
    void loadPoster(const QUrl& posterViewerUrl);
    void loadTags();
    void loadActorImageUrls();

    void parseAndAssignInfos(const QString& html);
    void parseAndAssignPoster(const QString& html);
    void parseAndStoreActors(const QString& html);
    QUrl parsePosterViewerUrl(const QString& html);
    void parseAndAssignTags(const QString& html);
    QString parseActorImageUrl(const QString& html);

    void mergeActors();
    void decreaseDownloadCount();

private:
    int m_itemsLeftToDownloads = 0;

    ImdbApi& m_api;
    ImdbMovie& m_scraper;
    ImdbId m_imdbId;
    Movie& m_movie;
    QSet<MovieScraperInfo> m_infos;
    mediaelch::network::NetworkManager m_network;
    bool m_loadAllTags = false;

    QVector<QPair<Actor, QUrl>> m_actorUrls;
};

} // namespace scraper
} // namespace mediaelch
