#pragma once

#include "settings/Settings.h"
#include "utils/Meta.h"

namespace mediaelch {
namespace exporter {

enum class CsvExportType
{
    Invalid = -1,
    Movies = 0,
    Concerts,
    TvShows,
    TvEpisodes,
    MusicArtists,
    MusicAlbums,
};

QString serializeExportType(CsvExportType type);
CsvExportType deserializeExportType(QString type);

QVector<CsvExportType> allCsvExportTypes();

QVector<CsvExportType> deserializeCsvExportTypes(QStringList types);
QStringList serializeCsvExportTypes(QVector<CsvExportType> types);


class CsvExportConfiguration
{
public:
    explicit CsvExportConfiguration(Settings& settings);
    virtual ~CsvExportConfiguration() = default;

    void init();

public:
    ELCH_NODISCARD virtual QString csvExportSeparator();
    virtual void setCsvExportSeparator(QString separator);

    ELCH_NODISCARD virtual QString csvExportReplacement();
    virtual void setCsvExportReplacement(QString replacement);

    ELCH_NODISCARD virtual QVector<CsvExportType> csvExportTypes();
    virtual void setCsvExportTypes(QVector<CsvExportType> types);

    ELCH_NODISCARD virtual QStringList csvExportMovieFields();
    virtual void setCsvExportMovieFields(QStringList fields);

    ELCH_NODISCARD virtual QStringList csvExportTvShowFields();
    virtual void setCsvExportTvShowFields(QStringList fields);

    ELCH_NODISCARD virtual QStringList csvExportTvEpisodeFields();
    virtual void setCsvExportTvEpisodeFields(QStringList fields);

    ELCH_NODISCARD virtual QStringList csvExportConcertFields();
    virtual void setCsvExportConcertFields(QStringList fields);

    ELCH_NODISCARD virtual QStringList csvExportMusicArtistFields();
    virtual void setCsvExportMusicArtistFields(QStringList fields);

    ELCH_NODISCARD virtual QStringList csvExportMusicAlbumFields();
    virtual void setCsvExportMusicAlbumFields(QStringList fields);

private:
    Settings& m_settings;
};

} // namespace exporter
} // namespace mediaelch
