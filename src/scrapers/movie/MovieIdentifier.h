#pragma once

#include "data/ImdbId.h"
#include "data/TmdbId.h"

#include <QDebug>
#include <QString>

namespace mediaelch {
namespace scraper {

/// \brief This class uniquely identifies a movie for scrapers.
class MovieIdentifier
{
public:
    MovieIdentifier() = default;
    explicit MovieIdentifier(QString movieIdentifier) : id{std::move(movieIdentifier)} {}
    explicit MovieIdentifier(TmdbId movieIdentifier) : id{movieIdentifier.toString()} {}
    explicit MovieIdentifier(ImdbId movieIdentifier) : id{movieIdentifier.toString()} {}
    ~MovieIdentifier() = default;

    const QString& str() const { return id; }

private:
    QString id;
};

QDebug operator<<(QDebug debug, const MovieIdentifier& id);

} // namespace scraper
} // namespace mediaelch
