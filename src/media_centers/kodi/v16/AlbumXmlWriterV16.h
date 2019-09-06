#pragma once

#include "media_centers/kodi/AlbumXmlWriter.h"

#include <QByteArray>

class Album;

namespace mediaelch {
namespace kodi {

class AlbumXmlWriterV16 : public AlbumXmlWriter
{
public:
    explicit AlbumXmlWriterV16(Album& album);
    QByteArray getAlbumXml() override;

private:
    Album& m_album;
};

} // namespace kodi
} // namespace mediaelch
