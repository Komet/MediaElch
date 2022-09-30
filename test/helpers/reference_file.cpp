#include "test/helpers/reference_file.h"

#include "data/concert/Concert.h"
#include "data/movie/Movie.h"
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
static void writeToReference(QTextStream& out, const QString& key, const TmdbId& value)
{
    out << key << ": " << value.toString() << "\n";
}
static void writeToReference(QTextStream& out, const QString& key, const ImdbId& value)
{
    out << key << ": " << value.toString() << "\n";
}
static void writeToReference(QTextStream& out, const QString& key, const QUrl& value)
{
    out << key << ": " << value.toString() << "\n";
}
static void writeToReference(QTextStream& out, const QString& key, const QDate& value)
{
    out << key << ": " << value.toString(Qt::DateFormat::ISODate) << "\n";
}
static void writeToReference(QTextStream& out, const QString& key, const QDateTime& value)
{
    out << key << ": " << value.toString(Qt::DateFormat::ISODate) << "\n";
}
static void writeToReference(QTextStream& out, const QString& key, const Certification& value)
{
    out << key << ": " << value.toString() << "\n";
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

} // namespace test
