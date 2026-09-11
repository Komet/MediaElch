#pragma once

#include "globals/Globals.h"
#include "utils/Meta.h"

#include <QString>
#include <QStringList>
#include <QVector>

namespace mediaelch {
namespace kodi {

/// \brief Whether to name the movie set's folder the way Kodi expects it to be.
///
/// Only meaningful for MovieSetArtworkType::SeparateArtworkFolder; the "artwork next to movies"
/// layout has no folder component that could be legalised.
enum class LegalisePath
{
    Yes,
    No
};

/// \brief One file name to look for when reading a movie set's artwork, plus how to spell the
///        movie set information folder while doing so.
struct MovieSetArtworkCandidate
{
    QString fileName;
    LegalisePath legalise = LegalisePath::Yes;
};

/// \brief File names that older MediaElch versions used for movie set artwork and that are no
///        longer written, minus any that are still configured.
///
/// MediaElch used to spell the movie set poster `folder.jpg` (in the movie set information folder)
/// and `<setName>-folder.jpg` (next to the movies).  Both spellings are still on users' disks, so
/// they have to stay readable.  Fanart was always `fanart.jpg` / `<setName>-fanart.jpg`, so there
/// is no legacy spelling for MovieSetBackdrop and the list is empty for it.
///
/// A name that is also in \p configuredFileNames is dropped (case-insensitively, because the
/// file systems that matter here are too), so the same path is never stat()ed twice.  That is
/// not the case of an untouched installation: `folder.jpg` has never been a shipped default --
/// Settings.cpp ships `<setName>-poster.jpg` -- so it is in a user's settings only if they
/// picked a movie set artwork layout under a MediaElch that still suggested it.
///
/// \param type              MovieSetPoster or MovieSetBackdrop; anything else yields an empty list.
/// \param layout            Which of the two movie set artwork layouts is configured.
/// \param configuredFileNames  The file names currently configured for \p type.
ELCH_NODISCARD QStringList legacyMovieSetFileNames(DataFileType type,
    MovieSetArtworkType layout,
    const QStringList& configuredFileNames);

/// \brief Every place a movie set's artwork may be read from, in the order it must be tried.
///
/// Two orthogonal axes, ordered so that the configured spelling always wins over a legacy one:
///   1. configured names, legalised folder
///   2. configured names, verbatim folder   (movie set information folder only)
///   3. legacy names, legalised folder
///   4. legacy names, verbatim folder       (movie set information folder only)
///
/// Name before folder, not folder before name: a user holding both the new `poster.jpg` and a
/// stale `folder.jpg` must see the new one, whichever folder spelling each of them sits under.
ELCH_NODISCARD QVector<MovieSetArtworkCandidate>
movieSetArtworkCandidates(DataFileType type, MovieSetArtworkType layout, const QStringList& configuredFileNames);

} // namespace kodi
} // namespace mediaelch
