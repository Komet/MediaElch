#pragma once

#include <QXmlStreamReader>

class Concert;
class StreamDetails;

namespace mediaelch {
namespace kodi {

class ConcertXmlReader
{
public:
    explicit ConcertXmlReader(Concert& concert);
    void parse(QXmlStreamReader& reader);

private:
    void parseConcert(QXmlStreamReader& reader);
    void parseUniqueId(QXmlStreamReader& reader);
    void parseRatings(QXmlStreamReader& reader);
    void parseFanart(QXmlStreamReader& reader);
    void parsePoster(QXmlStreamReader& reader);

private:
    Concert& m_concert;
};

} // namespace kodi
} // namespace mediaelch
