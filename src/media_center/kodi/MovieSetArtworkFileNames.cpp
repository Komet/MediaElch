#include "media_center/kodi/MovieSetArtworkFileNames.h"

namespace mediaelch {
namespace kodi {

QStringList
legacyMovieSetFileNames(DataFileType type, MovieSetArtworkType layout, const QStringList& configuredFileNames)
{
    QStringList legacyNames;

    // Only the poster ever had a second spelling.  `fanart.jpg` / `<setName>-fanart.jpg` have been
    // written unchanged for as long as either layout has existed, so MovieSetBackdrop deliberately
    // gets nothing here -- inventing a legacy name for every type would only add stat() calls that
    // can never succeed.
    if (type == DataFileType::MovieSetPoster) {
        switch (layout) {
        case MovieSetArtworkType::SeparateArtworkFolder: legacyNames << QStringLiteral("folder.jpg"); break;
        case MovieSetArtworkType::ArtworkNextToMovies: legacyNames << QStringLiteral("<setName>-folder.jpg"); break;
        }
    }

    QStringList result;
    for (const QString& legacyName : asConst(legacyNames)) {
        if (!configuredFileNames.contains(legacyName, Qt::CaseInsensitive)) {
            result << legacyName;
        }
    }
    return result;
}

QVector<MovieSetArtworkCandidate>
movieSetArtworkCandidates(DataFileType type, MovieSetArtworkType layout, const QStringList& configuredFileNames)
{
    // Only the movie set information folder has a folder component, so it is the only layout where
    // the pre-legalisation spelling of that folder can exist on disk.
    const bool hasFolderComponent = layout == MovieSetArtworkType::SeparateArtworkFolder;

    const QStringList legacyFileNames = legacyMovieSetFileNames(type, layout, configuredFileNames);

    QVector<MovieSetArtworkCandidate> candidates;
    const auto appendGroup = [&candidates, hasFolderComponent](const QStringList& fileNames) {
        for (const QString& fileName : fileNames) {
            candidates.append({fileName, LegalisePath::Yes});
        }
        if (hasFolderComponent) {
            for (const QString& fileName : fileNames) {
                candidates.append({fileName, LegalisePath::No});
            }
        }
    };

    appendGroup(configuredFileNames);
    appendGroup(legacyFileNames);

    return candidates;
}

} // namespace kodi
} // namespace mediaelch
