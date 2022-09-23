#pragma once

#include <QDomElement>

class Artist;

namespace mediaelch {
namespace kodi {

class ArtistXmlReader
{
public:
    explicit ArtistXmlReader(Artist& artist);
    void parseNfoDom(QDomDocument domDoc);

private:
    Artist& m_artist;
};

} // namespace kodi
} // namespace mediaelch
