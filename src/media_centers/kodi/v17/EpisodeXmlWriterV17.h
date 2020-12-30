#pragma once

#include "media_centers/kodi/EpisodeXmlWriter.h"

#include <QByteArray>
#include <QVector>
#include <QXmlStreamWriter>

class TvShowEpisode;

namespace mediaelch {
namespace kodi {

class EpisodeXmlWriterV17 : public EpisodeXmlWriter
{
public:
    EpisodeXmlWriterV17(QVector<TvShowEpisode*> episodes);
    QByteArray getEpisodeXml(bool testMode = false) override;

private:
    void writeSingleEpisodeDetails(QXmlStreamWriter& xml, TvShowEpisode* episode, bool testMode);

    const QVector<TvShowEpisode*> m_episodes;
};

} // namespace kodi
} // namespace mediaelch
