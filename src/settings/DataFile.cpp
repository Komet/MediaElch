#include "DataFile.h"

#include <QDebug>
#include <QFileInfo>
#include <QStringList>
#include <utility>

#include "file/FilenameUtils.h"
#include "globals/Globals.h"
#include "globals/Helper.h"

/**
 * \brief DataFile::DataFile
 * \param type Type of this File (DataFileType)
 * \param fileName Name of this file
 * \param pos Position
 */
DataFile::DataFile(DataFileType type, QString fileName, int pos) :
    m_fileName{std::move(fileName)}, m_pos{pos}, m_type{type}
{
}

/**
 * \brief Filename
 * \return Filename
 */
QString DataFile::fileName() const
{
    return m_fileName;
}

void DataFile::setFileName(QString fileName)
{
    m_fileName = fileName;
}

/**
 * \brief Position of this file
 */
int DataFile::pos() const
{
    return m_pos;
}

/**
 * \brief Constructs a filename to save
 * \param fileName File name
 * \param season Season number
 */
QString DataFile::saveFileName(const QString& fileName, SeasonNumber season, bool stacked)
{
    if (type() == DataFileType::MovieSetBackdrop || type() == DataFileType::MovieSetPoster) {
        QString newFileName = m_fileName;
        newFileName.replace("<setName>", fileName);
        helper::sanitizeFileName(newFileName);
        return newFileName;
    }

    QFileInfo fi(fileName);
    QString newFileName = m_fileName;

    QString baseName = fi.completeBaseName();
    if (stacked) {
        baseName = mediaelch::file::stackedBaseName(fileName);
    }
    newFileName.replace("<baseFileName>", baseName);

    if (season != SeasonNumber::NoSeason) {
        if (season == SeasonNumber::SpecialsSeason) {
            newFileName.replace("<seasonNumber>", "-specials");
        } else {
            newFileName.replace("<seasonNumber>", season.toPaddedString());
        }
    }

    return newFileName;
}

/**
 * \brief Returns the type of this file
 * \return Type
 */
DataFileType DataFile::type() const
{
    return m_type;
}

/**
 * \brief Comparator
 * \return a lessThan b
 */
bool DataFile::lessThan(DataFile a, DataFile b)
{
    return a.pos() < b.pos();
}

DataFileType DataFile::dataFileTypeForImageType(ImageType imageType)
{
    switch (imageType) {
    case ImageType::MovieBackdrop: return DataFileType::MovieBackdrop;
    case ImageType::MovieBanner: return DataFileType::MovieBanner;
    case ImageType::MovieCdArt: return DataFileType::MovieCdArt;
    case ImageType::MovieClearArt: return DataFileType::MovieClearArt;
    case ImageType::MovieLogo: return DataFileType::MovieLogo;
    case ImageType::MoviePoster: return DataFileType::MoviePoster;
    case ImageType::MovieThumb: return DataFileType::MovieThumb;

    case ImageType::ConcertBackdrop: return DataFileType::ConcertBackdrop;
    case ImageType::ConcertCdArt: return DataFileType::ConcertCdArt;
    case ImageType::ConcertClearArt: return DataFileType::ConcertClearArt;
    case ImageType::ConcertLogo: return DataFileType::ConcertLogo;
    case ImageType::ConcertPoster: return DataFileType::ConcertPoster;

    case ImageType::TvShowBackdrop: return DataFileType::TvShowBackdrop;
    case ImageType::TvShowBanner: return DataFileType::TvShowBanner;
    case ImageType::TvShowCharacterArt: return DataFileType::TvShowCharacterArt;
    case ImageType::TvShowClearArt: return DataFileType::TvShowClearArt;
    case ImageType::TvShowLogos: return DataFileType::TvShowLogo;
    case ImageType::TvShowPoster: return DataFileType::TvShowPoster;
    case ImageType::TvShowThumb: return DataFileType::TvShowThumb;

    case ImageType::TvShowSeasonBackdrop: return DataFileType::TvShowSeasonBackdrop;
    case ImageType::TvShowSeasonPoster: return DataFileType::TvShowSeasonPoster;
    case ImageType::TvShowSeasonBanner: return DataFileType::TvShowSeasonBanner;
    case ImageType::TvShowSeasonThumb: return DataFileType::TvShowSeasonThumb;

    case ImageType::AlbumCdArt: return DataFileType::AlbumCdArt;
    case ImageType::AlbumThumb: return DataFileType::AlbumThumb;

    case ImageType::ArtistFanart: return DataFileType::ArtistFanart;
    case ImageType::ArtistLogo: return DataFileType::ArtistLogo;
    case ImageType::ArtistThumb: return DataFileType::ArtistThumb;

    default: return DataFileType::NoType;
    }
}
