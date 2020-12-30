#pragma once

#include <QByteArray>

namespace mediaelch {
namespace kodi {

class TvShowXmlWriter
{
public:
    virtual ~TvShowXmlWriter() = default;
    virtual QByteArray getTvShowXml(bool testMode = false) = 0;
};

} // namespace kodi
} // namespace mediaelch
