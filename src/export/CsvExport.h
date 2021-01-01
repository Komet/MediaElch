#pragma once

#include "data/Rating.h"
#include "globals/Meta.h"

#include <QMap>
#include <QObject>
#include <QRegularExpression>
#include <QString>
#include <QVector>
#include <functional>

struct Actor;
class Movie;

namespace mediaelch {

class CsvMovieExport : public QObject
{
    Q_OBJECT

public:
    enum class MovieField
    {
        Imdbid = 1,
        Tmdbid,
        Title,
        OriginalTitle,
        SortTitle,
        Overview,
        Outline,
        Ratings,
        UserRating,
        IsImdbTop250,
        ReleaseDate,
        Tagline,
        Runtime,
        Certification,
        Writers,
        Directors,
        Genres,
        Countries,
        Studios,
        Tags,
        Trailer,
        Actors,
        PlayCount,
        LastPlayed,
        MovieSet
    };

public:
    explicit CsvMovieExport(QVector<MovieField> fields, QObject* parent = nullptr);

    void setSeparator(QString separator) { m_separator = std::move(separator); }
    void setReplacement(QString replacement) { m_replacement = std::move(replacement); }

public:
    /// \brief Exports the given movies
    ELCH_NODISCARD QString exportMovies(const QVector<Movie*>& movies, std::function<void()> callback);

private:
    QVector<QString> fieldsToStrings() const;
    QString ratingsToString(const QVector<Rating>& ratings) const;
    QString actorsToString(const QVector<Actor*>& actors) const;

private:
    QVector<MovieField> m_fields;
    QString m_separator = "\t";
    QString m_replacement = " ";
};


class CsvExport : public QObject
{
    Q_OBJECT

public:
    explicit CsvExport(QObject* parent = nullptr) : QObject(parent) {}

    void setFieldsInOrder(QVector<QString> fieldsInOrder) { m_fieldsInOrder = std::move(fieldsInOrder); }
    void setSeparator(QString separator) { m_separator = std::move(separator); }
    void setReplacement(QString replacement) { m_replacement = std::move(replacement); }

    const QString& csv() const;

    void addRow(const QMap<QString, QString>& values);

private:
    void writeEscaped(const QString& text);

private:
    QVector<QString> m_fieldsInOrder;
    QString m_separator;
    QString m_replacement;

    QString m_csv;
};

} // namespace mediaelch
