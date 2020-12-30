#pragma once

#include <QByteArray>

class Concert;

namespace mediaelch {
namespace kodi {

class ConcertXmlWriter
{
public:
    virtual ~ConcertXmlWriter() = default;
    virtual QByteArray getConcertXml(bool testMode = false) = 0;
};

} // namespace kodi
} // namespace mediaelch
