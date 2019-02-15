#pragma once

#include <QDomDocument>
#include <QString>

class Concert;

namespace Kodi {

class ConcertXmlReader
{
public:
    ConcertXmlReader(Concert& concert);
    void parseNfoDom(QDomDocument domDoc);

private:
    Concert& m_concert;
};

} // namespace Kodi
