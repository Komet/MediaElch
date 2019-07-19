#pragma once

#include <QByteArray>
#include <QDomElement>
#include <QString>

class Artist;

namespace mediaelch {
namespace kodi {

class ArtistXmlWriter
{
public:
    explicit ArtistXmlWriter(Artist& artist);
    QByteArray getArtistXml();

private:
    Artist& m_artist;
};

} // namespace kodi
} // namespace mediaelch
