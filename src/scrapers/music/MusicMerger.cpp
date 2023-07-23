#include "scrapers/music/MusicMerger.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "log/Log.h"

namespace mediaelch {
namespace scraper {

namespace {

// TODO: This is not the same as MovieMerger: Here, we merge only if source has a value.
void copyDetailToArtist(Artist& target, const Artist& source, MusicScraperInfo detail)
{
    if (source.mbId().isValid()) {
        target.setMbId(source.mbId());
    }
    if (source.allMusicId().isValid()) {
        target.setAllMusicId(source.allMusicId());
    }

    switch (detail) {
    case MusicScraperInfo::Invalid: {
        qCCritical(generic) << "[ArtistMerger] Cannot copy details 'invalid'";
        break;
    }
    case MusicScraperInfo::Name:
        if (!source.name().isEmpty()) {
            target.setName(source.name());
        }
        break;
    case MusicScraperInfo::YearsActive:
        if (!source.yearsActive().isEmpty()) {
            target.setYearsActive(source.yearsActive());
        }
        break;
    case MusicScraperInfo::Formed:
        if (!source.formed().isEmpty()) {
            target.setFormed(source.formed());
        }
        break;
    case MusicScraperInfo::Born:
        if (!source.born().isEmpty()) {
            target.setBorn(source.born());
        }
        break;
    case MusicScraperInfo::Died:
        if (!source.died().isEmpty()) {
            target.setDied(source.died());
        }
        break;
    case MusicScraperInfo::Disbanded:
        if (!source.disbanded().isEmpty()) {
            target.setDisbanded(source.disbanded());
        }
        break;
    case MusicScraperInfo::Biography:
        if (!source.biography().isEmpty()) {
            target.setBiography(source.biography());
        }
        break;
    case MusicScraperInfo::Genres:
        if (!source.genres().isEmpty()) {
            target.setGenres(source.genres());
        }
        break;
    case MusicScraperInfo::Styles:
        if (!source.styles().isEmpty()) {
            target.setStyles(source.styles());
        }
        break;
    case MusicScraperInfo::Moods:
        if (!source.moods().isEmpty()) {
            target.setMoods(source.moods());
        }
        break;
    case MusicScraperInfo::Discography:
        if (!source.discographyAlbums().isEmpty()) {
            target.setDiscographyAlbums(source.discographyAlbums());
        }
        break;
    default: break; // TODO: Check for other fields
    }
}

void copyDetailToAlbum(Album& target, const Album& source, MusicScraperInfo detail)
{
    if (source.mbAlbumId().isValid()) {
        target.setMbAlbumId(source.mbAlbumId());
    }
    if (source.mbReleaseGroupId().isValid()) {
        target.setMbReleaseGroupId(source.mbReleaseGroupId());
    }
    if (source.allMusicId().isValid()) {
        target.setAllMusicId(source.allMusicId());
    }

    switch (detail) {
    case MusicScraperInfo::Invalid: {
        qCCritical(generic) << "[AlbumMerger] Cannot copy details 'invalid'";
        break;
    }
    case MusicScraperInfo::Title:
        if (!source.title().isEmpty()) {
            target.setTitle(source.title());
        }
        break;
    case MusicScraperInfo::Artist:
        if (!source.artist().isEmpty()) {
            target.setArtist(source.artist());
        }
        break;
    case MusicScraperInfo::Review:
        if (!source.review().isEmpty()) {
            target.setReview(source.review());
        }
        break;
    case MusicScraperInfo::ReleaseDate:
        if (!source.releaseDate().isEmpty()) {
            target.setReleaseDate(source.releaseDate());
        }
        break;
    case MusicScraperInfo::Label:
        if (!source.label().isEmpty()) {
            target.setLabel(source.label());
        }
        break;
    case MusicScraperInfo::Rating:
        if (source.rating() != 0) {
            target.setRating(source.rating());
        }
        break;
    case MusicScraperInfo::Year:
        if (source.year() != 0) {
            target.setYear(source.year());
        }
        break;
    case MusicScraperInfo::Genres:
        if (!source.genres().isEmpty()) {
            target.setGenres(source.genres());
        }
        break;
    case MusicScraperInfo::Styles:
        if (!source.styles().isEmpty()) {
            target.setStyles(source.styles());
        }
        break;
    case MusicScraperInfo::Moods:
        if (!source.moods().isEmpty()) {
            target.setMoods(source.moods());
        }
        break;
    default: break; // TODO: Check for other fields
    }
}


// TODO: This is not the same as MovieMerger: Here, we merge only if source has a value.
void copyDetailToArtistIfEmpty(Artist& target, const Artist& source, MusicScraperInfo detail)
{
    if (!target.mbId().isValid() && source.mbId().isValid()) {
        target.setMbId(source.mbId());
    }
    if (!target.allMusicId().isValid() && source.allMusicId().isValid()) {
        target.setAllMusicId(source.allMusicId());
    }

    switch (detail) {
    case MusicScraperInfo::Invalid: {
        qCCritical(generic) << "[ArtistMerger] Cannot copy details 'invalid'";
        break;
    }
    case MusicScraperInfo::Name:
        if (target.name().isEmpty() && !source.name().isEmpty()) {
            target.setName(source.name());
        }
        break;
    case MusicScraperInfo::YearsActive:
        if (target.yearsActive().isEmpty() && !source.yearsActive().isEmpty()) {
            target.setYearsActive(source.yearsActive());
        }
        break;
    case MusicScraperInfo::Formed:
        if (target.formed().isEmpty() && !source.formed().isEmpty()) {
            target.setFormed(source.formed());
        }
        break;
    case MusicScraperInfo::Born:
        if (target.born().isEmpty() && !source.born().isEmpty()) {
            target.setBorn(source.born());
        }
        break;
    case MusicScraperInfo::Died:
        if (target.died().isEmpty() && !source.died().isEmpty()) {
            target.setDied(source.died());
        }
        break;
    case MusicScraperInfo::Disbanded:
        if (target.disbanded().isEmpty() && !source.disbanded().isEmpty()) {
            target.setDisbanded(source.disbanded());
        }
        break;
    case MusicScraperInfo::Biography:
        if (target.biography().isEmpty() && !source.biography().isEmpty()) {
            target.setBiography(source.biography());
        }
        break;
    case MusicScraperInfo::Genres:
        if (target.genres().isEmpty() && !source.genres().isEmpty()) {
            target.setGenres(source.genres());
        }
        break;
    case MusicScraperInfo::Styles:
        if (target.styles().isEmpty() && !source.styles().isEmpty()) {
            target.setStyles(source.styles());
        }
        break;
    case MusicScraperInfo::Moods:
        if (target.moods().isEmpty() && !source.moods().isEmpty()) {
            target.setMoods(source.moods());
        }
        break;
    case MusicScraperInfo::Discography:
        if (target.discographyAlbums().isEmpty() && !source.discographyAlbums().isEmpty()) {
            target.setDiscographyAlbums(source.discographyAlbums());
        }
        break;
    default: break; // TODO: Check for other fields
    }
}

void copyDetailToAlbumIfEmpty(Album& target, const Album& source, MusicScraperInfo detail)
{
    if (!target.mbAlbumId().isValid() && source.mbAlbumId().isValid()) {
        target.setMbAlbumId(source.mbAlbumId());
    }
    if (!target.mbReleaseGroupId().isValid() && source.mbReleaseGroupId().isValid()) {
        target.setMbReleaseGroupId(source.mbReleaseGroupId());
    }
    if (!target.allMusicId().isValid() && source.allMusicId().isValid()) {
        target.setAllMusicId(source.allMusicId());
    }

    switch (detail) {
    case MusicScraperInfo::Invalid: {
        qCCritical(generic) << "[AlbumMerger] Cannot copy details 'invalid'";
        break;
    }
    case MusicScraperInfo::Title:
        if (target.title().isEmpty() && !source.title().isEmpty()) {
            target.setTitle(source.title());
        }
        break;
    case MusicScraperInfo::Artist:
        if (target.artist().isEmpty() && !source.artist().isEmpty()) {
            target.setArtist(source.artist());
        }
        break;
    case MusicScraperInfo::Review:
        if (target.review().isEmpty() && !source.review().isEmpty()) {
            target.setReview(source.review());
        }
        break;
    case MusicScraperInfo::ReleaseDate:
        if (target.releaseDate().isEmpty() && !source.releaseDate().isEmpty()) {
            target.setReleaseDate(source.releaseDate());
        }
        break;
    case MusicScraperInfo::Label:
        if (target.label().isEmpty() && !source.label().isEmpty()) {
            target.setLabel(source.label());
        }
        break;
    case MusicScraperInfo::Rating:
        // Use some epsilon; should not be necessary, but just in case
        if (target.rating() >= -0.01 && target.rating() <= 0.01 && source.rating() != 0) {
            target.setRating(source.rating());
        }
        break;
    case MusicScraperInfo::Year:
        if (target.year() == 0 && source.year() != 0) {
            target.setYear(source.year());
        }
        break;
    case MusicScraperInfo::Genres:
        if (target.genres().isEmpty() && !source.genres().isEmpty()) {
            target.setGenres(source.genres());
        }
        break;
    case MusicScraperInfo::Styles:
        if (target.styles().isEmpty() && !source.styles().isEmpty()) {
            target.setStyles(source.styles());
        }
        break;
    case MusicScraperInfo::Moods:
        if (target.moods().isEmpty() && !source.moods().isEmpty()) {
            target.setMoods(source.moods());
        }
        break;
    default: break; // TODO: Check for other fields
    }
}

} // namespace

void copyDetailsToArtist(Artist& target, const Artist& source, const QSet<MusicScraperInfo>& details)
{
    for (MusicScraperInfo detail : details) {
        copyDetailToArtist(target, source, detail);
    }
}

void copyDetailsToAlbum(Album& target, const Album& source, const QSet<MusicScraperInfo>& details)
{
    for (MusicScraperInfo detail : details) {
        copyDetailToAlbum(target, source, detail);
    }
}

void copyDetailsToArtistIfEmpty(Artist& target, const Artist& source, const QSet<MusicScraperInfo>& details)
{
    for (MusicScraperInfo detail : details) {
        copyDetailToArtistIfEmpty(target, source, detail);
    }
}

void copyDetailsToAlbumIfEmpty(Album& target, const Album& source, const QSet<MusicScraperInfo>& details)
{
    for (MusicScraperInfo detail : details) {
        copyDetailToAlbumIfEmpty(target, source, detail);
    }
}

} // namespace scraper
} // namespace mediaelch
