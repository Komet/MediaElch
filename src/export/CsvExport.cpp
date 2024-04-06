#include "export/CsvExport.h"

#include "data/Rating.h"
#include "data/concert/Concert.h"
#include "data/movie/Movie.h"
#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "data/tv_show/TvShow.h"
#include "data/tv_show/TvShowEpisode.h"

#include <utility>

static QString ratingsToString(const Ratings& ratings)
{
    QStringList out;
    for (const Rating& rating : ratings) {
        if (rating.voteCount > 1) {
            out << QStringLiteral("%1: %2 (%3 votes)")
                       .arg(rating.source,
                           QString::number(rating.rating), //
                           QString::number(rating.voteCount));
        } else {
            out << QStringLiteral("%1: %2").arg(rating.source, QString::number(rating.rating));
        }
    }

    return out.join(", ");
}

static QString actorsToString(const Actors& actors)
{
    QStringList out;
    for (const Actor* actor : actors.actors()) {
        if (!actor->role.isEmpty()) {
            out << QStringLiteral("%1 (%2)").arg(actor->name, actor->role);
        } else {
            out << actor->name;
        }
    }
    return out.join(", ");
}

static QString filesToString(const mediaelch::FileList& files)
{
    QStringList filenames;
    for (const mediaelch::FilePath& path : files) {
        if (path.isValid()) {
            filenames << path.fileName();
        }
    }
    return filenames.join(", ");
}

static QString dirFromFileList(const mediaelch::FileList& files)
{
    if (files.isEmpty()) {
        return "";
    }
    return files.first().dir().toNativePathString();
}

static QString getStreamDetails(const StreamDetails* streamDetails, StreamDetails::VideoDetails detail)
{
    if (streamDetails == nullptr) {
        return {};
    }
    const auto details = streamDetails->videoDetails();
    return details.contains(detail) ? *details.find(detail) : QString{};
}

static QString getStreamDetails(const StreamDetails* streamDetails, StreamDetails::AudioDetails detail)
{
    if (streamDetails == nullptr) {
        return {};
    }
    QStringList values;
    const auto details = streamDetails->audioDetails();
    for (const auto& map : details) {
        if (map.contains(detail)) {
            values << *map.find(detail);
        }
    }
    return values.join(", ");
}

static QString getStreamDetails(const StreamDetails* streamDetails, StreamDetails::SubtitleDetails detail)
{
    if (streamDetails == nullptr) {
        return {};
    }
    QStringList values;
    const auto details = streamDetails->subtitleDetails();
    for (const auto& map : details) {
        if (map.contains(detail)) {
            values << *map.find(detail);
        }
    }
    return values.join(", ");
}

namespace mediaelch {

CsvMovieExport::CsvMovieExport(QTextStream& outStream, QVector<CsvMovieExport::Field> fields) :
    CsvMediaExport(outStream), m_fields{std::move(fields)}
{
}

void CsvMovieExport::exportMovies(const QVector<Movie*>& movies, std::function<void()> callback)
{
    CsvExport csv(m_out);
    csv.setFieldsInOrder(fieldsToStrings());
    csv.setSeparator(m_separator);
    csv.setReplacement(m_replacement);

    const auto s = [](Field field) { return fieldToString(field); };

    csv.writeHeader();

    for (Movie* movie : asConst(movies)) {
        const auto* st = movie->streamDetails();
        csv.addRow({
            {s(Field::Type), "movie"},
            {s(Field::Imdbid), movie->imdbId().toString()},
            {s(Field::Tmdbid), movie->tmdbId().toString()},
            {s(Field::WikidataId), movie->wikidataId().toString()},
            {s(Field::Title), movie->name()},
            {s(Field::OriginalTitle), movie->originalName()},
            {s(Field::SortTitle), movie->sortTitle()},
            {s(Field::Overview), movie->overview()},
            {s(Field::Outline), movie->outline()},
            {s(Field::Ratings), ratingsToString(movie->ratings())},
            {s(Field::UserRating), QString::number(movie->userRating())},
            {s(Field::IsImdbTop250), QString::number(movie->top250())},
            {s(Field::ReleaseDate), movie->released().isValid() ? movie->released().toString(Qt::ISODate) : ""},
            {s(Field::Tagline), movie->tagline()},
            {s(Field::Runtime), QString::number(movie->runtime().count())},
            {s(Field::Certification), movie->certification().toString()},
            {s(Field::Writers), movie->writer()},
            {s(Field::Directors), movie->director()},
            {s(Field::Genres), movie->genres().join(", ")},
            {s(Field::Countries), movie->countries().join(", ")},
            {s(Field::Studios), movie->studios().join(", ")},
            {s(Field::Tags), movie->tags().join(", ")},
            {s(Field::Trailer), movie->trailer().toString()},
            {s(Field::Actors), actorsToString(movie->actors())},
            {s(Field::PlayCount), QString::number(movie->playcount())},
            {s(Field::LastPlayed), movie->lastPlayed().toString(Qt::ISODate)},
            {s(Field::MovieSet), movie->set().name},
            {s(Field::Directory), dirFromFileList(movie->files())},
            {s(Field::Filenames), filesToString(movie->files())},
            {s(Field::LastModified), movie->fileLastModified().toString(Qt::ISODate)},
            {s(Field::StreamDetails_Video_DurationInSeconds),
                getStreamDetails(st, StreamDetails::VideoDetails::DurationInSeconds)},
            {s(Field::StreamDetails_Video_Aspect), getStreamDetails(st, StreamDetails::VideoDetails::Aspect)},
            {s(Field::StreamDetails_Video_Width), getStreamDetails(st, StreamDetails::VideoDetails::Width)},
            {s(Field::StreamDetails_Video_Height), getStreamDetails(st, StreamDetails::VideoDetails::Height)},
            {s(Field::StreamDetails_Video_Codec), getStreamDetails(st, StreamDetails::VideoDetails::Codec)},
            {s(Field::StreamDetails_Audio_Language), getStreamDetails(st, StreamDetails::AudioDetails::Language)},
            {s(Field::StreamDetails_Audio_Codec), getStreamDetails(st, StreamDetails::AudioDetails::Codec)},
            {s(Field::StreamDetails_Audio_Channels), getStreamDetails(st, StreamDetails::AudioDetails::Channels)},
            {s(Field::StreamDetails_Subtitle_Language),
                getStreamDetails(st, StreamDetails::SubtitleDetails::Language)} //
        });
        callback();
    }
}

QVector<QString> CsvMovieExport::fieldsToStrings() const
{
    QVector<QString> out;
    for (const Field field : asConst(m_fields)) {
        out << fieldToString(field);
    }
    return out;
}

QString CsvMovieExport::fieldToString(Field field)
{
    switch (field) {
    case Field::Type: return "type";
    case Field::Imdbid: return "movie_imdb_id";
    case Field::Tmdbid: return "movie_tmdb_id";
    case Field::WikidataId: return "movie_wikidata_id";
    case Field::Title: return "movie_title";
    case Field::OriginalTitle: return "movie_original_title";
    case Field::SortTitle: return "movie_sort_title";
    case Field::Overview: return "movie_overview";
    case Field::Outline: return "movie_outline";
    case Field::Ratings: return "movie_ratings";
    case Field::UserRating: return "movie_user_rating";
    case Field::IsImdbTop250: return "movie_top250";
    case Field::ReleaseDate: return "movie_release_date";
    case Field::Tagline: return "movie_tagline";
    case Field::Runtime: return "movie_runtime";
    case Field::Certification: return "movie_certification";
    case Field::Writers: return "movie_writers";
    case Field::Directors: return "movie_directors";
    case Field::Genres: return "movie_genres";
    case Field::Countries: return "movie_countries";
    case Field::Studios: return "movie_studios";
    case Field::Tags: return "movie_tags";
    case Field::Trailer: return "movie_trailers";
    case Field::Actors: return "movie_actors";
    case Field::PlayCount: return "movie_playcount";
    case Field::LastPlayed: return "movie_last_played";
    case Field::MovieSet: return "movie_set";
    case Field::Directory: return "movie_directory";
    case Field::Filenames: return "movie_filenames";
    case Field::LastModified: return "movie_date_added";
    case Field::StreamDetails_Video_DurationInSeconds: return "movie_streamdetails_video_duration_in_seconds";
    case Field::StreamDetails_Video_Aspect: return "movie_streamdetails_video_aspect";
    case Field::StreamDetails_Video_Width: return "movie_streamdetails_video_width";
    case Field::StreamDetails_Video_Height: return "movie_streamdetails_video_height";
    case Field::StreamDetails_Video_Codec: return "movie_streamdetails_video_codec";
    case Field::StreamDetails_Audio_Language: return "movie_streamdetails_audio_languages";
    case Field::StreamDetails_Audio_Codec: return "movie_streamdetails_audio_codecs";
    case Field::StreamDetails_Audio_Channels: return "movie_streamdetails_audio_channels";
    case Field::StreamDetails_Subtitle_Language: return "movie_streamdetails_subtitle_languages";
    };
    return "unknown";
}

CsvTvShowExport::CsvTvShowExport(QTextStream& outStream, QVector<CsvTvShowExport::Field> fields) :
    CsvMediaExport(outStream), m_fields{std::move(fields)}
{
}

void CsvTvShowExport::exportTvShows(const QVector<TvShow*>& shows, std::function<void()> callback)
{
    CsvExport csv(m_out);
    csv.setFieldsInOrder(fieldsToStrings());
    csv.setSeparator(m_separator);
    csv.setReplacement(m_replacement);

    const auto s = [](Field field) { return fieldToString(field); };

    csv.writeHeader();

    for (TvShow* show : shows) {
        csv.addRow({
            {s(Field::Type), "tvshow"},
            {s(Field::ShowTmdbId), show->tmdbId().toString()},
            {s(Field::ShowImdbId), show->imdbId().toString()},
            {s(Field::ShowTvDbId), show->tvdbId().toString()},
            {s(Field::ShowTvMazeId), show->tvmazeId().toString()},
            {s(Field::ShowTitle), show->title()},
            {s(Field::ShowSortTitle), show->sortTitle()},
            {s(Field::ShowOriginalTitle), show->originalTitle()},
            {s(Field::ShowFirstAired), show->firstAired().toString(Qt::ISODate)},
            {s(Field::ShowNetwork), show->networks().join(", ")},
            {s(Field::ShowCertification), show->certification().toString()},
            {s(Field::ShowGenres), show->genres().join(", ")},
            {s(Field::ShowTags), show->tags().join(", ")},
            {s(Field::ShowRuntime), QString::number(show->runtime().count())},
            {s(Field::ShowRatings), ratingsToString(show->ratings())},
            {s(Field::ShowUserRating), QString::number(show->userRating())},
            {s(Field::ShowActors), actorsToString(show->actors())},
            {s(Field::ShowOverview), show->overview()},
            {s(Field::ShowIsImdbTop250), QString::number(show->top250())},
            {s(Field::ShowDirectory), show->dir().toNativePathString()}, //
        });

        callback();
    }
}

QVector<QString> CsvTvShowExport::fieldsToStrings() const
{
    QVector<QString> out;
    for (const Field field : asConst(m_fields)) {
        out << fieldToString(field);
    }
    return out;
}

QString CsvTvShowExport::fieldToString(CsvTvShowExport::Field field)
{
    switch (field) {
    case Field::Type: return "type";
    case Field::ShowImdbId: return "show_imdb_id";
    case Field::ShowTmdbId: return "show_tmdb_id";
    case Field::ShowTvMazeId: return "show_tvmaze_id";
    case Field::ShowTvDbId: return "show_tvdb_id";
    case Field::ShowTitle: return "show_title";
    case Field::ShowSortTitle: return "show_sort_title";
    case Field::ShowOriginalTitle: return "show_original_title";
    case Field::ShowFirstAired: return "show_first_aired";
    case Field::ShowNetwork: return "show_network";
    case Field::ShowGenres: return "show_genres";
    case Field::ShowRuntime: return "show_runtime";
    case Field::ShowRatings: return "show_ratings";
    case Field::ShowUserRating: return "show_user_rating";
    case Field::ShowOverview: return "show_overview";
    case Field::ShowActors: return "show_actors";
    case Field::ShowCertification: return "show_certification";
    case Field::ShowTags: return "show_tags";
    case Field::ShowIsImdbTop250: return "show_imdb_top_250";
    case Field::ShowDirectory: return "show_directory";
    }
    return "unknown";
}


CsvTvEpisodeExport::CsvTvEpisodeExport(QTextStream& outStream, QVector<CsvTvEpisodeExport::Field> fields) :
    CsvMediaExport(outStream), m_fields{std::move(fields)}
{
}

void CsvTvEpisodeExport::exportEpisodes(const QVector<TvShow*>& shows, std::function<void()> callback)
{
    CsvExport csv(m_out);
    csv.setFieldsInOrder(fieldsToStrings());
    csv.setSeparator(m_separator);
    csv.setReplacement(m_replacement);

    const auto s = [](Field field) { return fieldToString(field); };

    csv.writeHeader();

    for (TvShow* show : shows) {
        for (TvShowEpisode* episode : asConst(show->episodes())) {
            const auto* st = episode->streamDetails();
            csv.addRow({
                {s(Field::Type), "episode"},
                {s(Field::ShowTmdbId), show->tmdbId().toString()},
                {s(Field::ShowImdbId), show->imdbId().toString()},
                {s(Field::ShowTvDbId), show->tvdbId().toString()},
                {s(Field::ShowTvMazeId), show->tvmazeId().toString()},
                {s(Field::ShowTitle), show->title()},
                {s(Field::EpisodeSeason), episode->seasonNumber().toString()},
                {s(Field::EpisodeNumber), episode->episodeNumber().toString()},
                {s(Field::EpisodeTmdbId), episode->tmdbId().toString()},
                {s(Field::EpisodeImdbId), episode->imdbId().toString()},
                {s(Field::EpisodeTvDbId), episode->tvdbId().toString()},
                {s(Field::EpisodeTvMazeId), episode->tvmazeId().toString()},
                {s(Field::EpisodeFirstAired), episode->firstAired().toString(Qt::ISODate)},
                {s(Field::EpisodeTitle), episode->title()},
                {s(Field::EpisodeOverview), episode->overview()},
                {s(Field::EpisodeTitle), episode->title()},
                {s(Field::EpisodeUserRating), QString::number(episode->userRating())},
                {s(Field::EpisodeActors), actorsToString(episode->actors())},
                {s(Field::EpisodeDirectory), dirFromFileList(episode->files())},
                {s(Field::EpisodeFilenames), filesToString(episode->files())},
                {s(Field::EpisodeStreamDetails_Video_DurationInSeconds),
                    getStreamDetails(st, StreamDetails::VideoDetails::DurationInSeconds)},
                {s(Field::EpisodeStreamDetails_Video_Aspect),
                    getStreamDetails(st, StreamDetails::VideoDetails::Aspect)},
                {s(Field::EpisodeStreamDetails_Video_Width), getStreamDetails(st, StreamDetails::VideoDetails::Width)},
                {s(Field::EpisodeStreamDetails_Video_Height),
                    getStreamDetails(st, StreamDetails::VideoDetails::Height)},
                {s(Field::EpisodeStreamDetails_Video_Codec), getStreamDetails(st, StreamDetails::VideoDetails::Codec)},
                {s(Field::EpisodeStreamDetails_Audio_Language),
                    getStreamDetails(st, StreamDetails::AudioDetails::Language)},
                {s(Field::EpisodeStreamDetails_Audio_Codec), getStreamDetails(st, StreamDetails::AudioDetails::Codec)},
                {s(Field::EpisodeStreamDetails_Audio_Channels),
                    getStreamDetails(st, StreamDetails::AudioDetails::Channels)},
                {s(Field::EpisodeStreamDetails_Subtitle_Language),
                    getStreamDetails(st, StreamDetails::SubtitleDetails::Language)} //
            });
        }
        callback();
    }
}

QVector<QString> CsvTvEpisodeExport::fieldsToStrings() const
{
    QVector<QString> out;
    for (const Field field : asConst(m_fields)) {
        out << fieldToString(field);
    }
    return out;
}

QString CsvTvEpisodeExport::fieldToString(CsvTvEpisodeExport::Field field)
{
    switch (field) {
    case Field::Type: return "type";
    case Field::ShowImdbId: return "show_imdb_id";
    case Field::ShowTmdbId: return "show_tmdb_id";
    case Field::ShowTvMazeId: return "show_tvmaze_id";
    case Field::ShowTvDbId: return "show_tvdb_id";
    case Field::ShowTitle: return "show_title";
    case Field::EpisodeSeason: return "episode_season";
    case Field::EpisodeNumber: return "episode_number";
    case Field::EpisodeImdbId: return "episode_imdb_id";
    case Field::EpisodeTmdbId: return "episode_tmdb_id";
    case Field::EpisodeTvDbId: return "episode_tvdb_id";
    case Field::EpisodeTvMazeId: return "episode_tvmaze_id";
    case Field::EpisodeFirstAired: return "episode_actors";
    case Field::EpisodeTitle: return "episode_title";
    case Field::EpisodeOverview: return "episode_overview";
    case Field::EpisodeUserRating: return "episode_user_rating";
    case Field::EpisodeDirectors: return "episode_directors";
    case Field::EpisodeWriters: return "episode_writers";
    case Field::EpisodeActors: return "episode_actors";
    case Field::EpisodeDirectory: return "episode_directory";
    case Field::EpisodeFilenames: return "episode_filenames";
    case Field::EpisodeStreamDetails_Video_DurationInSeconds: return "episode_streamdetails_video_duration_in_seconds";
    case Field::EpisodeStreamDetails_Video_Aspect: return "episode_streamdetails_video_aspect";
    case Field::EpisodeStreamDetails_Video_Width: return "episode_streamdetails_video_width";
    case Field::EpisodeStreamDetails_Video_Height: return "episode_streamdetails_video_height";
    case Field::EpisodeStreamDetails_Video_Codec: return "episode_streamdetails_video_codec";
    case Field::EpisodeStreamDetails_Audio_Language: return "episode_streamdetails_audio_languages";
    case Field::EpisodeStreamDetails_Audio_Codec: return "episode_streamdetails_audio_codecs";
    case Field::EpisodeStreamDetails_Audio_Channels: return "episode_streamdetails_audio_channels";
    case Field::EpisodeStreamDetails_Subtitle_Language: return "episode_streamdetails_subtitle_languages";
    }
    return "unknown";
}

CsvConcertExport::CsvConcertExport(QTextStream& outStream, QVector<CsvConcertExport::Field> fields) :
    CsvMediaExport(outStream), m_fields{std::move(fields)}
{
}

class CsvConcertExport::FieldExport final : public mediaelch::ConcertData::Exporter
{
public:
    void startExport() override { fields = {{s(Field::Type), "concert"}}; }
    void endExport() override {};

    // clang-format off
    void exportDatabaseId(DatabaseId databaseId) override { /* ignored */ Q_UNUSED(databaseId); }
    void exportMediaCenterId(int mediaCenterId)  override { /* ignored */ Q_UNUSED(mediaCenterId); }
    void exportTmdbId(const TmdbId& tmdbId)      override { fields[s(Field::TmdbId)] = tmdbId.toString(); }
    void exportImdbId(const ImdbId& imdbId)      override { fields[s(Field::ImdbId)] = imdbId.toString(); }

    void exportTitle(const QString& title)                 override { fields[s(Field::Title)]        = title; };
    void exportOriginalTitle(const QString& originalTitle) override { fields[s(Field::OriginalTitle)] = originalTitle; }

    void exportArtist(const QString& artist)         override { fields[s(Field::Artist)]      = artist; }
    void exportAlbum(const QString& album)           override { fields[s(Field::Album)]       = album; }
    void exportOverview(const QString& overview)     override { fields[s(Field::Overview)]    = overview; }
    void exportRatings(const Ratings& ratings)       override { fields[s(Field::Ratings)]     = ratingsToString(ratings); }
    void exportUserRating(double userRating)         override { fields[s(Field::UserRating)] = QString::number(userRating); }
    void exportReleaseDate(const QDate& releaseDate) override { fields[s(Field::ReleaseDate)] = releaseDate.toString(Qt::ISODate); }
    void exportTagline(const QString& tagline)       override { fields[s(Field::Tagline)]     = tagline; }

    void exportRuntime(const std::chrono::minutes& runtime)      override { fields[s(Field::Runtime)]       = QString::number(runtime.count()); }
    void exportCertification(const Certification& certification) override { fields[s(Field::Certification)] = certification.toString(); }

    void exportGenres(const QStringList& genres) override { fields[s(Field::Genres)]     = genres.join(", "); }
    void exportTags(const QStringList& tags)     override { fields[s(Field::Tags)]       = tags.join(", "); }
    void exportTrailer(const QUrl& trailer)      override { fields[s(Field::TrailerUrl)] = trailer.toString(); }
    void exportPlaycount(const int& playcount)   override { fields[s(Field::Playcount)]  = QString::number(playcount); }

    void exportLastPlayed(const QDateTime& lastPlayed)           override { fields[s(Field::LastPlayed)]   = lastPlayed.toString(Qt::ISODate); }
    void exportLastModified(const QDateTime& lastModified)       override { fields[s(Field::LastModified)] = lastModified.toString(Qt::ISODate); }
    void exportPosters(const QVector<Poster>& posters)           override { /* ignored */ Q_UNUSED(posters); }
    void exportBackdrops(const QVector<Poster>& backdrops)       override { /* ignored */ Q_UNUSED(backdrops); }
    void exportExtraFanarts(const QStringList& extraFanarts)     override { /* ignored */ Q_UNUSED(extraFanarts); }
    void exportImages(const QMap<ImageType, QByteArray>& images) override { /* ignored */ Q_UNUSED(images); }

    void exportStreamDetails(const StreamDetails* st) override {
        fields[s(Field::StreamDetails_Video_DurationInSeconds)] = getStreamDetails(st, StreamDetails::VideoDetails::DurationInSeconds);
        fields[s(Field::StreamDetails_Video_Aspect)]      = getStreamDetails(st, StreamDetails::VideoDetails::Aspect);
        fields[s(Field::StreamDetails_Video_Width)]       = getStreamDetails(st, StreamDetails::VideoDetails::Width);
        fields[s(Field::StreamDetails_Video_Height)]      = getStreamDetails(st, StreamDetails::VideoDetails::Height);
        fields[s(Field::StreamDetails_Video_Codec)]       = getStreamDetails(st, StreamDetails::VideoDetails::Codec);
        fields[s(Field::StreamDetails_Audio_Language)]    = getStreamDetails(st, StreamDetails::AudioDetails::Language);
        fields[s(Field::StreamDetails_Audio_Codec)]       = getStreamDetails(st, StreamDetails::AudioDetails::Codec);
        fields[s(Field::StreamDetails_Audio_Channels)]    = getStreamDetails(st, StreamDetails::AudioDetails::Channels);
        fields[s(Field::StreamDetails_Subtitle_Language)] = getStreamDetails(st, StreamDetails::SubtitleDetails::Language);
    }
    // clang-format on

    void exportFiles(const mediaelch::FileList& files) override
    {
        fields[fieldToString(Field::Directory)] = dirFromFileList(files);
        fields[fieldToString(Field::Filenames)] = filesToString(files);
    }

private:
    QString s(Field field) { return fieldToString(field); }

public:
    QMap<QString, QString> fields;
};


void CsvConcertExport::exportConcerts(const QVector<Concert*>& concerts, std::function<void()> callback)
{
    CsvExport csv(m_out);
    csv.setFieldsInOrder(fieldsToStrings());
    csv.setSeparator(m_separator);
    csv.setReplacement(m_replacement);
    csv.writeHeader();

    CsvConcertExport::FieldExport exporter;
    for (Concert* concert : asConst(concerts)) {
        concert->exportTo(exporter);
        csv.addRow(exporter.fields);
        callback();
    }
}

QVector<QString> CsvConcertExport::fieldsToStrings() const
{
    QVector<QString> out;
    for (const Field field : asConst(m_fields)) {
        out << fieldToString(field);
    }
    return out;
}

QString CsvConcertExport::fieldToString(CsvConcertExport::Field field)
{
    switch (field) {
    case Field::Type: return "type";
    case Field::TmdbId: return "concert_tmdb_id";
    case Field::ImdbId: return "concert_imdb_id";
    case Field::Title: return "concert_title";
    case Field::OriginalTitle: return "concert_original_title";
    case Field::Artist: return "concert_artist";
    case Field::Album: return "concert_album";
    case Field::Overview: return "concert_overview";
    case Field::Ratings: return "concert_ratings";
    case Field::UserRating: return "concert_user_rating";
    case Field::ReleaseDate: return "concert_release_date";
    case Field::Tagline: return "concert_tagline";
    case Field::Runtime: return "concert_runtime";
    case Field::Certification: return "concert_certification";
    case Field::Genres: return "concert_genres";
    case Field::Tags: return "concert_tags";
    case Field::TrailerUrl: return "concert_trailer_url";
    case Field::Playcount: return "concert_playcount";
    case Field::LastPlayed: return "concert_last_played";
    case Field::LastModified: return "concert_last_modified";
    case Field::Directory: return "concert_directory";
    case Field::Filenames: return "concert_filenames";
    case Field::StreamDetails_Video_DurationInSeconds: return "concert_streamdetails_video_duration_in_seconds";
    case Field::StreamDetails_Video_Aspect: return "concert_streamdetails_video_aspect";
    case Field::StreamDetails_Video_Width: return "concert_streamdetails_video_width";
    case Field::StreamDetails_Video_Height: return "concert_streamdetails_video_height";
    case Field::StreamDetails_Video_Codec: return "concert_streamdetails_video_codec";
    case Field::StreamDetails_Audio_Language: return "concert_streamdetails_audio_languages";
    case Field::StreamDetails_Audio_Codec: return "concert_streamdetails_audio_codecs";
    case Field::StreamDetails_Audio_Channels: return "concert_streamdetails_audio_channels";
    case Field::StreamDetails_Subtitle_Language: return "concert_streamdetails_subtitle_languages";
    }
    return "unknown";
}

CsvArtistExport::CsvArtistExport(QTextStream& outStream, QVector<CsvArtistExport::Field> fields) :
    CsvMediaExport(outStream), m_fields{std::move(fields)}
{
}

void CsvArtistExport::exportArtists(const QVector<Artist*>& artists, std::function<void()> callback)
{
    CsvExport csv(m_out);
    csv.setFieldsInOrder(fieldsToStrings());
    csv.setSeparator(m_separator);
    csv.setReplacement(m_replacement);

    const auto s = [](Field field) { return fieldToString(field); };

    csv.writeHeader();

    for (Artist* artist : asConst(artists)) {
        csv.addRow({
            {s(Field::Type), "artist"},
            {s(Field::ArtistName), artist->name()},
            {s(Field::ArtistGenres), artist->genres().join(", ")},
            {s(Field::ArtistStyles), artist->styles().join(", ")},
            {s(Field::ArtistMoods), artist->moods().join(", ")},
            {s(Field::ArtistYearsActive), artist->yearsActive()},
            {s(Field::ArtistFormed), artist->formed()},
            {s(Field::ArtistBiography), artist->biography()},
            {s(Field::ArtistBorn), artist->born()},
            {s(Field::ArtistDied), artist->died()},
            {s(Field::ArtistDisbanded), artist->disbanded()},
            {s(Field::ArtistMusicBrainzId), artist->mbId().toString()},
            {s(Field::ArtistAllMusicId), artist->allMusicId().toString()},
            {s(Field::ArtistDirectory), artist->path().toNativePathString()} //
        });
        callback();
    }
}

QVector<QString> CsvArtistExport::fieldsToStrings() const
{
    QVector<QString> out;
    for (const Field field : asConst(m_fields)) {
        out << fieldToString(field);
    }
    return out;
}

QString CsvArtistExport::fieldToString(CsvArtistExport::Field field)
{
    switch (field) {
    case Field::Type: return "type";
    case Field::ArtistName: return "artist_name";
    case Field::ArtistGenres: return "artist_genres";
    case Field::ArtistStyles: return "artist_styles";
    case Field::ArtistMoods: return "artist_moods";
    case Field::ArtistYearsActive: return "artist_years_active";
    case Field::ArtistFormed: return "artist_formed";
    case Field::ArtistBiography: return "artist_biography";
    case Field::ArtistBorn: return "artist_born";
    case Field::ArtistDied: return "artist_died";
    case Field::ArtistDisbanded: return "artist_disbanded";
    case Field::ArtistMusicBrainzId: return "artist_music_brainz_id";
    case Field::ArtistAllMusicId: return "artist_all_music_id";
    case Field::ArtistDirectory: return "artist_directory";
    }
    return "unknown";
}


CsvAlbumExport::CsvAlbumExport(QTextStream& outStream, QVector<CsvAlbumExport::Field> fields) :
    CsvMediaExport(outStream), m_fields{std::move(fields)}
{
}

void CsvAlbumExport::exportAlbumsOfArtists(const QVector<Artist*>& artists, std::function<void()> callback)
{
    CsvExport csv(m_out);
    csv.setFieldsInOrder(fieldsToStrings());
    csv.setSeparator(m_separator);
    csv.setReplacement(m_replacement);

    const auto s = [](Field field) { return fieldToString(field); };

    csv.writeHeader();

    for (Artist* artist : asConst(artists)) {
        const auto albums = artist->albums();
        for (Album* album : albums) {
            csv.addRow({
                {s(Field::Type), "album"},
                {s(Field::ArtistName), artist->name()},
                {s(Field::AlbumTitle), album->title()},
                {s(Field::AlbumArtistName), album->artist()},
                {s(Field::AlbumGenres), album->genres().join(", ")},
                {s(Field::AlbumStyles), album->styles().join(", ")},
                {s(Field::AlbumMoods), album->moods().join(", ")},
                {s(Field::AlbumReview), album->review()},
                {s(Field::AlbumReleaseDate), album->releaseDate()},
                {s(Field::AlbumLabel), album->label()},
                {s(Field::AlbumRating), QString::number(album->rating())},
                {s(Field::AlbumYear), QString::number(album->year())},
                {s(Field::AlbumMusicBrainzId), album->mbAlbumId().toString()},
                {s(Field::AlbumMusicBrainzReleaseGroupId), album->mbReleaseGroupId().toString()},
                {s(Field::AlbumAllMusicId), album->allMusicId().toString()},
                {s(Field::AlbumDirectory), album->path().toNativePathString()} //
            });
        }
        callback();
    }
}

QVector<QString> CsvAlbumExport::fieldsToStrings() const
{
    QVector<QString> out;
    for (const Field field : asConst(m_fields)) {
        out << fieldToString(field);
    }
    return out;
}

QString CsvAlbumExport::fieldToString(CsvAlbumExport::Field field)
{
    switch (field) {
    case Field::Type: return "type";
    case Field::ArtistName: return "artist_name";
    case Field::AlbumTitle: return "album_title";
    case Field::AlbumArtistName: return "album_artist_name";
    case Field::AlbumGenres: return "album_genres";
    case Field::AlbumStyles: return "album_styles";
    case Field::AlbumMoods: return "album_moods";
    case Field::AlbumReview: return "album_review";
    case Field::AlbumReleaseDate: return "album_release_date";
    case Field::AlbumLabel: return "album_label";
    case Field::AlbumRating: return "album_rating";
    case Field::AlbumYear: return "album_year";
    case Field::AlbumMusicBrainzId: return "album_music_brainz_id";
    case Field::AlbumMusicBrainzReleaseGroupId: return "album_music_brainz_release_group_id";
    case Field::AlbumAllMusicId: return "album_all_music_id";
    case Field::AlbumDirectory: return "album_directory";
    }
    return "unknown";
}

void CsvExport::writeHeader()
{
    QVector<QString>::const_iterator i = m_fieldsInOrder.cbegin();
    writeEscaped(*i);
    ++i;

    for (; i != m_fieldsInOrder.cend(); ++i) {
        m_out << m_separator;
        writeEscaped(*i);
    }
    m_out << "\n";
}

void CsvExport::addRow(const QMap<QString, QString>& values)
{
    if (m_fieldsInOrder.isEmpty()) {
        return;
    }

    QVector<QString>::const_iterator i = m_fieldsInOrder.cbegin();
    writeEscaped(values.value(*i));
    ++i;

    for (; i != m_fieldsInOrder.cend(); ++i) {
        m_out << m_separator;
        writeEscaped(values.value(*i));
    }
    m_out << "\n";
}

void CsvExport::writeEscaped(const QString& text)
{
    // See https://owasp.org/www-community/attacks/CSV_Injection
    // Microsoft Excel and other tools are... bad with user provided data.  If a field starts with '=', it's
    // interpreted as code.  So we prepend it with a '.
    bool startsWithForbiddenCharacter = text.startsWith("=") || text.startsWith("@") || text.startsWith("\t")
                                        || text.startsWith("\r") || text.startsWith("+") || text.startsWith("-");

    if (!text.contains(m_separator) && !text.contains("\n") && !startsWithForbiddenCharacter) {
        m_out << text;
        return;
    }

    if (startsWithForbiddenCharacter) {
        m_out << "'";
    }

    m_out << (QString(text)
                  .replace(m_separator, m_replacement)
                  .replace("\r\n", "\\n")
                  .replace("\n", "\\n")
                  .replace("\r", "\\n"));
}

} // namespace mediaelch
