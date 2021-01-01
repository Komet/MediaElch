#include "export/CsvExport.h"

#include "movies/Movie.h"

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

    csv.addRow({
        {"imdbid", "IMDb ID"}, //
        {"tmdbid", "TMDb ID"}, //
        {"title", "Title"},    //
        {"originalTitle", "Original Title"},
        {"sortTitle", "Sort Title"},
        {"overview", "Overview"},
        {"outline", "Outline"},
        {"rating", "Rating"},
        {"userRating", "User Rating"},
        {"top250", "Top 250"},
        {"releaseDate", "Release Date"},
        {"tagline", "Tagline"},
        {"runtime", "Runtime"},
        {"certification", "Certfication"},
        {"writers", "Writers"},
        {"directors", "Directors"},
        {"genres", "Genres"},
        {"countries", "Countries"},
        {"studios", "Studios"},
        {"tags", "Tags"},
        {"trailers", "Trailers"},
        {"actors", "Actors"},
        {"playcount", "Playcount"},
        {"lastPlayed", "Last Played"},
        {"movieSet", "MovieSet"} //
    });

    for (Movie* movie : asConst(movies)) {
        csv.addRow({
            {"imdbid", movie->imdbId().toString()},
            {"tmdbid", movie->tmdbId().toString()},
            {"title", movie->name()},
            {"originalTitle", movie->originalName()},
            {"sortTitle", movie->sortTitle()},
            {"overview", movie->overview()},
            {"outline", movie->outline()},
            {"rating", ratingsToString(movie->ratings())},
            {"userRating", QString::number(movie->userRating())},
            {"top250", QString::number(movie->top250())},
            {"releaseDate", movie->released().isValid() ? movie->released().toString(Qt::ISODate) : ""},
            {"tagline", movie->tagline()},
            {"runtime", QString::number(movie->runtime().count())},
            {"certification", movie->certification().toString()},
            {"writers", movie->writer()},
            {"directors", movie->director()},
            {"genres", movie->genres().join(", ")},
            {"countries", movie->countries().join(", ")},
            {"studios", movie->studios().join(", ")},
            {"tags", movie->tags().join(", ")},
            {"trailers", movie->trailer().toString()},
            {"actors", actorsToString(movie->actors())},
            {"playcount", QString::number(movie->playcount())},
            {"lastPlayed", movie->lastPlayed().toString(Qt::ISODate)},
            {"movieSet", movie->set().name} //
        });
        callback();
    }

    return csv.csv();
}

QVector<QString> CsvMovieExport::fieldsToStrings() const
{
    const static QMap<MovieField, QString> fieldToString = {
        {MovieField::Imdbid, "imdbid"},
        {MovieField::Tmdbid, "tmdbid"},
        {MovieField::Title, "title"},
        {MovieField::OriginalTitle, "originalTitle"},
        {MovieField::SortTitle, "sortTitle"},
        {MovieField::Overview, "overview"},
        {MovieField::Outline, "outline"},
        {MovieField::Ratings, "rating"},
        {MovieField::UserRating, "userRating"},
        {MovieField::IsImdbTop250, "top250"},
        {MovieField::ReleaseDate, "releaseDate"},
        {MovieField::Tagline, "tagline"},
        {MovieField::Runtime, "runtime"},
        {MovieField::Certification, "certification"},
        {MovieField::Writers, "writers"},
        {MovieField::Directors, "directors"},
        {MovieField::Genres, "genres"},
        {MovieField::Countries, "countries"},
        {MovieField::Studios, "studios"},
        {MovieField::Tags, "tags"},
        {MovieField::Trailer, "trailers"},
        {MovieField::Actors, "actors"},
        {MovieField::PlayCount, "playcount"},
        {MovieField::LastPlayed, "lastPlayed"},
        {MovieField::MovieSet, "movieSet"} //
    };

    QVector<QString> out;
    for (const MovieField field : asConst(m_fields)) {
        out << fieldToString.value(field);
    }
    return out;
}

QString CsvMovieExport::ratingsToString(const QVector<Rating>& ratings) const
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

QString CsvMovieExport::actorsToString(const QVector<Actor*>& actors) const
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

const QString& CsvExport::csv() const
{
    return m_csv;
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

    m_csv.append(QString(text).replace(m_separator, m_replacement).replace("\n", "\\n"));
}

} // namespace mediaelch
