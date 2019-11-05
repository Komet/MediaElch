#include "globals/Globals.h"


bool isShowUpdateType(TvShowUpdateType type)
{
    return (type == TvShowUpdateType::Show || type == TvShowUpdateType::ShowAndAllEpisodes
            || type == TvShowUpdateType::ShowAndNewEpisodes);
}

bool isEpisodeUpdateType(TvShowUpdateType type)
{
    return (type == TvShowUpdateType::NewEpisodes || type == TvShowUpdateType::AllEpisodes
            || type == TvShowUpdateType::ShowAndAllEpisodes || type == TvShowUpdateType::ShowAndNewEpisodes);
}

bool isNewEpisodeUpdateType(TvShowUpdateType type)
{
    return (type == TvShowUpdateType::NewEpisodes || type == TvShowUpdateType::ShowAndNewEpisodes);
}

bool isAllEpisodeUpdateType(TvShowUpdateType type)
{
    return (type == TvShowUpdateType::ShowAndAllEpisodes || type == TvShowUpdateType::AllEpisodes);
}

QDebug operator<<(QDebug lhs, const ScraperSearchResult& rhs)
{
    lhs << QString(R"(("%1", "%2", %3))").arg(rhs.id, rhs.name, rhs.released.toString("yyyy"));
    return lhs;
}
