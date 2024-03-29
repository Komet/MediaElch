#pragma once

#include "media_center/kodi/KodiXmlWriter.h"

#include <QByteArray>

class Concert;

namespace mediaelch {
namespace kodi {

class ConcertXmlWriter : public KodiXmlWriter
{
public:
    ConcertXmlWriter(KodiVersion version) : KodiXmlWriter(std::move(version)) {}
    virtual ~ConcertXmlWriter() = default;
    virtual QByteArray getConcertXml(bool testMode = false) = 0;
};

class ConcertXmlWriterGeneric : public ConcertXmlWriter
{
public:
    ConcertXmlWriterGeneric(KodiVersion version, const Concert& concert);
    QByteArray getConcertXml(bool testMode = false) override;

private:
    const Concert& m_concert;
};

} // namespace kodi
} // namespace mediaelch
