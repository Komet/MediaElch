#pragma once

#include <QByteArray>
#include <QDomElement>
#include <QString>

class Concert;

namespace kodi {

class ConcertXmlWriter
{
public:
    ConcertXmlWriter(Concert& concert);
    QByteArray getConcertXml();

private:
    Concert& m_concert;
};

} // namespace kodi
