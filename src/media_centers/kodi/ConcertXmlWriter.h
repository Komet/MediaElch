#pragma once

#include <QByteArray>
#include <QDomElement>
#include <QString>

class Concert;

namespace mediaelch {
namespace kodi {

class ConcertXmlWriter
{
public:
    explicit ConcertXmlWriter(Concert& concert);
    QByteArray getConcertXml();

private:
    Concert& m_concert;
};

} // namespace kodi
} // namespace mediaelch
