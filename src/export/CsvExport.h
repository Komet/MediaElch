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
class TvShow;
class Concert;
class Artist;

namespace mediaelch {

class CsvMediaExport : public QObject
{
    Q_OBJECT

public:
    explicit CsvMediaExport(QObject* parent = nullptr) : QObject(parent) {}

public:
    void setSeparator(QString separator) { m_separator = std::move(separator); }
    void setReplacement(QString replacement) { m_replacement = std::move(replacement); }

protected:
    QString m_separator = "\t";
    QString m_replacement = " ";
};

class CsvMovieExport final : public CsvMediaExport
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
    ~CsvMovieExport() override = default;

public:
    /// \brief Exports the given movies
    ELCH_NODISCARD QString exportMovies(const QVector<Movie*>& movies, std::function<void()> callback);

private:
    QVector<QString> fieldsToStrings() const;
    QString fieldToString(MovieField field) const;

private:
    QVector<MovieField> m_fields;
};


class CsvTvExport final : public CsvMediaExport
{
    Q_OBJECT

public:
    enum class TvField
    {
        ShowImdbId = 1,
        ShowTmdbId,
        ShowTvDbId,
        ShowTvMazeId,
        ShowTitle,
        ShowFirstAired,
        ShowNetwork,
        ShowGenres,
        ShowRuntime,
        ShowRatings,
        ShowUserRating,
        EpisodeSeason,
        EpisodeNumber,
        EpisodeImdbId,
        EpisodeTmdbId,
        EpisodeTvDbId,
        EpisodeTvMazeId,
        EpisodeFirstAired,
        EpisodeTitle,
        EpisodeOverview,
        EpisodeUserRating,
        EpisodeWriters,
        EpisodeDirectors,
        EpisodeActors
    };

public:
    explicit CsvTvExport(QVector<TvField> fields, QObject* parent = nullptr);
    ~CsvTvExport() override = default;

public:
    /// \brief Exports the given TV shows and their episodes
    /// \param callback Called after each TV show
    ELCH_NODISCARD QString exportTvShows(const QVector<TvShow*>& movies, std::function<void()> callback);

private:
    QVector<QString> fieldsToStrings() const;
    QString fieldToString(TvField field) const;

private:
    QVector<TvField> m_fields;
};


class CsvConcertExport final : public CsvMediaExport
{
    Q_OBJECT

public:
    enum class Field
    {
        TmdbId = 1,
        ImdbId,
        Title,
        Artist,
        Album,
        Overview,
        Ratings,
        UserRating,
        ReleaseDate,
        Tagline,
        Runtime,
        Certification,
        Genres,
        Tags,
        TrailerUrl,
        Playcount,
        LastPlayed
    };

public:
    explicit CsvConcertExport(QVector<Field> fields, QObject* parent = nullptr);
    ~CsvConcertExport() override = default;


public:
    /// \brief Exports the given movies
    ELCH_NODISCARD QString exportConcerts(const QVector<Concert*>& concerts, std::function<void()> callback);

private:
    QVector<QString> fieldsToStrings() const;
    QString fieldToString(Field field) const;

private:
    QVector<Field> m_fields;
};


class CsvArtistExport final : public CsvMediaExport
{
    Q_OBJECT

public:
    enum class Field
    {
        ArtistName = 1,
        ArtistGenres,
        ArtistStyles,
        ArtistMoods,
        ArtistYearsActive,
        ArtistFormed,
        ArtistBiography,
        ArtistBorn,
        ArtistDied,
        ArtistDisbanded,
        ArtistMusicBrainzId,
        ArtistAllMusicId
    };

public:
    explicit CsvArtistExport(QVector<Field> fields, QObject* parent = nullptr);
    ~CsvArtistExport() override = default;


public:
    /// \brief Exports the given artists
    ELCH_NODISCARD QString exportArtists(const QVector<Artist*>& artists, std::function<void()> callback);

private:
    QVector<QString> fieldsToStrings() const;
    QString fieldToString(Field field) const;

private:
    QVector<Field> m_fields;
};


class CsvAlbumExport final : public CsvMediaExport
{
    Q_OBJECT

public:
    enum class Field
    {
        ArtistName = 1,
        AlbumTitle,
        AlbumArtistName,
        AlbumGenres,
        AlbumStyles,
        AlbumMoods,
        AlbumReview,
        AlbumReleaseDate,
        AlbumLabel,
        AlbumRating,
        AlbumYear,
        AlbumMusicBrainzId,
        AlbumMusicBrainzReleaseGroupId,
        AlbumAllMusicId
    };

public:
    explicit CsvAlbumExport(QVector<Field> fields, QObject* parent = nullptr);
    ~CsvAlbumExport() override = default;


public:
    /// \brief Exports the albums of the given artists
    ELCH_NODISCARD QString exportAlbumsOfArtists(const QVector<Artist*>& artists, std::function<void()> callback);

private:
    QVector<QString> fieldsToStrings() const;
    QString fieldToString(Field field) const;

private:
    QVector<Field> m_fields;
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

    /// \brief Writes a CSV header using the given fieldsInOrder
    void writeHeader();
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
