#pragma once

#include "media_centers/kodi/ArtistXmlWriter.h"

#include <QByteArray>
#include <QDomElement>
#include <QString>

class Artist;

namespace mediaelch {
namespace kodi {

class ArtistXmlWriterV16 : public ArtistXmlWriter
{
public:
    explicit ArtistXmlWriterV16(Artist& artist);
    QByteArray getArtistXml() override;

private:
    Artist& m_artist;
};

} // namespace kodi
} // namespace mediaelch
