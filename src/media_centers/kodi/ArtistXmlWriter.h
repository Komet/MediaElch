#pragma once

#include <QByteArray>
#include <QDomElement>
#include <QString>

namespace mediaelch {
namespace kodi {

class ArtistXmlWriter
{
public:
    virtual ~ArtistXmlWriter() = default;
    virtual QByteArray getArtistXml() = 0;
};

} // namespace kodi
} // namespace mediaelch
