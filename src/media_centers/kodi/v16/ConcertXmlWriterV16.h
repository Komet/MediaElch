#pragma once

#include "media_centers/kodi/ConcertXmlWriter.h"

#include <QByteArray>

class Concert;

namespace mediaelch {
namespace kodi {

class ConcertXmlWriterV16 : public ConcertXmlWriter
{
public:
    ConcertXmlWriterV16(Concert& concert);
    QByteArray getConcertXml() override;

private:
    Concert& m_concert;
};

} // namespace kodi
} // namespace mediaelch
