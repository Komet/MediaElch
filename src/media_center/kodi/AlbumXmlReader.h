#pragma once

#include "utils/Meta.h"

#include <QDomElement>

class Album;

namespace mediaelch {
namespace kodi {

class AlbumXmlReader
{
public:
    explicit AlbumXmlReader(Album& album);
    /// \brief Parse album's details from the document. Returns true for success.
    ELCH_NODISCARD bool parseNfoDom(QDomDocument domDoc);

private:
    Album& m_album;
};

} // namespace kodi
} // namespace mediaelch
