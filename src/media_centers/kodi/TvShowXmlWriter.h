#pragma once

#include "media_centers/kodi/KodiXmlWriter.h"

#include <QByteArray>

class TvShow;

namespace mediaelch {
namespace kodi {

class TvShowXmlWriter : public KodiXmlWriter
{
public:
    TvShowXmlWriter(KodiVersion version) : KodiXmlWriter(std::move(version)) {}
    virtual ~TvShowXmlWriter() = default;
    virtual QByteArray getTvShowXml(bool testMode = false) = 0;
};

class TvShowXmlWriterGeneric : public TvShowXmlWriter
{
public:
    TvShowXmlWriterGeneric(KodiVersion version, TvShow& tvShow);
    QByteArray getTvShowXml(bool testMode = false) override;

private:
    TvShow& m_show;
};

} // namespace kodi
} // namespace mediaelch
