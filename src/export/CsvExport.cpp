#include "export/CsvExport.h"

#include "concerts/Concert.h"
#include "movies/Movie.h"
#include "music/Album.h"
#include "music/Artist.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"

static QString ratingsToString(const QVector<Rating>& ratings)
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

static QString actorsToString(const QVector<Actor*>& actors)
{
    QStringList out;
    for (const Actor* actor : actors) {
        if (!actor->role.isEmpty()) {
            out << QStringLiteral("%1 (%2)").arg(actor->name, actor->role);
        } else {
            out << actor->name;
        }
    }
    return out.join(", ");
}

namespace mediaelch {

CsvMovieExport::CsvMovieExport(QVector<CsvMovieExport::MovieField> fields, QObject* parent) :
    CsvMediaExport(parent), m_fields{fields}
{
}

QString CsvMovieExport::exportMovies(const QVector<Movie*>& movies, std::function<void()> callback)
{
    CsvExport csv;
    csv.setFieldsInOrder(fieldsToStrings());
    csv.setSeparator(m_separator);
    csv.setReplacement(m_replacement);

    const auto s = [this](MovieField field) { return fieldToString(field); };

    csv.writeHeader();

    for (Movie* movie : asConst(movies)) {
        csv.addRow({
            {s(MovieField::Imdbid), movie->imdbId().toString()},
            {s(MovieField::Tmdbid), movie->tmdbId().toString()},
            {s(MovieField::Title), movie->name()},
            {s(MovieField::OriginalTitle), movie->originalName()},
            {s(MovieField::SortTitle), movie->sortTitle()},
            {s(MovieField::Overview), movie->overview()},
            {s(MovieField::Outline), movie->outline()},
            {s(MovieField::Ratings), ratingsToString(movie->ratings())},
            {s(MovieField::UserRating), QString::number(movie->userRating())},
            {s(MovieField::IsImdbTop250), QString::number(movie->top250())},
            {s(MovieField::ReleaseDate), movie->released().isValid() ? movie->released().toString(Qt::ISODate) : ""},
            {s(MovieField::Tagline), movie->tagline()},
            {s(MovieField::Runtime), QString::number(movie->runtime().count())},
            {s(MovieField::Certification), movie->certification().toString()},
            {s(MovieField::Writers), movie->writer()},
            {s(MovieField::Directors), movie->director()},
            {s(MovieField::Genres), movie->genres().join(", ")},
            {s(MovieField::Countries), movie->countries().join(", ")},
            {s(MovieField::Studios), movie->studios().join(", ")},
            {s(MovieField::Tags), movie->tags().join(", ")},
            {s(MovieField::Trailer), movie->trailer().toString()},
            {s(MovieField::Actors), actorsToString(movie->actors())},
            {s(MovieField::PlayCount), QString::number(movie->playcount())},
            {s(MovieField::LastPlayed), movie->lastPlayed().toString(Qt::ISODate)},
            {s(MovieField::MovieSet), movie->set().name} //
        });
        callback();
    }

    return csv.csv();
}

QVector<QString> CsvMovieExport::fieldsToStrings() const
{
    QVector<QString> out;
    for (const MovieField field : asConst(m_fields)) {
        out << fieldToString(field);
    }
    return out;
}

QString CsvMovieExport::fieldToString(MovieField field) const
{
    switch (field) {
    case MovieField::Imdbid: return "imdb_id";
    case MovieField::Tmdbid: return "tmdb_id";
    case MovieField::Title: return "title";
    case MovieField::OriginalTitle: return "original_title";
    case MovieField::SortTitle: return "sort_title";
    case MovieField::Overview: return "overview";
    case MovieField::Outline: return "outline";
    case MovieField::Ratings: return "ratings";
    case MovieField::UserRating: return "user_rating";
    case MovieField::IsImdbTop250: return "top250";
    case MovieField::ReleaseDate: return "release_date";
    case MovieField::Tagline: return "tagline";
    case MovieField::Runtime: return "runtime";
    case MovieField::Certification: return "certification";
    case MovieField::Writers: return "writers";
    case MovieField::Directors: return "directors";
    case MovieField::Genres: return "genres";
    case MovieField::Countries: return "countries";
    case MovieField::Studios: return "studios";
    case MovieField::Tags: return "tags";
    case MovieField::Trailer: return "trailers";
    case MovieField::Actors: return "actors";
    case MovieField::PlayCount: return "playcount";
    case MovieField::LastPlayed: return "last_played";
    case MovieField::MovieSet: return "movie_set";
    };
    return "unknown";
}

CsvTvShowExport::CsvTvShowExport(QVector<CsvTvShowExport::Field> fields, QObject* parent) :
    CsvMediaExport(parent), m_fields{fields}
{
}

QString CsvTvShowExport::exportTvShows(const QVector<TvShow*>& shows, std::function<void()> callback)
{
    CsvExport csv;
    csv.setFieldsInOrder(fieldsToStrings());
    csv.setSeparator(m_separator);
    csv.setReplacement(m_replacement);

    const auto s = [this](Field field) { return fieldToString(field); };

    csv.writeHeader();

    for (TvShow* show : shows) {
        csv.addRow({
            {s(Field::ShowTmdbId), show->tmdbId().toString()},
            {s(Field::ShowImdbId), show->imdbId().toString()},
            {s(Field::ShowTvDbId), show->tvdbId().toString()},
            {s(Field::ShowTvMazeId), show->tvmazeId().toString()},
            {s(Field::ShowTitle), show->title()},
            {s(Field::ShowFirstAired), show->firstAired().toString(Qt::ISODate)},
            {s(Field::ShowNetwork), show->network()},
            {s(Field::ShowGenres), show->genres().join(", ")},
            {s(Field::ShowRuntime), QString::number(show->runtime().count())},
            {s(Field::ShowRatings), ratingsToString(show->ratings())},
            {s(Field::ShowUserRating), QString::number(show->userRating())} //
        });

        callback();
    }

    return csv.csv();
}

QVector<QString> CsvTvShowExport::fieldsToStrings() const
{
    QVector<QString> out;
    for (const Field field : asConst(m_fields)) {
        out << fieldToString(field);
    }
    return out;
}

QString CsvTvShowExport::fieldToString(CsvTvShowExport::Field field) const
{
    switch (field) {
    case Field::ShowImdbId: return "show_imdb_id";
    case Field::ShowTmdbId: return "show_tmdb_id";
    case Field::ShowTvMazeId: return "show_tvmaze_id";
    case Field::ShowTvDbId: return "show_tvdb_id";
    case Field::ShowFirstAired: return "show_first_aired";
    case Field::ShowTitle: return "show_title";
    case Field::ShowNetwork: return "show_network";
    case Field::ShowGenres: return "show_genres";
    case Field::ShowRuntime: return "show_runtime";
    case Field::ShowRatings: return "show_ratings";
    case Field::ShowUserRating: return "show_user_rating";
    }
    return "unknown";
}


CsvTvEpisodeExport::CsvTvEpisodeExport(QVector<CsvTvEpisodeExport::Field> fields, QObject* parent) :
    CsvMediaExport(parent), m_fields{fields}
{
}

QString CsvTvEpisodeExport::exportTvEpisodes(const QVector<TvShow*>& shows, std::function<void()> callback)
{
    CsvExport csv;
    csv.setFieldsInOrder(fieldsToStrings());
    csv.setSeparator(m_separator);
    csv.setReplacement(m_replacement);

    const auto s = [this](Field field) { return fieldToString(field); };

    csv.writeHeader();

    for (TvShow* show : shows) {
        for (TvShowEpisode* episode : asConst(show->episodes())) {
            csv.addRow({
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
                {s(Field::EpisodeActors), actorsToString(episode->actors())} //
            });
        }
        callback();
    }

    return csv.csv();
}

QVector<QString> CsvTvEpisodeExport::fieldsToStrings() const
{
    QVector<QString> out;
    for (const Field field : asConst(m_fields)) {
        out << fieldToString(field);
    }
    return out;
}

QString CsvTvEpisodeExport::fieldToString(CsvTvEpisodeExport::Field field) const
{
    switch (field) {
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
    }
    return "unknown";
}

CsvConcertExport::CsvConcertExport(QVector<CsvConcertExport::Field> fields, QObject* parent) :
    CsvMediaExport(parent), m_fields{fields}
{
}

QString CsvConcertExport::exportConcerts(const QVector<Concert*>& concerts, std::function<void()> callback)
{
    CsvExport csv;
    csv.setFieldsInOrder(fieldsToStrings());
    csv.setSeparator(m_separator);
    csv.setReplacement(m_replacement);

    const auto s = [this](Field field) { return fieldToString(field); };

    csv.writeHeader();

    for (Concert* concert : asConst(concerts)) {
        csv.addRow({
            {s(Field::TmdbId), concert->tmdbId().toString()},
            {s(Field::ImdbId), concert->imdbId().toString()},
            {s(Field::Title), concert->name()},
            {s(Field::Artist), concert->artist()},
            {s(Field::Album), concert->album()},
            {s(Field::Overview), concert->overview()},
            {s(Field::Ratings), ratingsToString(concert->ratings())},
            {s(Field::UserRating), QString::number(concert->userRating())},
            {s(Field::ReleaseDate), concert->released().toString(Qt::ISODate)},
            {s(Field::Tagline), concert->tagline()},
            {s(Field::Runtime), QString::number(concert->runtime().count())},
            {s(Field::Certification), concert->certification().toString()},
            {s(Field::Genres), concert->genres().join(", ")},
            {s(Field::Tags), concert->tags().join(", ")},
            {s(Field::TrailerUrl), concert->trailer().toString()},
            {s(Field::Playcount), QString::number(concert->playcount())},
            {s(Field::LastPlayed), concert->lastPlayed().toString(Qt::ISODate)} //

        });
        callback();
    }

    return csv.csv();
}

QVector<QString> CsvConcertExport::fieldsToStrings() const
{
    QVector<QString> out;
    for (const Field field : asConst(m_fields)) {
        out << fieldToString(field);
    }
    return out;
}

QString CsvConcertExport::fieldToString(CsvConcertExport::Field field) const
{
    switch (field) {
    case Field::TmdbId: return "tmdb_id";
    case Field::ImdbId: return "imdb_id";
    case Field::Title: return "title";
    case Field::Artist: return "artist";
    case Field::Album: return "album";
    case Field::Overview: return "overview";
    case Field::Ratings: return "ratings";
    case Field::UserRating: return "user_rating";
    case Field::ReleaseDate: return "release_date";
    case Field::Tagline: return "tagline";
    case Field::Runtime: return "runtime";
    case Field::Certification: return "certification";
    case Field::Genres: return "genres";
    case Field::Tags: return "tags";
    case Field::TrailerUrl: return "trailer_url";
    case Field::Playcount: return "playcount";
    case Field::LastPlayed: return "last_played";
    }
    return "unknown";
}

CsvArtistExport::CsvArtistExport(QVector<CsvArtistExport::Field> fields, QObject* parent) :
    CsvMediaExport(parent), m_fields{fields}
{
}

QString CsvArtistExport::exportArtists(const QVector<Artist*>& artists, std::function<void()> callback)
{
    CsvExport csv;
    csv.setFieldsInOrder(fieldsToStrings());
    csv.setSeparator(m_separator);
    csv.setReplacement(m_replacement);

    const auto s = [this](Field field) { return fieldToString(field); };

    csv.writeHeader();

    for (Artist* artist : asConst(artists)) {
        csv.addRow({
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
            {s(Field::ArtistAllMusicId), artist->allMusicId().toString()} //
        });
        callback();
    }

    return csv.csv();
}

QVector<QString> CsvArtistExport::fieldsToStrings() const
{
    QVector<QString> out;
    for (const Field field : asConst(m_fields)) {
        out << fieldToString(field);
    }
    return out;
}

QString CsvArtistExport::fieldToString(CsvArtistExport::Field field) const
{
    switch (field) {
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
    }
}


CsvAlbumExport::CsvAlbumExport(QVector<CsvAlbumExport::Field> fields, QObject* parent) :
    CsvMediaExport(parent), m_fields{fields}
{
}

QString CsvAlbumExport::exportAlbumsOfArtists(const QVector<Artist*>& artists, std::function<void()> callback)
{
    CsvExport csv;
    csv.setFieldsInOrder(fieldsToStrings());
    csv.setSeparator(m_separator);
    csv.setReplacement(m_replacement);

    const auto s = [this](Field field) { return fieldToString(field); };

    csv.writeHeader();

    for (Artist* artist : asConst(artists)) {
        const auto albums = artist->albums();
        for (Album* album : albums) {
            csv.addRow({
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
                {s(Field::AlbumAllMusicId), album->allMusicId().toString()} //
            });
        }
        callback();
    }

    return csv.csv();
}

QVector<QString> CsvAlbumExport::fieldsToStrings() const
{
    QVector<QString> out;
    for (const Field field : asConst(m_fields)) {
        out << fieldToString(field);
    }
    return out;
}

QString CsvAlbumExport::fieldToString(CsvAlbumExport::Field field) const
{
    switch (field) {
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
    }
}

const QString& CsvExport::csv() const
{
    return m_csv;
}

void CsvExport::writeHeader()
{
    QVector<QString>::const_iterator i = m_fieldsInOrder.cbegin();
    writeEscaped(*i);
    ++i;

    for (; i != m_fieldsInOrder.cend(); ++i) {
        m_csv.append(m_separator);
        writeEscaped(*i);
    }
    m_csv.append('\n');
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
        m_csv.append(m_separator);
        writeEscaped(values.value(*i));
    }
    m_csv.append('\n');
}

void CsvExport::writeEscaped(const QString& text)
{
    if (!text.contains(m_separator) && !text.contains("\n")) {
        m_csv.append(text);
        return;
    }

    m_csv.append(QString(text)
                     .replace(m_separator, m_replacement)
                     .replace("\r\n", "\\n")
                     .replace("\n", "\\n")
                     .replace("\r", "\\n"));
}

} // namespace mediaelch
