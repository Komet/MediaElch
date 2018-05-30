#include "DataFile.h"

#include <QDebug>
#include <QFileInfo>
#include <QRegExp>
#include <QStringList>

#include "globals/Globals.h"
#include "globals/Helper.h"

/**
 * @brief DataFile::DataFile
 * @param type Type of this File (DataFileType)
 * @param fileName Name of this file
 * @param pos Position
 */
DataFile::DataFile(int type, QString fileName, int pos) : m_fileName{fileName}, m_pos{pos}, m_type{type}
{
}

/**
 * @brief Filename
 * @return Filename
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
 * @brief Position of this file
 * @return
 */
int DataFile::pos() const
{
    return m_pos;
}

/**
 * @brief Constructs a filename to save
 * @param fileName File name
 * @param season Season number
 * @return
 */
QString DataFile::saveFileName(const QString &fileName, int season, bool stacked)
{
    if (type() == DataFileType::MovieSetBackdrop || type() == DataFileType::MovieSetPoster) {
        QString newFileName = m_fileName;
        return Helper::sanitizeFileName(newFileName.replace("<setName>", fileName));
    }

    QFileInfo fi(fileName);
    QString newFileName = m_fileName;

    QString baseName = fi.completeBaseName();
    if (stacked) {
        QString f = fileName;
        baseName = Helper::stackedBaseName(f);
    }
    newFileName.replace("<baseFileName>", baseName);

    if (season != -1) {
        if (season == 0) {
            newFileName.replace("<seasonNumber>", "-specials");
        } else {
            QString s = QString("%1").arg(season);
            if (season < 10)
                s.prepend("0");
            newFileName.replace("<seasonNumber>", s);
        }
    }

    return newFileName;
}

/**
 * @brief Returns the type of this file
 * @return Type
 */
int DataFile::type() const
{
    return m_type;
}

/**
 * @brief Comparator
 * @param a
 * @param b
 * @return a lessThan b
 */
bool DataFile::lessThan(DataFile a, DataFile b)
{
    return a.pos() < b.pos();
}

int DataFile::dataFileTypeForImageType(int imageType)
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

    default: return -1;
    }
}
