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
