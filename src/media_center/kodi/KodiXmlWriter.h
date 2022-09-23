#pragma once

#include "data/Actor.h"
#include "media_center/KodiVersion.h"

#include <QByteArray>
#include <QVector>
#include <QXmlStreamWriter>

class Ratings;

namespace mediaelch {
namespace kodi {

class KodiXmlWriter
{
public:
    KodiXmlWriter(KodiVersion version) : m_version{std::move(version)} {};
    virtual ~KodiXmlWriter() = default;

    const KodiVersion& version() const { return m_version; }
    void addMediaelchGeneratorTag(QXmlStreamWriter& xml);

    void writeActors(QXmlStreamWriter& xml, const Actors& actors) const;

    bool writeThumbUrlsToNfo() const;
    void setWriteThumbUrlsToNfo(bool writeThumbUrlsToNfo);

private:
    KodiVersion m_version;
    bool m_writeThumbUrlsToNfo = true;
};

void writeRatings(QXmlStreamWriter& xml, const Ratings& ratings);

} // namespace kodi
} // namespace mediaelch
