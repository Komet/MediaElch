#ifndef KODI_ARTIST_XML_WRITER
#define KODI_ARTIST_XML_WRITER

#include <QByteArray>
#include <QDomElement>
#include <QString>

class Artist;

namespace Kodi {

class ArtistXmlWriter
{
public:
    ArtistXmlWriter(Artist &artist);
    QByteArray getArtistXml();

private:
    Artist &m_artist;
};

} // namespace Kodi

#endif // KODI_ARTIST_XML_WRITER
