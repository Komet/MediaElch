#pragma once

#include "media_centers/kodi/TvShowXmlWriter.h"

#include <QByteArray>
#include <QDomElement>
#include <QString>

class TvShow;

namespace mediaelch {
namespace kodi {

class TvShowXmlWriterV16 : public TvShowXmlWriter
{
public:
    TvShowXmlWriterV16(TvShow& tvShow);
    QByteArray getTvShowXml() override;

private:
    TvShow& m_show;
};

} // namespace kodi
} // namespace mediaelch
