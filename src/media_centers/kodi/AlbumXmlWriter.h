#pragma once

#include "media_centers/kodi/KodiXmlWriter.h"

#include <QByteArray>
#include <QXmlStreamWriter>

class Album;

namespace mediaelch {
namespace kodi {

class AlbumXmlWriter : public KodiXmlWriter
{
public:
    AlbumXmlWriter(KodiVersion version) : KodiXmlWriter(std::move(version)) {}
    virtual ~AlbumXmlWriter() = default;
    virtual QByteArray getAlbumXml(bool testMode = false) = 0;
};

class AlbumXmlWriterGeneric : public AlbumXmlWriter
{
public:
    explicit AlbumXmlWriterGeneric(KodiVersion version, Album& album);
    QByteArray getAlbumXml(bool testMode = false) override;

private:
    void writeAlbumTags(QXmlStreamWriter& xml);

private:
    Album& m_album;

    void writeArtistCredits(QXmlStreamWriter& xml);
};

} // namespace kodi
} // namespace mediaelch
