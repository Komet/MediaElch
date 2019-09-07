#pragma once

#include "media_centers/kodi/AlbumXmlWriter.h"

#include <QByteArray>
#include <QDomDocument>

class Album;

namespace mediaelch {
namespace kodi {

class AlbumXmlWriterV17 : public AlbumXmlWriter
{
public:
    explicit AlbumXmlWriterV17(Album& album);
    QByteArray getAlbumXml() override;

private:
    Album& m_album;

    void writeArtistCredits(QDomDocument& doc);
};

} // namespace kodi
} // namespace mediaelch
