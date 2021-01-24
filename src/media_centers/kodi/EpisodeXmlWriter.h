#pragma once

#include "media_centers/kodi/KodiXmlWriter.h"

#include <QByteArray>
#include <QXmlStreamWriter>

class TvShowEpisode;

namespace mediaelch {
namespace kodi {

class EpisodeXmlWriter : public KodiXmlWriter
{
public:
    EpisodeXmlWriter(KodiVersion version) : KodiXmlWriter(std::move(version)) {}
    virtual ~EpisodeXmlWriter() = default;
    virtual QByteArray getEpisodeXml(bool testMode = false) = 0;
    /// Get the episode's XML content with an extra root element: <episodes>
    /// Used for testing to allow multi-episodes in one DOM.
    virtual QString getEpisodeXmlWithSingleRoot(bool testMode);
};

class EpisodeXmlWriterGeneric : public EpisodeXmlWriter
{
public:
    EpisodeXmlWriterGeneric(KodiVersion version, const QVector<TvShowEpisode*>& episodes);
    QByteArray getEpisodeXml(bool testMode = false) override;

    bool usePlotForOutline() const;
    void setUsePlotForOutline(bool usePlotForOutline);

private:
    void writeSingleEpisodeDetails(QXmlStreamWriter& xml, TvShowEpisode* episode, bool testMode);

    const QVector<TvShowEpisode*> m_episodes;
    bool m_usePlotForOutline = false;
};


} // namespace kodi
} // namespace mediaelch
