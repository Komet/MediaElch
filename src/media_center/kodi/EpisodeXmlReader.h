#pragma once

#include "utils/Meta.h"

#include <QDomElement>
#include <QString>

class TvShowEpisode;

namespace mediaelch {
namespace kodi {

class EpisodeXmlReader
{
public:
    explicit EpisodeXmlReader(TvShowEpisode& episode);
    ELCH_NODISCARD bool parseNfoDom(QDomElement episodeDetails);

    static QString makeValidEpisodeXml(const QString& nfoContent);

private:
    TvShowEpisode& m_episode;
};

} // namespace kodi
} // namespace mediaelch
