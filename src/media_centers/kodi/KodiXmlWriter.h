#pragma once

#include "media_centers/KodiVersion.h"

#include <QByteArray>
#include <QXmlStreamWriter>

namespace mediaelch {
namespace kodi {

class KodiXmlWriter
{
public:
    KodiXmlWriter(KodiVersion version) : m_version{std::move(version)} {};
    virtual ~KodiXmlWriter() = default;

    const KodiVersion& version() const { return m_version; }
    void addMediaelchGeneratorTag(QXmlStreamWriter& xml);

    bool writeThumbUrlsToNfo() const;
    void setWriteThumbUrlsToNfo(bool writeThumbUrlsToNfo);

private:
    KodiVersion m_version;
    bool m_writeThumbUrlsToNfo = true;
};

} // namespace kodi
} // namespace mediaelch
