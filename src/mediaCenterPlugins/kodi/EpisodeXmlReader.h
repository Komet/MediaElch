#ifndef KODI_EPISODEXMLREADER_H
#define KODI_EPISODEXMLREADER_H

#include <QDomElement>
#include <QString>

class TvShowEpisode;

namespace Kodi {

class EpisodeXmlReader
{
public:
    EpisodeXmlReader(TvShowEpisode &episode);
    void parseNfoDom(QDomDocument domDoc, QDomElement episodeDetails);

private:
    TvShowEpisode &m_episode;
};

} // namespace Kodi

#endif // KODI_EPISODEXMLREADER_H
