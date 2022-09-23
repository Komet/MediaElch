#pragma once

#include <QDomElement>

class Album;

namespace mediaelch {
namespace kodi {

class AlbumXmlReader
{
public:
    explicit AlbumXmlReader(Album& album);
    void parseNfoDom(QDomDocument domDoc);

private:
    Album& m_album;
};

} // namespace kodi
} // namespace mediaelch
