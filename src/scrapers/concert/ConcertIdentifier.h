#pragma once

#include "data/ImdbId.h"
#include "data/TmdbId.h"

#include <QDebug>
#include <QString>

namespace mediaelch {
namespace scraper {

/// \brief This class uniquely identifies a concert for scrapers.
class ConcertIdentifier
{
public:
    ConcertIdentifier() = default;
    explicit ConcertIdentifier(QString concertIdentifier) : id{std::move(concertIdentifier)} {}
    explicit ConcertIdentifier(TmdbId concertIdentifier) : id{concertIdentifier.toString()} {}
    explicit ConcertIdentifier(ImdbId concertIdentifier) : id{concertIdentifier.toString()} {}
    ~ConcertIdentifier() = default;

    const QString& str() const { return id; }

private:
    QString id;
};

QDebug operator<<(QDebug debug, const ConcertIdentifier& id);

} // namespace scraper
} // namespace mediaelch
