#pragma once

#include "data/ImdbId.h"
#include "data/TmdbId.h"
#include "tv_shows/TvDbId.h"
#include "tv_shows/TvMazeId.h"

#include <QDebug>
#include <QString>

namespace mediaelch {
namespace scraper {

/// \brief This class uniquely identifies a TV show for scrapers.
class ShowIdentifier
{
public:
    ShowIdentifier() = default;
    explicit ShowIdentifier(QString _showIdentifier) : id{std::move(_showIdentifier)} {}
    explicit ShowIdentifier(TmdbId _showIdentifier) : id{_showIdentifier.toString()} {}
    explicit ShowIdentifier(TvDbId _showIdentifier) : id{_showIdentifier.toString()} {}
    explicit ShowIdentifier(ImdbId _showIdentifier) : id{_showIdentifier.toString()} {}
    explicit ShowIdentifier(TvMazeId _showIdentifier) : id{_showIdentifier.toString()} {}
    ~ShowIdentifier() = default;

    const QString& str() const { return id; }

private:
    QString id;
};

QDebug operator<<(QDebug debug, const ShowIdentifier& id);

} // namespace scraper
} // namespace mediaelch
