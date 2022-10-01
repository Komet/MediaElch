#include "test/helpers/reference_file.h"

#include "data/concert/Concert.h"
#include "data/movie/Movie.h"
#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "data/tv_show/TvShowEpisode.h"
#include "media/StreamDetails.h"

#include <QTextStream>
#include <algorithm>

// TODO: Move exporter, etc.
//       Probably streamline, nicer, etc.

using namespace mediaelch;

namespace {

/// Write a string to out, limiting its size at around 80 characters per line.
static void writeToReference(QTextStream& out, const QString& key, const QString& value)
{
    out << key << ":";
    const int paragraphLength = 80;
    if (value.length() > paragraphLength) {
        out << "\n";
        const int length = value.length();
        for (int i = 0; i < length; i += paragraphLength) {
            out << "    " << value.midRef(i, std::min(paragraphLength, length - i)) << "\n";
        }

    } else {
        out << " " << value << "\n";
    }
}

// For all types that have a toString() method.
template<class T>
static auto writeToReference(QTextStream& out, const QString& key, const T& value) -> decltype(value.toString(), void())
{
    writeToReference(out, key, value.toString());
}

static void writeToReference(QTextStream& out, const QString& key, const QStringList& value)
{
    out << key << ": (N=" << value.size() << ")\n";
    for (const auto& item : value) {
        out << "  - " << item << "\n";
    }
}
static void writeToReference(QTextStream& out, const QString& key, int value)
{
    out << key << ": " << value << "\n";
}
static void writeToReference(QTextStream& out, const QString& key, const QDate& value)
{
    out << key << ": " << value.toString(Qt::DateFormat::ISODate) << "\n";
}
static void writeToReference(QTextStream& out, const QString& key, const QDateTime& value)
{
    out << key << ": " << value.toString(Qt::DateFormat::ISODate) << "\n";
}
static void writeToReference(QTextStream& out, const QString& key, const QTime& value)
{
    out << key << ": " << value.toString(Qt::DateFormat::ISODate) << "\n";
}
static void writeToReference(QTextStream& out, const QString& key, const std::chrono::minutes& value)
{
    out << key << ": " << value.count() << "min\n";
}
static void writeToReference(QTextStream& out, const QString& key, const QSize& value)
{
    out << key << ": h=" << value.height() << " w=" << value.width() << "\n";
}
static void writeToReference(QTextStream& out, const QString& key, const SeasonNumber& value)
{
    out << key << ": SeasonNumber=" << value.toPaddedString() << "\n";
}
static void writeToReference(QTextStream& out, const QString& key, const EpisodeNumber& value)
{
    out << key << ": EpisodeNumber=" << value.toPaddedString() << "\n";
}
static void writeToReference(QTextStream& out, const QString& key, const DiscType& value)
{
    out << key << ": ";
    switch (value) {
    case DiscType::Single: out << "Single"; break;
    case DiscType::BluRay: out << "BluRay"; break;
    case DiscType::Dvd: out << "Dvd"; break;
    };
    out << "\n";
}
static void writeToReference(QTextStream& out, const QString& key, const Ratings& value)
{
    out << key << " (N=" << value.size() << ")\n";
    for (const auto& rating : value) {
        out << "  "                                  //
            << "source=" << rating.source << " | "   //
            << "rating=" << rating.rating << " | "   //
            << "votes=" << rating.voteCount << " | " //
            << "min=" << rating.minRating << " | "   //
            << "max=" << rating.maxRating            //
            << "\n";
    }
}

static void writeToReference(QTextStream& out, const QString& key, const QVector<Poster>& value)
{
    out << key << ": (N=" << value.count() << ")\n";
    for (const Poster& poster : value) {
        writeToReference(out, "  - id", poster.id);
        writeToReference(out, "    originalUrl", poster.originalUrl);
        writeToReference(out, "    thumbUrl", poster.thumbUrl);
        writeToReference(out, "    originalSize", poster.originalSize);
        writeToReference(out, "    language", poster.language);
        writeToReference(out, "    hint", poster.hint);
        writeToReference(out, "    aspect", poster.aspect);
        writeToReference(out, "    season", poster.season);
    }
}

static void writeToReference(QTextStream& out, const QString& key, const Actors& value)
{
    out << key << ": (N=" << value.size() << ")\n";
    for (const Actor* actor : value) {
        writeToReference(out, " - id", actor->id);
        writeToReference(out, "   name", actor->name);
        writeToReference(out, "   role", actor->role);
        writeToReference(out, "   thumb", actor->thumb);
        writeToReference(out, "   order", actor->order);
        writeToReference(out, "   imageHasChanged", actor->imageHasChanged);
    }
}

static void writeToReference(QTextStream& out, const QString& key, const mediaelch::FileList& value)
{
    writeToReference(out, key, value.toStringList());
}
static void writeToReference(QTextStream& out, const QString& key, const mediaelch::ResumeTime& value)
{
    out << key << ": position=" << value.position << " / total=" << value.total << "\n";
}

static void writeToReference(QTextStream& out, const QString& key, const StreamDetails* value)
{
    out << key << ": ";
    if (value == nullptr) {
        out << "<nullptr>";
    } else if (value->hasLoaded()) {
        out << "<loaded>";
    } else {
        out << "<not loaded>";
    }
    out << "\n";
}

static void writeToReference(QTextStream& out, const QString& key, const QVector<Subtitle*>& value)
{
    out << key << ": (N=" << value.size() << ")\n";
    for (const Subtitle* subtitle : value) {
        writeToReference(out, "  - language=", subtitle->language());
        writeToReference(out, "    forced=", subtitle->forced());
        writeToReference(out, "    files=", subtitle->files().join(", "));
    }
}

static void writeToReference(QTextStream& out, const QString& key, const QVector<DiscographyAlbum>& value)
{
    out << key << ": (N=" << value.size() << ")\n";
    for (const DiscographyAlbum& album : value) {
        writeToReference(out, "  - title=", album.title);
        writeToReference(out, "    year=", album.year);
    }
}


class AlbumTestExporter final : public Album::Exporter
{
public:
    AlbumTestExporter(QTextStream& out) : m_out{out} {}

    void startExport() override { m_out << "Album Reference File\n----------------------\n\n"; }
    void endExport() override { m_out.flush(); }

    void exportDatabaseId(const mediaelch::DatabaseId& databaseId) override
    {
        // ignored, because it is database dependent
        Q_UNUSED(databaseId);
    }
    void exporTmbAlbumId(const MusicBrainzId& mbAlbumId) override { writeToReference(m_out, "mbAlbumId", mbAlbumId); }
    void exporTmbReleaseGroupId(const MusicBrainzId& mbReleaseGroupId) override
    {
        writeToReference(m_out, "mbReleaseGroupId", mbReleaseGroupId);
    }
    void exportAllMusicId(const AllMusicId& allMusicId) override { writeToReference(m_out, "allMusicId", allMusicId); }

    void exportTitle(const QString& title) override { writeToReference(m_out, "title", title); }
    void exportArtist(const QString& artist) override { writeToReference(m_out, "artist", artist); }
    void exportGenres(const QStringList& genres) override { writeToReference(m_out, "genres", genres); }
    void exportStyles(const QStringList& styles) override { writeToReference(m_out, "styles", styles); }
    void exportMoods(const QStringList& moods) override { writeToReference(m_out, "moods", moods); }
    void exportReleaseDate(const QString& releaseDate) override { writeToReference(m_out, "releaseDate", releaseDate); }
    void exportLabel(const QString& label) override { writeToReference(m_out, "label", label); }
    void exportRating(const qreal& rating) override { writeToReference(m_out, "rating", rating); }
    void exportReview(const QString& review) override { writeToReference(m_out, "review", review); }
    void exportYear(int year) override { writeToReference(m_out, "year", year); }
    void exportImages(const QMap<ImageType, QVector<Poster>>& images) override
    {
        auto i = images.constBegin();
        while (i != images.constEnd()) {
            writeToReference(m_out, QStringLiteral("images_%").arg(int(i.key())), i.value());
        }
    }
    void exportPath(const mediaelch::DirectoryPath& path) override
    {
        // ignored because path dependent
        Q_UNUSED(path);
    }

private:
    QTextStream& m_out;
};


class ArtistTestExporter final : public Artist::Exporter
{
public:
    ArtistTestExporter(QTextStream& out) : m_out{out} {}

    void startExport() override { m_out << "Artist Reference File\n----------------------\n\n"; }
    void endExport() override { m_out.flush(); }

    void exportDatabaseId(const mediaelch::DatabaseId& databaseId) override
    {
        // ignored, because it is database dependent
        Q_UNUSED(databaseId);
    }
    void exporTmbId(const MusicBrainzId& mbId) override { writeToReference(m_out, "mbId", mbId); }
    void exportAllMusicId(const AllMusicId& allMusicId) override { writeToReference(m_out, "allMusicId", allMusicId); }

    void exportName(const QString& name) override { writeToReference(m_out, "name", name); }
    void exportBiography(const QString& biography) override { writeToReference(m_out, "biography", biography); }
    void exportDiscography(const QVector<DiscographyAlbum>& discography) override
    {
        writeToReference(m_out, "discography", discography);
    }
    void exportGenres(const QStringList& genres) override { writeToReference(m_out, "genres", genres); }
    void exportStyles(const QStringList& styles) override { writeToReference(m_out, "styles", styles); }
    void exportMoods(const QStringList& moods) override { writeToReference(m_out, "moods", moods); }
    void exportYearsActive(const QString& yearsActive) override { writeToReference(m_out, "yearsActive", yearsActive); }
    void exportFormed(const QString& formed) override { writeToReference(m_out, "formed", formed); }
    void exportBorn(const QString& born) override { writeToReference(m_out, "born", born); }
    void exportDied(const QString& died) override { writeToReference(m_out, "died", died); }
    void exportDisbanded(const QString& disbanded) override { writeToReference(m_out, "disbanded", disbanded); }
    void exportImages(const QMap<ImageType, QVector<Poster>>& images) override
    {
        auto i = images.constBegin();
        while (i != images.constEnd()) {
            writeToReference(m_out, QStringLiteral("images_%").arg(int(i.key())), i.value());
        }
    }

    void exportExtraFanarts(const QStringList& extraFanarts) override
    {
        writeToReference(m_out, "extraFanarts", extraFanarts);
    }
    void exportPath(const mediaelch::DirectoryPath& path) override
    {
        // ignored because path dependent
        Q_UNUSED(path);
    }

private:
    QTextStream& m_out;
};

class ConcertTestExporter final : public ConcertData::Exporter
{
public:
    ConcertTestExporter(QTextStream& out) : m_out{out} {}

    void startExport() override { m_out << "Concert Reference File\n----------------------\n\n"; }
    void endExport() override { m_out.flush(); }

    void exportDatabaseId(DatabaseId databaseId) override
    {
        // ignored, because it is database dependent
        Q_UNUSED(databaseId);
    }
    void exportMediaCenterId(int mediaCenterId) override { writeToReference(m_out, "mediaCenterId", mediaCenterId); }
    void exportTmdbId(const TmdbId& tmdbId) override { writeToReference(m_out, "tmdbId", tmdbId); }
    void exportImdbId(const ImdbId& imdbId) override { writeToReference(m_out, "imdbId", imdbId); }
    void exportTitle(const QString& title) override { writeToReference(m_out, "title", title); }
    void exportOriginalTitle(const QString& originalTitle) override
    {
        writeToReference(m_out, "originalTitle", originalTitle);
    }
    void exportArtist(const QString& artist) override { writeToReference(m_out, "artist", artist); }
    void exportAlbum(const QString& album) override { writeToReference(m_out, "album", album); }
    void exportOverview(const QString& overview) override { writeToReference(m_out, "overview", overview); }
    void exportRatings(const Ratings& ratings) override { writeToReference(m_out, "ratings", ratings); }
    void exportUserRating(double userRating) override { writeToReference(m_out, "userRating", userRating); }
    void exportReleaseDate(const QDate& releaseDate) override { writeToReference(m_out, "releaseDate", releaseDate); }
    void exportTagline(const QString& tagline) override { writeToReference(m_out, "tagline", tagline); }
    void exportRuntime(const std::chrono::minutes& runtime) override { writeToReference(m_out, "runtime", runtime); }
    void exportCertification(const Certification& certification) override
    {
        writeToReference(m_out, "certification", certification);
    }
    void exportGenres(const QStringList& genres) override { writeToReference(m_out, "genres", genres); }
    void exportTags(const QStringList& tags) override { writeToReference(m_out, "tags", tags); }
    void exportTrailer(const QUrl& trailer) override { writeToReference(m_out, "trailer", trailer); }
    void exportPlaycount(const int& playcount) override { writeToReference(m_out, "playcount", playcount); }
    void exportLastPlayed(const QDateTime& lastPlayed) override { writeToReference(m_out, "lastPlayed", lastPlayed); }
    void exportLastModified(const QDateTime& lastModified) override
    {
        writeToReference(m_out, "lastModified", lastModified);
    }
    void exportPosters(const QVector<Poster>& posters) override { writeToReference(m_out, "posters", posters); }
    void exportBackdrops(const QVector<Poster>& backdrops) override { writeToReference(m_out, "backdrops", backdrops); }
    void exportExtraFanarts(const QStringList& extraFanarts) override
    {
        writeToReference(m_out, "extraFanarts", extraFanarts);
    }
    void exportStreamDetails(const StreamDetails* streamDetails) override
    {
        writeToReference(m_out, "streamDetails", streamDetails);
    }
    void exportImages(const QMap<ImageType, QByteArray>& images) override { Q_UNUSED(images); }
    void exportFiles(const mediaelch::FileList& files) override { writeToReference(m_out, "files", files); }

private:
    QTextStream& m_out;
};


class MovieTestExporter final : public Movie::Exporter
{
public:
    MovieTestExporter(QTextStream& out) : m_out{out} {}

    void startExport() override { m_out << "Movie Reference File\n----------------------\n\n"; }
    void endExport() override { m_out.flush(); }

    void exportMovieId(int movieId) override
    {
        // ignored, because it is dependent on a global counter
        Q_UNUSED(movieId);
    }
    void exportDatabaseId(mediaelch::DatabaseId databaseId) override
    {
        // ignored, because it is database dependent
        Q_UNUSED(databaseId);
    }
    void exportImdbId(const ImdbId& imdbId) override { writeToReference(m_out, "imdbId", imdbId); }
    void exportTmdbId(const TmdbId& tmdbId) override { writeToReference(m_out, "tmdbId", tmdbId); }
    void exportMediaCenterId(int mediaCenterId) override { writeToReference(m_out, "mediaCenterId", mediaCenterId); }

    void exportTitle(const QString& title) override { writeToReference(m_out, "title", title); }
    void exportSortTitle(const QString& sortTitle) override { writeToReference(m_out, "sortTitle", sortTitle); }
    void exportOriginalTitle(const QString& originalTitle) override
    {
        writeToReference(m_out, "originalTitle", originalTitle);
    }

    void exportFiles(const mediaelch::FileList& files) override { writeToReference(m_out, "files", files); }
    void exportMovieImages(const MovieImages& movieImages) override
    {
        writeToReference(m_out, "posters", movieImages.posters());
        writeToReference(m_out, "backdrops", movieImages.backdrops());
        writeToReference(m_out, "discArts", movieImages.discArts());
        writeToReference(m_out, "clearArts", movieImages.clearArts());
        writeToReference(m_out, "logos", movieImages.logos());
    }
    void exportFolderName(const QString& folderName) override { writeToReference(m_out, "folderName", folderName); }
    void exportOverview(const QString& overview) override { writeToReference(m_out, "overview", overview); }
    void exportRatings(const Ratings& ratings) override { writeToReference(m_out, "ratings", ratings); }
    void exportUserRating(double userRating) override { writeToReference(m_out, "userRating", userRating); }
    void exportImdbTop250(int imdbTop250) override { writeToReference(m_out, "imdbTop250", imdbTop250); }
    void exportReleased(const QDate& released) override { writeToReference(m_out, "released", released); }
    void exportTagline(const QString& tagline) override { writeToReference(m_out, "tagline", tagline); }
    void exportOutline(const QString& outline) override { writeToReference(m_out, "outline", outline); }
    void exportCrew(const MovieCrew& crew) override
    {
        writeToReference(m_out, "writer", crew.writer());
        writeToReference(m_out, "director", crew.director());
        writeToReference(m_out, "actors", crew.actors());
    }
    void exportRuntime(std::chrono::minutes runtime) override { writeToReference(m_out, "runtime", runtime); }
    void exportCertification(const Certification& certification) override
    {
        writeToReference(m_out, "certification", certification);
    }
    void exportGenres(const QStringList& genres) override { writeToReference(m_out, "genres", genres); }
    void exportCountries(const QStringList& countries) override { writeToReference(m_out, "countries", countries); }
    void exportStudios(const QStringList& studios) override { writeToReference(m_out, "studios", studios); }
    void exportTags(const QStringList& tags) override { writeToReference(m_out, "tags", tags); }
    void exportTrailer(const QUrl& trailer) override { writeToReference(m_out, "trailer", trailer); }
    void exportPlaycount(int playcount) override { writeToReference(m_out, "playcount", playcount); }
    void exportLastPlayed(const QDateTime& lastPlayed) override { writeToReference(m_out, "lastPlayed", lastPlayed); }
    void exportMovieSet(const MovieSet& set) override
    {
        writeToReference(
            m_out, "movie set", QStringLiteral("tmdbid=%1 | name=%2").arg(set.tmdbId.toString(), set.name));
        writeToReference(m_out, "movie set overview", set.overview);
    }
    void exportStreamDetails(const StreamDetails* streamDetails) override
    {
        writeToReference(m_out, "streamDetails", streamDetails);
    }
    void exportFileLastModified(const QDateTime& fileLastModified) override
    {
        writeToReference(m_out, "fileLastModified", fileLastModified);
    }
    void exportDateAdded(const QDateTime& dateAdded) override { writeToReference(m_out, "dateAdded", dateAdded); }
    void exportDiscType(DiscType discType) override { writeToReference(m_out, "discType", discType); }
    void exportLabel(const ColorLabel& label) override { writeToReference(m_out, "color label", int(label)); }
    void exportSubtitles(const QVector<Subtitle*>& subtitles) override
    {
        writeToReference(m_out, "subtitles", subtitles);
    }
    void exportResumeTime(mediaelch::ResumeTime resumeTime) override
    {
        writeToReference(m_out, "resumeTime", resumeTime);
    }

private:
    QTextStream& m_out;
};


class TvShowEpisodeTestExporter final : public TvShowEpisode::Exporter
{
public:
    TvShowEpisodeTestExporter(QTextStream& out) : m_out{out} {}

    void startExport() override { m_out << "TvShowEpisode Reference File\n------------------\n\n"; }
    void endExport() override { m_out.flush(); }

    void exportEpisodeId(int episodeId) override
    {
        // ignored, because it is dependent on a global variable
        Q_UNUSED(episodeId);
    }
    void exportDatabaseId(const mediaelch::DatabaseId& databaseId) override
    {
        // ignored, because it is database dependent
        Q_UNUSED(databaseId);
    }
    void exportTmdbId(const TmdbId& tmdbId) override { writeToReference(m_out, "tmdbId", tmdbId); }
    void exportImdbId(const ImdbId& imdbId) override { writeToReference(m_out, "imdbId", imdbId); }
    void exportTvdbId(const TvDbId& tvdbId) override { writeToReference(m_out, "tvdbId", tvdbId); }
    void exportTvMazeId(const TvMazeId& tvmazeId) override { writeToReference(m_out, "tvmazeId", tvmazeId); }

    void exportTitle(const QString& title) override { writeToReference(m_out, "title", title); }
    void exportShowTitle(const QString& showTitle) override { writeToReference(m_out, "showTitle", showTitle); }

    void exportRatings(const Ratings& ratings) override { writeToReference(m_out, "ratings", ratings); }
    void exportUserRating(double userRating) override { writeToReference(m_out, "userRating", userRating); }
    void exportImdbTop250(int imdbTop250) override { writeToReference(m_out, "imdbTop250", imdbTop250); }

    void exportSeason(SeasonNumber season) override { writeToReference(m_out, "season", season); }
    void exportEpisode(EpisodeNumber episode) override { writeToReference(m_out, "episode", episode); }
    void exportDisplaySeason(SeasonNumber displaySeason) override
    {
        writeToReference(m_out, "displaySeason", displaySeason);
    }
    void exportDisplayEpisode(EpisodeNumber displayEpisode) override
    {
        writeToReference(m_out, "displayEpisode", displayEpisode);
    }

    void exportOverview(const QString& overview) override { writeToReference(m_out, "overview", overview); }
    void exportWriters(const QStringList& writers) override { writeToReference(m_out, "writers", writers); }
    void exportDirectors(const QStringList& directors) override { writeToReference(m_out, "directors", directors); }
    void exportPlayCount(int playCount) override { writeToReference(m_out, "playCount", playCount); }
    void exportLastPlayed(const QDateTime& lastPlayed) override { writeToReference(m_out, "lastPlayed", lastPlayed); }
    void exportFirstAired(const QDate& firstAired) override { writeToReference(m_out, "firstAired", firstAired); }
    void exportTags(const QStringList& tags) override { writeToReference(m_out, "tags", tags); }
    void exportEpBookmark(const QTime& epBookmark) override { writeToReference(m_out, "epBookmark", epBookmark); }
    void exportCertification(const Certification& certification) override
    {
        writeToReference(m_out, "certification", certification);
    }
    void exportNetwork(const QString& network) override { writeToReference(m_out, "network", network); }
    void exportThumbnail(const QUrl& thumbnail) override { writeToReference(m_out, "thumbnail", thumbnail); }
    void exportActors(const Actors& actors) override { writeToReference(m_out, "actors", actors); }
    void exportStreamDetails(const StreamDetails* streamDetails) override
    {
        writeToReference(m_out, "streamDetails", streamDetails);
    }
    void exportFiles(const mediaelch::FileList& files) override { writeToReference(m_out, "files", files); }


private:
    QTextStream& m_out;
};


} // namespace

namespace test {

QString serializeForReference(const Concert& concert)
{
    QString buffer;
    QTextStream out(&buffer);
    out.setGenerateByteOrderMark(true);
    ConcertTestExporter exporter{out};
    concert.exportTo(exporter);
    return buffer;
}

QString serializeForReference(const Movie& concert)
{
    QString buffer;
    QTextStream out(&buffer);
    out.setGenerateByteOrderMark(true);
    MovieTestExporter exporter{out};
    concert.exportTo(exporter);
    return buffer;
}

QString serializeForReference(const Album& album)
{
    QString buffer;
    QTextStream out(&buffer);
    out.setGenerateByteOrderMark(true);
    AlbumTestExporter exporter{out};
    album.exportTo(exporter);
    return buffer;
}

QString serializeForReference(const Artist& artist)
{
    QString buffer;
    QTextStream out(&buffer);
    out.setGenerateByteOrderMark(true);
    ArtistTestExporter exporter{out};
    artist.exportTo(exporter);
    return buffer;
}

QString serializeForReference(const TvShowEpisode& episode)
{
    QString buffer;
    QTextStream out(&buffer);
    out.setGenerateByteOrderMark(true);
    TvShowEpisodeTestExporter exporter{out};
    episode.exportTo(exporter);
    return buffer;
}

} // namespace test
