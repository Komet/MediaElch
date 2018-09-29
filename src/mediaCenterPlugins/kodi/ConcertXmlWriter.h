#ifndef KODI_CONCERTXMLWRITER_H
#define KODI_CONCERTXMLWRITER_H

#include <QByteArray>
#include <QDomElement>
#include <QString>

class Concert;

namespace Kodi {

class ConcertXmlWriter
{
public:
    ConcertXmlWriter(Concert &concert);
    QByteArray getConcertXml();

private:
    Concert &m_concert;
};

} // namespace Kodi

#endif // KODI_CONCERTXMLWRITER_H
