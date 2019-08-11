#pragma once

#include "media_centers/kodi/ConcertXmlWriter.h"

#include <QByteArray>

class Concert;

namespace mediaelch {
namespace kodi {

class ConcertXmlWriterV17 : public ConcertXmlWriter
{
public:
    ConcertXmlWriterV17(Concert& movie);
    QByteArray getConcertXml() override;

private:
    Concert& m_concert;
};

} // namespace kodi
} // namespace mediaelch
