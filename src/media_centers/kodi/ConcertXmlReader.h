#pragma once

#include <QDomDocument>
#include <QString>

class Concert;

namespace kodi {

class ConcertXmlReader
{
public:
    ConcertXmlReader(Concert& concert);
    void parseNfoDom(QDomDocument domDoc);

private:
    Concert& m_concert;
};

} // namespace kodi
