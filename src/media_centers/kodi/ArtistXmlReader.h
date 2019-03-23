#pragma once

#include <QDomElement>

class Artist;

namespace Kodi {

class ArtistXmlReader
{
public:
    ArtistXmlReader(Artist& artist);
    void parseNfoDom(QDomDocument domDoc);

private:
    Artist& m_artist;
};

} // namespace Kodi
