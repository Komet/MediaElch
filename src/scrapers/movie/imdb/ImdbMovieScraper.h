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
};

} // namespace scraper
} // namespace mediaelch
