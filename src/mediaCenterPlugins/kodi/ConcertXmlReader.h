#ifndef KODI_CONCERTXMLREADER_H
#define KODI_CONCERTXMLREADER_H

#include <QDomDocument>
#include <QString>

class Concert;

namespace Kodi {

class ConcertXmlReader
{
public:
    ConcertXmlReader(Concert &concert);
    void parseNfoDom(QDomDocument domDoc);

private:
    Concert &m_concert;
};

} // namespace Kodi

#endif // KODI_CONCERTXMLREADER_H
