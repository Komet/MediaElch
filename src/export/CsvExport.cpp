#include "export/CsvExport.h"

#include "movies/Movie.h"
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
    QObject(parent), m_fields{fields}
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

CsvTvExport::CsvTvExport(QVector<CsvTvExport::TvField> fields, QObject* parent) : QObject(parent), m_fields{fields}
{
}

QString CsvTvExport::exportTvShows(const QVector<TvShow*>& movies, std::function<void()> callback)
{
    CsvExport csv;
    csv.setFieldsInOrder(fieldsToStrings());
    csv.setSeparator(m_separator);
    csv.setReplacement(m_replacement);

    const auto s = [this](TvField field) { return fieldToString(field); };

    csv.writeHeader();

    for (TvShow* show : asConst(movies)) {
        for (TvShowEpisode* episode : asConst(show->episodes())) {
            csv.addRow({
                {s(TvField::ShowTmdbId), show->tmdbId().toString()},
                {s(TvField::ShowImdbId), show->imdbId().toString()},
                {s(TvField::ShowTvDbId), show->tvdbId().toString()},
                {s(TvField::ShowTvMazeId), show->tvmazeId().toString()},
                {s(TvField::ShowTitle), show->title()},
                {s(TvField::ShowFirstAired), show->firstAired().toString(Qt::ISODate)},
                {s(TvField::ShowNetwork), show->network()},
                {s(TvField::ShowGenres), show->genres().join(", ")},
                {s(TvField::ShowRuntime), QString::number(show->runtime().count())},
                {s(TvField::ShowRatings), ratingsToString(show->ratings())},
                {s(TvField::ShowUserRating), QString::number(show->userRating())},
                {s(TvField::EpisodeSeason), episode->seasonNumber().toString()},
                {s(TvField::EpisodeNumber), episode->episodeNumber().toString()},
                {s(TvField::EpisodeTmdbId), episode->tmdbId().toString()},
                {s(TvField::EpisodeImdbId), episode->imdbId().toString()},
                {s(TvField::EpisodeTvDbId), episode->tvdbId().toString()},
                {s(TvField::EpisodeTvMazeId), episode->tvmazeId().toString()},
                {s(TvField::EpisodeFirstAired), episode->firstAired().toString(Qt::ISODate)},
                {s(TvField::EpisodeTitle), episode->title()},
                {s(TvField::EpisodeOverview), episode->overview()},
                {s(TvField::EpisodeTitle), episode->title()},
                {s(TvField::EpisodeUserRating), QString::number(episode->userRating())},
                {s(TvField::EpisodeActors), actorsToString(episode->actors())} //
            });
        }
        callback();
    }

    return csv.csv();
}

QVector<QString> CsvTvExport::fieldsToStrings() const
{
    QVector<QString> out;
    for (const TvField field : asConst(m_fields)) {
        out << fieldToString(field);
    }
    return out;
}

QString CsvTvExport::fieldToString(CsvTvExport::TvField field) const
{
    switch (field) {
    case TvField::ShowImdbId: return "show_imdb_id";
    case TvField::ShowTmdbId: return "show_tmdb_id";
    case TvField::ShowTvMazeId: return "show_tvmaze_id";
    case TvField::ShowTvDbId: return "show_tvdb_id";
    case TvField::ShowFirstAired: return "show_first_aired";
    case TvField::ShowTitle: return "show_title";
    case TvField::ShowNetwork: return "show_network";
    case TvField::ShowGenres: return "show_genres";
    case TvField::ShowRuntime: return "show_runtime";
    case TvField::ShowRatings: return "show_ratings";
    case TvField::ShowUserRating: return "show_user_rating";
    case TvField::EpisodeSeason: return "episode_season";
    case TvField::EpisodeNumber: return "episode_number";
    case TvField::EpisodeImdbId: return "episode_imdb_id";
    case TvField::EpisodeTmdbId: return "episode_tmdb_id";
    case TvField::EpisodeTvDbId: return "episode_tvdb_id";
    case TvField::EpisodeTvMazeId: return "episode_tvmaze_id";
    case TvField::EpisodeFirstAired: return "episode_actors";
    case TvField::EpisodeTitle: return "episode_title";
    case TvField::EpisodeOverview: return "episode_overview";
    case TvField::EpisodeUserRating: return "episode_user_rating";
    case TvField::EpisodeDirectors: return "episode_directors";
    case TvField::EpisodeWriters: return "episode_writers";
    case TvField::EpisodeActors: return "episode_actors";
    }
    return "unknown";
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
