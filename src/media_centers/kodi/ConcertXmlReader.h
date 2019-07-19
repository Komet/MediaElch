#pragma once

#include <QDomDocument>
#include <QString>

class Concert;

namespace mediaelch {
namespace kodi {

class ConcertXmlReader
{
public:
    explicit ConcertXmlReader(Concert& concert);
    void parseNfoDom(QDomDocument domDoc);

private:
    Concert& m_concert;
};

} // namespace kodi
} // namespace mediaelch
