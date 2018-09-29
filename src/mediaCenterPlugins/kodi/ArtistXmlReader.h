#ifndef KODI_ARTIST_XML_READER
#define KODI_ARTIST_XML_READER

#include <QDomElement>

class Artist;

namespace Kodi {

class ArtistXmlReader
{
public:
    ArtistXmlReader(Artist &artist);
    void parseNfoDom(QDomDocument domDoc);

private:
    Artist &m_artist;
};

} // namespace Kodi

#endif // KODI_ARTIST_XML_READER
