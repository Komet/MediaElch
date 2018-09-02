#ifndef EPISODE_RENAMER_H
#define EPISODE_RENAMER_H

#include "renamer/Renamer.h"

class RenamerDialog;
class TvShowEpisode;

class EpisodeRenamer : public Renamer
{
public:
    EpisodeRenamer(RenamerConfig renamerConfig, RenamerDialog *dialog);
    RenameError renameEpisode(TvShowEpisode &episode, QList<TvShowEpisode *> &episodesRenamed);
};

#endif // EPISODE_RENAMER_H
