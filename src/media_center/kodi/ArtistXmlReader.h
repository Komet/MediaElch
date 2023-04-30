#pragma once

#include "utils/Meta.h"

#include <QDomElement>

class Artist;

namespace mediaelch {
namespace kodi {

class ArtistXmlReader
{
public:
    explicit ArtistXmlReader(Artist& artist);
    ELCH_NODISCARD bool parseNfoDom(QDomDocument domDoc);

private:
    Artist& m_artist;
};

} // namespace kodi
} // namespace mediaelch
