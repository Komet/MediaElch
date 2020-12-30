#pragma once

#include <QByteArray>

namespace mediaelch {
namespace kodi {

class AlbumXmlWriter
{
public:
    virtual ~AlbumXmlWriter() = default;
    virtual QByteArray getAlbumXml(bool testMode = false) = 0;
};

} // namespace kodi
} // namespace mediaelch
