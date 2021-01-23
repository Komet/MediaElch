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

private:
    KodiVersion m_version;
};

} // namespace kodi
} // namespace mediaelch
