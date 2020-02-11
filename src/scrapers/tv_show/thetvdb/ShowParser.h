#pragma once

#include "globals/Globals.h"
#include "tv_shows/TvShow.h"

#include <QString>
#include <QVector>
#include <memory>
#include <vector>

namespace thetvdb {

using ApiPage = int;

struct Paginate
{
    ApiPage first{0};
    ApiPage last{0};
    ApiPage next{0};
    ApiPage prev{0};

    bool hasNextPage() { return next > 0; }
};

class ShowParser
{
public:
    ShowParser(TvShow& show, QVector<TvShowScraperInfos> showInfosToLoad) : m_show{show}, m_infosToLoad{showInfosToLoad}
    {
    }

    void parseInfos(const QString& json);
    void parseActors(const QString& json);
    void parseImages(const QString& json);
    Paginate parseEpisodes(const QString& json, QVector<TvShowScraperInfos> episodeInfosToLoad);

    const std::vector<std::unique_ptr<TvShowEpisode>>& episodes() const { return m_episodes; }

private:
    TvShow& m_show;
    QVector<TvShowScraperInfos> m_infosToLoad;
    // not using QVector because "append" does not work with unique_ptr&&.
    std::vector<std::unique_ptr<TvShowEpisode>> m_episodes;
    QVector<TvShowEpisode*> m_updatedEpisodes;
};

} // namespace thetvdb
