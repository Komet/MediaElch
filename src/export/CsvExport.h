#pragma once

#include "utils/Meta.h"

#include <QMap>
#include <QString>
#include <QTextStream>
#include <QVector>
#include <functional>

struct Actor;
class Movie;
class TvShow;
class Concert;
class Artist;

namespace mediaelch {

class CsvMediaExport
{
public:
    explicit CsvMediaExport(QTextStream& outStream) : m_out{outStream} {}

public:
    void setSeparator(QString separator) { m_separator = std::move(separator); }
    void setReplacement(QString replacement) { m_replacement = std::move(replacement); }

protected:
    QTextStream& m_out;
    QString m_separator = "\t";
    QString m_replacement = " ";
};


class CsvMovieExport final : public CsvMediaExport
{
public:
    // TODO: Maybe have this as global enums?
    enum class Field
    {
        ImdbId = 1,
        TmdbId,
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
        MovieSet,
        Filenames,
        Directory,
        StreamDetails_Video_DurationInSeconds,
        StreamDetails_Video_Aspect,
        StreamDetails_Video_Width,
        StreamDetails_Video_Height,
        StreamDetails_Video_Codec,
        StreamDetails_Audio_Language,
        StreamDetails_Audio_Codec,
        StreamDetails_Audio_Channels,
        StreamDetails_Subtitle_Language,
        Type,         // @since 2.8.17
        LastModified, // @since 2.8.17
        WikidataId,   // @since 2.10.1
        TvShowLinks,  // @since 2.10.8
    };

public:
    explicit CsvMovieExport(QTextStream& outStream, QVector<Field> fields);

public:
    /// \brief Exports the given movies
    void exportMovies(const QVector<Movie*>& movies, std::function<void()> callback);
    /// \brief Returns a string representation of the field that can be used for serializing.
    static QString fieldToString(Field field);

private:
    QVector<QString> fieldsToStrings() const;

private:
    QVector<Field> m_fields;
};


class CsvTvShowExport final : public CsvMediaExport
{
public:
    enum class Field
    {
        ShowImdbId = 1,
        ShowTmdbId,
        ShowTvDbId,
        ShowTvMazeId,
        ShowTitle,
        ShowSortTitle,
        ShowOriginalTitle,
        ShowFirstAired,
        ShowNetwork,
        ShowGenres,
        ShowCertification,
        ShowActors,
        ShowTags,
        ShowRuntime,
        ShowRatings,
        ShowUserRating,
        ShowIsImdbTop250,
        ShowOverview,
        ShowDirectory,
        Type, // @since 2.8.17
    };

public:
    explicit CsvTvShowExport(QTextStream& outStream, QVector<Field> fields);

public:
    /// \brief Exports the given TV shows.
    /// \param callback Called after each TV show
    void exportTvShows(const QVector<TvShow*>& shows, std::function<void()> callback);
    /// \brief Returns a string representation of the field that can be used for serializing.
    static QString fieldToString(Field field);

private:
    QVector<QString> fieldsToStrings() const;

private:
    QVector<Field> m_fields;
};


class CsvTvEpisodeExport final : public CsvMediaExport
{
public:
    enum class Field
    {
        ShowImdbId = 1,
        ShowTmdbId,
        ShowTvDbId,
        ShowTvMazeId,
        ShowTitle,
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
        EpisodeActors,
        EpisodeFilenames,
        EpisodeDirectory,
        EpisodeStreamDetails_Video_DurationInSeconds,
        EpisodeStreamDetails_Video_Aspect,
        EpisodeStreamDetails_Video_Width,
        EpisodeStreamDetails_Video_Height,
        EpisodeStreamDetails_Video_Codec,
        EpisodeStreamDetails_Audio_Language,
        EpisodeStreamDetails_Audio_Codec,
        EpisodeStreamDetails_Audio_Channels,
        EpisodeStreamDetails_Subtitle_Language,
        Type, // @since 2.8.17
    };

public:
    explicit CsvTvEpisodeExport(QTextStream& outStream, QVector<Field> fields);

public:
    /// \brief Exports the episodes of the given TV shows.
    /// \param callback Called after each TV show
    void exportEpisodes(const QVector<TvShow*>& shows, std::function<void()> callback);
    /// \brief Returns a string representation of the field that can be used for serializing.
    static QString fieldToString(Field field);

private:
    QVector<QString> fieldsToStrings() const;

private:
    QVector<Field> m_fields;
};


class CsvConcertExport final : public CsvMediaExport
{
public:
    enum class Field
    {
        TmdbId = 1,
        ImdbId,
        Title,
        OriginalTitle,
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
        LastPlayed,
        Filenames,
        Directory,
        StreamDetails_Video_DurationInSeconds,
        StreamDetails_Video_Aspect,
        StreamDetails_Video_Width,
        StreamDetails_Video_Height,
        StreamDetails_Video_Codec,
        StreamDetails_Audio_Language,
        StreamDetails_Audio_Codec,
        StreamDetails_Audio_Channels,
        StreamDetails_Subtitle_Language,
        Type,         // @since 2.8.17
        LastModified, // @since 2.8.17
    };

public:
    explicit CsvConcertExport(QTextStream& outStream, QVector<Field> fields);

    /// \brief Exports the given movies
    void exportConcerts(const QVector<Concert*>& concerts, std::function<void()> callback);

    /// \brief Returns a string representation of the field that can be used for serializing.
    static QString fieldToString(Field field);

private:
    class FieldExport;
    QVector<QString> fieldsToStrings() const;

private:
    QVector<Field> m_fields;
};


class CsvArtistExport final : public CsvMediaExport
{
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
        ArtistAllMusicId,
        ArtistDirectory,
        Type, // @since 2.8.17
    };

public:
    explicit CsvArtistExport(QTextStream& outStream, QVector<Field> fields);

public:
    /// \brief Exports the given artists
    void exportArtists(const QVector<Artist*>& artists, std::function<void()> callback);
    /// \brief Returns a string representation of the field that can be used for serializing.
    static QString fieldToString(Field field);

private:
    QVector<QString> fieldsToStrings() const;

private:
    QVector<Field> m_fields;
};


class CsvAlbumExport final : public CsvMediaExport
{
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
        AlbumAllMusicId,
        AlbumDirectory,
        Type, // @since 2.8.17
    };

public:
    explicit CsvAlbumExport(QTextStream& outStream, QVector<Field> fields);

public:
    /// \brief Exports the albums of the given artists
    void exportAlbumsOfArtists(const QVector<Artist*>& artists, std::function<void()> callback);
    /// \brief Returns a string representation of the field that can be used for serializing.
    static QString fieldToString(Field field);

private:
    QVector<QString> fieldsToStrings() const;

private:
    QVector<Field> m_fields;
};


class CsvExport
{
public:
    explicit CsvExport(QTextStream& outStream) : m_out{outStream} {}

    void setFieldsInOrder(QVector<QString> fieldsInOrder) { m_fieldsInOrder = std::move(fieldsInOrder); }
    void setSeparator(QString separator) { m_separator = std::move(separator); }
    void setReplacement(QString replacement) { m_replacement = std::move(replacement); }

    /// \brief Writes a CSV header using the given fieldsInOrder
    void writeHeader();
    void addRow(const QMap<QString, QString>& values);

private:
    void writeEscaped(const QString& text);

private:
    QTextStream& m_out;
    QVector<QString> m_fieldsInOrder;
    QString m_separator;
    QString m_replacement;
};

} // namespace mediaelch
