#pragma once

#include "media_centers/kodi/KodiXmlWriter.h"

#include <QByteArray>
#include <QString>
#include <QXmlStreamWriter>

class Artist;

namespace mediaelch {
namespace kodi {

class ArtistXmlWriter : public KodiXmlWriter
{
public:
    ArtistXmlWriter(KodiVersion version) : KodiXmlWriter(std::move(version)) {}
    virtual ~ArtistXmlWriter() = default;
    virtual QByteArray getArtistXml(bool testMode = false) = 0;
};

class ArtistXmlWriterGeneric : public ArtistXmlWriter
{
public:
    explicit ArtistXmlWriterGeneric(KodiVersion version, Artist& artist);
    QByteArray getArtistXml(bool testMode = false) override;

private:
    void writeArtistTags(QXmlStreamWriter& xml);

private:
    Artist& m_artist;
};

} // namespace kodi
} // namespace mediaelch
