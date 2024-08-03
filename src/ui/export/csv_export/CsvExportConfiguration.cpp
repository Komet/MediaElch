#include "CsvExportConfiguration.h"

#include "settings/Settings.h"

#include <QString>

namespace {

static constexpr char moduleName[] = "csv_export";
static const Settings::Key KEY_CSV_EXPORT_SEPARATOR(moduleName, "CsvExport/Separator");
static const Settings::Key KEY_CSV_EXPORT_REPLACEMENT(moduleName, "CsvExport/Replacement");
static const Settings::Key KEY_CSV_EXPORT_TYPES(moduleName, "CsvExport/Types");
static const Settings::Key KEY_CSV_EXPORT_MOVIE_FIELDS(moduleName, "CsvExport/MovieFields");
static const Settings::Key KEY_CSV_EXPORT_TV_SHOW_FIELDS(moduleName, "CsvExport/TvShowFields");
static const Settings::Key KEY_CSV_EXPORT_TV_EPISODE_FIELDS(moduleName, "CsvExport/TvEpisodeFields");
static const Settings::Key KEY_CSV_EXPORT_CONCERT_FIELDS(moduleName, "CsvExport/ConcertFields");
static const Settings::Key KEY_CSV_EXPORT_MUSIC_ARTIST_FIELDS(moduleName, "CsvExport/MusicArtistFields");
static const Settings::Key KEY_CSV_EXPORT_MUSIC_ALBUM_FIELDS(moduleName, "CsvExport/MusicAlbumFields");

} // namespace


namespace mediaelch {
namespace exporter {

CsvExportType deserializeExportType(QString type)
{
    if (type == "movies") {
        return CsvExportType::Movies;
    }
    if (type == "concerts") {
        return CsvExportType::Concerts;
    }
    if (type == "tv_shows") {
        return CsvExportType::TvShows;
    }
    if (type == "tv_episodes") {
        return CsvExportType::TvEpisodes;
    }
    if (type == "music_artists") {
        return CsvExportType::MusicArtists;
    }
    if (type == "music_albums") {
        return CsvExportType::MusicAlbums;
    }
    return CsvExportType::Invalid;
}

QString serializeExportType(CsvExportType type)
{
    switch (type) {
    case CsvExportType::Invalid: break;
    case CsvExportType::Movies: return QStringLiteral("movies");
    case CsvExportType::Concerts: return QStringLiteral("concerts");
    case CsvExportType::TvShows: return QStringLiteral("tv_shows");
    case CsvExportType::TvEpisodes: return QStringLiteral("tv_episodes");
    case CsvExportType::MusicArtists: return QStringLiteral("music_artists");
    case CsvExportType::MusicAlbums: return QStringLiteral("music_albums");
    default: break;
    }
    MediaElch_Debug_Unreachable();
    return QStringLiteral("");
}

QVector<CsvExportType> allCsvExportTypes()
{
    return {
        CsvExportType::Movies,
        CsvExportType::Concerts,
        CsvExportType::TvShows,
        CsvExportType::TvEpisodes,
        CsvExportType::MusicArtists,
        CsvExportType::MusicAlbums,
    };
}

QVector<CsvExportType> deserializeCsvExportTypes(QStringList types)
{
    QVector<CsvExportType> list;
    for (QString type : types) {
        CsvExportType value = deserializeExportType(type);
        if (value != CsvExportType::Invalid) {
            list << value;
        }
    }
    return list;
}

QStringList serializeCsvExportTypes(QVector<CsvExportType> types)
{
    QStringList list;
    for (CsvExportType type : types) {
        QString value = serializeExportType(type);
        if (!value.isEmpty()) {
            list << value;
        }
    }
    return list;
}


CsvExportConfiguration::CsvExportConfiguration(Settings& settings) : m_settings{settings}
{
}

void CsvExportConfiguration::init()
{
    m_settings.setDefaultValue(KEY_CSV_EXPORT_SEPARATOR, ";");
    m_settings.setDefaultValue(KEY_CSV_EXPORT_REPLACEMENT, " ");
    m_settings.setDefaultValue(KEY_CSV_EXPORT_TYPES, serializeCsvExportTypes(allCsvExportTypes()).join(","));

    m_settings.setDefaultValue(KEY_CSV_EXPORT_MOVIE_FIELDS, "");
    m_settings.setDefaultValue(KEY_CSV_EXPORT_TV_SHOW_FIELDS, "");
    m_settings.setDefaultValue(KEY_CSV_EXPORT_TV_EPISODE_FIELDS, "");
    m_settings.setDefaultValue(KEY_CSV_EXPORT_CONCERT_FIELDS, "");
    m_settings.setDefaultValue(KEY_CSV_EXPORT_MUSIC_ARTIST_FIELDS, "");
    m_settings.setDefaultValue(KEY_CSV_EXPORT_MUSIC_ALBUM_FIELDS, "");
}

QString CsvExportConfiguration::csvExportSeparator()
{
    return m_settings.value(KEY_CSV_EXPORT_SEPARATOR).toString();
}

QString CsvExportConfiguration::csvExportReplacement()
{
    return m_settings.value(KEY_CSV_EXPORT_REPLACEMENT).toString();
}

QVector<CsvExportType> CsvExportConfiguration::csvExportTypes()
{
    return deserializeCsvExportTypes(m_settings.value(KEY_CSV_EXPORT_TYPES).toString().split(",", Qt::SkipEmptyParts));
}

QStringList CsvExportConfiguration::csvExportMovieFields()
{
    return m_settings.value(KEY_CSV_EXPORT_MOVIE_FIELDS).toString().split(",", Qt::SkipEmptyParts);
}

QStringList CsvExportConfiguration::csvExportTvShowFields()
{
    return m_settings.value(KEY_CSV_EXPORT_TV_SHOW_FIELDS).toString().split(",", Qt::SkipEmptyParts);
}

QStringList CsvExportConfiguration::csvExportTvEpisodeFields()
{
    return m_settings.value(KEY_CSV_EXPORT_TV_EPISODE_FIELDS).toString().split(",", Qt::SkipEmptyParts);
}

QStringList CsvExportConfiguration::csvExportConcertFields()
{
    return m_settings.value(KEY_CSV_EXPORT_CONCERT_FIELDS).toString().split(",", Qt::SkipEmptyParts);
}

QStringList CsvExportConfiguration::csvExportMusicArtistFields()
{
    return m_settings.value(KEY_CSV_EXPORT_MUSIC_ARTIST_FIELDS).toString().split(",", Qt::SkipEmptyParts);
}

QStringList CsvExportConfiguration::csvExportMusicAlbumFields()
{
    return m_settings.value(KEY_CSV_EXPORT_MUSIC_ALBUM_FIELDS).toString().split(",", Qt::SkipEmptyParts);
}

void CsvExportConfiguration::setCsvExportSeparator(QString separator)
{
    m_settings.setValue(KEY_CSV_EXPORT_SEPARATOR, separator);
}

void CsvExportConfiguration::setCsvExportReplacement(QString replacement)
{
    m_settings.setValue(KEY_CSV_EXPORT_REPLACEMENT, replacement);
}

void CsvExportConfiguration::setCsvExportTypes(QVector<CsvExportType> exportTypes)
{
    m_settings.setValue(KEY_CSV_EXPORT_TYPES, serializeCsvExportTypes(exportTypes).join(","));
}

void CsvExportConfiguration::setCsvExportMovieFields(QStringList fields)
{
    m_settings.setValue(KEY_CSV_EXPORT_MOVIE_FIELDS, fields.join(","));
}

void CsvExportConfiguration::setCsvExportTvShowFields(QStringList fields)
{
    m_settings.setValue(KEY_CSV_EXPORT_TV_SHOW_FIELDS, fields.join(","));
}

void CsvExportConfiguration::setCsvExportTvEpisodeFields(QStringList fields)
{
    m_settings.setValue(KEY_CSV_EXPORT_TV_EPISODE_FIELDS, fields.join(","));
}

void CsvExportConfiguration::setCsvExportConcertFields(QStringList fields)
{
    m_settings.setValue(KEY_CSV_EXPORT_CONCERT_FIELDS, fields.join(","));
}

void CsvExportConfiguration::setCsvExportMusicArtistFields(QStringList fields)
{
    m_settings.setValue(KEY_CSV_EXPORT_MUSIC_ARTIST_FIELDS, fields.join(","));
}

void CsvExportConfiguration::setCsvExportMusicAlbumFields(QStringList fields)
{
    m_settings.setValue(KEY_CSV_EXPORT_MUSIC_ALBUM_FIELDS, fields.join(","));
}


} // namespace exporter
} // namespace mediaelch
