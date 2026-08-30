#pragma once

#include "data/TmdbId.h"
#include "utils/Meta.h"

#include <QString>

/// Represents a movie collection (aka. set).
struct MovieSet
{
    /// A collection's TmdbId, e.g. 1241 for Harry Potter.
    /// Used for getting data from TMDB, e.g.
    /// themoviedb.org/movie/1241 which redirects to
    /// themoviedb.org/collection/1241-harry-potter-collection
    TmdbId tmdbId{TmdbId::NoId};
    QString name;
    QString overview;

    /// \brief Returns this collection under a new name, keeping overview and id.
    /// \details Only for renaming a collection.  Moving a movie to a _different_
    ///          collection must not use this: overview and id describe the old one.
    ELCH_NODISCARD MovieSet renamedTo(QString newName) const;
};
