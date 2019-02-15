#pragma once
#include "renamer/Renamer.h"

class RenamerDialog;
class TvShowEpisode;

class EpisodeRenamer : public Renamer
{
public:
    EpisodeRenamer(RenamerConfig renamerConfig, RenamerDialog* dialog);
    RenameError renameEpisode(TvShowEpisode& episode, QVector<TvShowEpisode*>& episodesRenamed);
};
