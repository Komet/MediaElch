#include "KodiXml.h"

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "image/Image.h"
#include "media_centers/kodi/AlbumXmlReader.h"
#include "media_centers/kodi/AlbumXmlWriter.h"
#include "media_centers/kodi/ArtistXmlReader.h"
#include "media_centers/kodi/ArtistXmlWriter.h"
#include "media_centers/kodi/ConcertXmlReader.h"
#include "media_centers/kodi/ConcertXmlWriter.h"
#include "media_centers/kodi/EpisodeXmlReader.h"
#include "media_centers/kodi/MovieXmlReader.h"
#include "media_centers/kodi/TvShowXmlReader.h"
#include "media_centers/kodi/v16/AlbumXmlWriterV16.h"
#include "media_centers/kodi/v16/ArtistXmlWriterV16.h"
#include "media_centers/kodi/v16/ConcertXmlWriterV16.h"
#include "media_centers/kodi/v16/EpisodeXmlWriterV16.h"
#include "media_centers/kodi/v16/MovieXmlWriterV16.h"
#include "media_centers/kodi/v16/TvShowXmlWriterV16.h"
#include "media_centers/kodi/v17/AlbumXmlWriterV17.h"
#include "media_centers/kodi/v17/ArtistXmlWriterV17.h"
#include "media_centers/kodi/v17/ConcertXmlWriterV17.h"
#include "media_centers/kodi/v17/EpisodeXmlWriterV17.h"
#include "media_centers/kodi/v17/MovieXmlWriterV17.h"
#include "media_centers/kodi/v17/TvShowXmlWriterV17.h"
#include "media_centers/kodi/v18/AlbumXmlWriterV18.h"
#include "media_centers/kodi/v18/ArtistXmlWriterV18.h"
#include "media_centers/kodi/v18/ConcertXmlWriterV18.h"
#include "media_centers/kodi/v18/EpisodeXmlWriterV18.h"
#include "media_centers/kodi/v18/MovieXmlWriterV18.h"
#include "media_centers/kodi/v18/TvShowXmlWriterV18.h"
#include "movies/Movie.h"
#include "settings/Settings.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"

#include <QApplication>
#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QXmlStreamWriter>
#include <array>
#include <memory>

KodiXml::KodiXml(QObject* parent)
{
    setParent(parent);
}

KodiXml::~KodiXml() = default;

void KodiXml::setVersion(mediaelch::KodiVersion version)
{
    m_version = version;
}

/// @brief Checks if our MediaCenterPlugin supports a feature
/// @param feature Feature to check
/// @return Feature is supported or not
bool KodiXml::hasFeature(MediaCenterFeature feature)
{
    Q_UNUSED(feature);
    return true;
}

QByteArray KodiXml::getMovieXml(Movie* movie)
{
    using namespace mediaelch;
    // @todo(bugwelle):
    // I'm fully aware that this is bad coding style but writing this clean
    // requires so much refactoring that writing this whole feature would be easier.
    // It's on my todo list to refactor this. Maybe into a Kodi factory.
    std::unique_ptr<kodi::MovieXmlWriter> writer;
    switch (m_version.version()) {
    case KodiVersion::v16: writer = std::make_unique<kodi::MovieXmlWriterV16>(*movie); break;
    case KodiVersion::v17: writer = std::make_unique<kodi::MovieXmlWriterV17>(*movie); break;
    case KodiVersion::v18: writer = std::make_unique<kodi::MovieXmlWriterV18>(*movie); break;
    }
    return writer->getMovieXml();
}

/// @brief Saves a movie (including images)
/// @param movie Movie to save
/// @return Saving success
/// @see KodiXml::writeMovieXml
bool KodiXml::saveMovie(Movie* movie)
{
    qDebug() << "Save movie as Kodi NFO file; movie: " << movie->name();
    QByteArray xmlContent = getMovieXml(movie);

    if (movie->files().empty()) {
        qWarning() << "Movie has no files";
        return false;
    }

    movie->setNfoContent(xmlContent);

    bool saved = false;
    QFileInfo fi(movie->files().at(0));
    for (auto dataFile : Settings::instance()->dataFiles(DataFileType::MovieNfo)) {
        QString saveFileName = dataFile.saveFileName(fi.fileName(), SeasonNumber::NoSeason, movie->files().count() > 1);
        QString saveFilePath = fi.absolutePath() + "/" + saveFileName;
        QDir saveFileDir = QFileInfo(saveFilePath).dir();
        if (!saveFileDir.exists()) {
            saveFileDir.mkpath(".");
        }
        QFile file(saveFilePath);
        qDebug() << "Saving to" << file.fileName();
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "File could not be openend";
        } else {
            file.write(xmlContent);
            file.close();
            saved = true;
        }
    }
    if (!saved) {
        return false;
    }

    for (const auto imageType : Movie::imageTypes()) {
        DataFileType dataFileType = DataFile::dataFileTypeForImageType(imageType);
        if (movie->images().imageHasChanged(imageType) && !movie->images().image(imageType).isNull()) {
            for (DataFile dataFile : Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName =
                    dataFile.saveFileName(fi.fileName(), SeasonNumber::NoSeason, movie->files().count() > 1);
                if (imageType == ImageType::MoviePoster
                    && (movie->discType() == DiscType::BluRay || movie->discType() == DiscType::Dvd)) {
                    saveFileName = "poster.jpg";
                }
                if (imageType == ImageType::MovieBackdrop
                    && (movie->discType() == DiscType::BluRay || movie->discType() == DiscType::Dvd)) {
                    saveFileName = "fanart.jpg";
                }
                QString path = getPath(movie);
                saveFile(path + "/" + saveFileName, movie->images().image(imageType));
            }
        }

        if (movie->images().imagesToRemove().contains(imageType)) {
            for (DataFile dataFile : Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName =
                    dataFile.saveFileName(fi.fileName(), SeasonNumber::NoSeason, movie->files().count() > 1);
                if (imageType == ImageType::MoviePoster
                    && (movie->discType() == DiscType::BluRay || movie->discType() == DiscType::Dvd)) {
                    saveFileName = "poster.jpg";
                }
                if (imageType == ImageType::MovieBackdrop
                    && (movie->discType() == DiscType::BluRay || movie->discType() == DiscType::Dvd)) {
                    saveFileName = "fanart.jpg";
                }
                QString path = getPath(movie);
                QFile(path + "/" + saveFileName).remove();
            }
        }
    }

    if (movie->inSeparateFolder() && !movie->files().isEmpty()) {
        for (const QString& file : movie->images().extraFanartsToRemove()) {
            QFile::remove(file);
        }
        QDir dir(QFileInfo(movie->files().first()).absolutePath() + "/extrafanart");
        if (!dir.exists() && !movie->images().extraFanartToAdd().isEmpty()) {
            QDir(QFileInfo(movie->files().first()).absolutePath()).mkdir("extrafanart");
        }
        for (const QByteArray& img : movie->images().extraFanartToAdd()) {
            int num = 1;
            while (QFileInfo(dir.absolutePath() + "/" + QString("fanart%1.jpg").arg(num)).exists()) {
                ++num;
            }
            saveFile(dir.absolutePath() + "/" + QString("fanart%1.jpg").arg(num), img);
        }
    }

    for (const Actor* actor : movie->actors()) {
        if (!actor->image.isNull()) {
            QDir dir;
            dir.mkdir(fi.absolutePath() + "/" + ".actors");
            QString actorName = actor->name;
            actorName = actorName.replace(" ", "_");
            saveFile(fi.absolutePath() + "/" + ".actors" + "/" + actorName + ".jpg", actor->image);
        }
    }

    for (Subtitle* subtitle : movie->subtitles()) {
        if (subtitle->changed()) {
            QString subFileName = fi.completeBaseName();
            if (!subtitle->language().isEmpty()) {
                subFileName.append("." + subtitle->language());
            }
            if (subtitle->forced()) {
                subFileName.append(".forced");
            }

            QStringList newFiles;
            for (const QString& subFile : subtitle->files()) {
                QFileInfo subFi(fi.absolutePath() + "/" + subFile);
                QString newFileName = subFileName + "." + subFi.suffix();
                QFile f(fi.absolutePath() + "/" + subFile);
                if (f.rename(fi.absolutePath() + "/" + newFileName)) {
                    newFiles << newFileName;
                } else {
                    qWarning() << "Could not rename" << subFi.absoluteFilePath() << "to"
                               << fi.absolutePath() + "/" + newFileName;
                    newFiles << subFi.fileName();
                }
            }
            subtitle->setFiles(newFiles);
        }
    }

    Manager::instance()->database()->update(movie);

    return true;
}

/**
 * @brief Tries to find an nfo file for the movie
 * @param movie Movie
 * @return Path to nfo file, if none found returns an empty string
 */
QString KodiXml::nfoFilePath(Movie* movie)
{
    QString nfoFile;
    if (movie->files().empty()) {
        qWarning() << "Movie has no files";
        return nfoFile;
    }
    QFileInfo fi(movie->files().at(0));
    if (!fi.isFile()) {
        qWarning() << "First file of the movie is not readable" << movie->files().at(0);
        return nfoFile;
    }

    for (DataFile dataFile : Settings::instance()->dataFiles(DataFileType::MovieNfo)) {
        QString file = dataFile.saveFileName(fi.fileName(), SeasonNumber::NoSeason, movie->files().count() > 1);
        QFileInfo nfoFi(fi.absolutePath() + "/" + file);
        if (nfoFi.exists()) {
            nfoFile = fi.absolutePath() + "/" + file;
            break;
        }
    }

    return nfoFile;
}

QString KodiXml::nfoFilePath(TvShowEpisode* episode)
{
    QString nfoFile;
    if (episode->files().empty()) {
        qWarning() << "[KodiXml] Episode has no files";
        return nfoFile;
    }
    QFileInfo fi(episode->files().at(0));
    if (!fi.isFile()) {
        qWarning() << "[KodiXml] First file of the episode is not readable" << episode->files().at(0);
        return nfoFile;
    }

    for (DataFile dataFile : Settings::instance()->dataFiles(DataFileType::TvShowEpisodeNfo)) {
        QString file = dataFile.saveFileName(fi.fileName(), SeasonNumber::NoSeason, episode->files().count() > 1);
        QFileInfo nfoFi(fi.absolutePath() + "/" + file);
        if (nfoFi.exists()) {
            nfoFile = fi.absolutePath() + "/" + file;
            break;
        }
    }

    return nfoFile;
}

QString KodiXml::nfoFilePath(TvShow* show)
{
    QString nfoFile;
    if (show->dir().isEmpty()) {
        qWarning() << "[KodiXml] Show dir is empty";
        return nfoFile;
    }

    for (DataFile dataFile : Settings::instance()->dataFiles(DataFileType::TvShowNfo)) {
        QFile file(show->dir() + "/" + dataFile.saveFileName(""));
        if (file.exists()) {
            nfoFile = file.fileName();
            break;
        }
    }

    return nfoFile;
}

/**
 * @brief Tries to find an nfo file for the concert
 * @param concert Concert
 * @return Path to nfo file, if none found returns an empty string
 */
QString KodiXml::nfoFilePath(Concert* concert)
{
    QString nfoFile;
    if (concert->files().empty()) {
        qWarning() << "[KodiXml] Concert has no files";
        return nfoFile;
    }
    QFileInfo fi(concert->files().at(0));
    if (!fi.isFile()) {
        qWarning() << "[KodiXml] First file of the concert is not readable" << concert->files().at(0);
        return nfoFile;
    }

    for (DataFile dataFile : Settings::instance()->dataFiles(DataFileType::ConcertNfo)) {
        QString file = dataFile.saveFileName(fi.fileName(), SeasonNumber::NoSeason, concert->files().count() > 1);
        QFileInfo nfoFi(fi.absolutePath() + "/" + file);
        if (nfoFi.exists()) {
            nfoFile = fi.absolutePath() + "/" + file;
            break;
        }
    }

    return nfoFile;
}

/**
 * @brief Loads movie infos (except images)
 * @param movie Movie to load
 * @return Loading success
 */
bool KodiXml::loadMovie(Movie* movie, QString initialNfoContent)
{
    movie->clear();
    movie->setChanged(false);

    QString nfoContent;
    if (initialNfoContent.isEmpty()) {
        QString nfoFile = nfoFilePath(movie);
        if (nfoFile.isEmpty()) {
            return false;
        }

        QFile file(nfoFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "[KodiXml] File" << nfoFile << "could not be opened for reading";
            return false;
        }
        nfoContent = QString::fromUtf8(file.readAll());
        movie->setNfoContent(nfoContent);
        file.close();
    } else {
        nfoContent = initialNfoContent;
    }

    QDomDocument domDoc;
    domDoc.setContent(nfoContent);

    mediaelch::kodi::MovieXmlReader reader(*movie);
    reader.parseNfoDom(domDoc);

    movie->setStreamDetailsLoaded(loadStreamDetails(movie->streamDetails(), domDoc));

    // Existence of images
    if (initialNfoContent.isEmpty()) {
        for (const auto imageType : Movie::imageTypes()) {
            movie->images().setHasImage(imageType, !imageFileName(movie, imageType).isEmpty());
        }
        movie->images().setHasExtraFanarts(!extraFanartNames(movie).isEmpty());
    }

    return true;
}

/// @brief Loads the stream details from the dom document
/// @param streamDetails StreamDetails object
/// @param domDoc Nfo document
/// @return Infos loaded
bool KodiXml::loadStreamDetails(StreamDetails* streamDetails, QDomDocument domDoc)
{
    streamDetails->clear();
    QDomNodeList elements = domDoc.elementsByTagName("streamdetails");
    if (elements.isEmpty()) {
        return false;
    }
    QDomElement elem = elements.at(0).toElement();
    loadStreamDetails(streamDetails, elem);
    return true;
}

void KodiXml::loadStreamDetails(StreamDetails* streamDetails, QDomElement elem)
{
    if (!elem.elementsByTagName("video").isEmpty()) {
        QDomElement videoElem = elem.elementsByTagName("video").at(0).toElement();

        std::array<StreamDetails::VideoDetails, 7> details{StreamDetails::VideoDetails::Codec,
            StreamDetails::VideoDetails::Aspect,
            StreamDetails::VideoDetails::Width,
            StreamDetails::VideoDetails::Height,
            StreamDetails::VideoDetails::DurationInSeconds,
            StreamDetails::VideoDetails::ScanType,
            StreamDetails::VideoDetails::StereoMode};

        for (const auto detail : details) {
            const QString detailStr = StreamDetails::detailToString(detail);
            QDomNodeList elements = videoElem.elementsByTagName(detailStr);
            if (!elements.isEmpty()) {
                streamDetails->setVideoDetail(detail, elements.at(0).toElement().text());
            }
        }
    }

    QDomNodeList audioElements = elem.elementsByTagName("audio");
    std::array<StreamDetails::AudioDetails, 3> audioDetails{StreamDetails::AudioDetails::Codec,
        StreamDetails::AudioDetails::Language,
        StreamDetails::AudioDetails::Channels};

    for (int i = 0, n = audioElements.count(); i < n; ++i) {
        QDomElement audioElem = audioElements.at(i).toElement();

        for (const auto detail : audioDetails) {
            const QString detailStr = StreamDetails::detailToString(detail);
            QDomNodeList detailElements = audioElem.elementsByTagName(detailStr);

            if (!detailElements.isEmpty()) {
                streamDetails->setAudioDetail(i, detail, detailElements.at(0).toElement().text());
            }
        }
    }

    QDomNodeList subtitleElements = elem.elementsByTagName("subtitle");
    std::array<StreamDetails::SubtitleDetails, 1> subtitleDetails{StreamDetails::SubtitleDetails::Language};

    for (int i = 0, n = subtitleElements.count(); i < n; ++i) {
        QDomElement subtitleElem = subtitleElements.at(i).toElement();
        if (!subtitleElem.elementsByTagName("file").isEmpty()) {
            continue;
        }
        for (const auto detail : subtitleDetails) {
            const auto detailStr = StreamDetails::detailToString(detail);
            if (!subtitleElem.elementsByTagName(detailStr).isEmpty()) {
                streamDetails->setSubtitleDetail(
                    i, detail, subtitleElem.elementsByTagName(detailStr).at(0).toElement().text());
            }
        }
    }
}

/// @brief Writes streamdetails to xml stream
/// @param xml XML Stream
/// @param streamDetails Stream Details object
void KodiXml::writeStreamDetails(QXmlStreamWriter& xml, StreamDetails* streamDetails)
{
    if (streamDetails->videoDetails().isEmpty() && streamDetails->audioDetails().isEmpty()
        && streamDetails->subtitleDetails().isEmpty()) {
        return;
    }

    xml.writeStartElement("fileinfo");
    xml.writeStartElement("streamdetails");

    xml.writeStartElement("video");
    QMapIterator<StreamDetails::VideoDetails, QString> itVideo(streamDetails->videoDetails());
    while (itVideo.hasNext()) {
        itVideo.next();
        if (itVideo.key() == StreamDetails::VideoDetails::Width && itVideo.value().toInt() == 0) {
            continue;
        }
        if (itVideo.key() == StreamDetails::VideoDetails::Height && itVideo.value().toInt() == 0) {
            continue;
        }
        if (itVideo.key() == StreamDetails::VideoDetails::DurationInSeconds && itVideo.value().toInt() == 0) {
            continue;
        }
        if (itVideo.value().isEmpty()) {
            continue;
        }

        QString value = itVideo.value();

        if (itVideo.key() == StreamDetails::VideoDetails::Aspect) {
            value = value.replace(",", ".");
        }

        xml.writeTextElement(StreamDetails::detailToString(itVideo.key()), value);
    }
    xml.writeEndElement();

    for (int i = 0, n = streamDetails->audioDetails().count(); i < n; ++i) {
        xml.writeStartElement("audio");
        QMapIterator<StreamDetails::AudioDetails, QString> itAudio(streamDetails->audioDetails().at(i));
        while (itAudio.hasNext()) {
            itAudio.next();
            if (itAudio.value() == "") {
                continue;
            }
            xml.writeTextElement(StreamDetails::detailToString(itAudio.key()), itAudio.value());
        }
        xml.writeEndElement();
    }

    for (int i = 0, n = streamDetails->subtitleDetails().count(); i < n; ++i) {
        xml.writeStartElement("subtitle");
        QMapIterator<StreamDetails::SubtitleDetails, QString> itSubtitle(streamDetails->subtitleDetails().at(i));
        while (itSubtitle.hasNext()) {
            itSubtitle.next();
            if (itSubtitle.value() == "") {
                continue;
            }
            xml.writeTextElement(StreamDetails::detailToString(itSubtitle.key()), itSubtitle.value());
        }
        xml.writeEndElement();
    }

    xml.writeEndElement();
    xml.writeEndElement();
}

void KodiXml::writeStreamDetails(QDomDocument& doc, const StreamDetails* streamDetails, QVector<Subtitle*> subtitles)
{
    removeChildNodes(doc, "fileinfo");

    if (streamDetails == nullptr
        || (streamDetails->videoDetails().isEmpty() && streamDetails->audioDetails().isEmpty()
            && streamDetails->subtitleDetails().isEmpty() && subtitles.isEmpty())) {
        return;
    }

    QDomElement elemFi = doc.createElement("fileinfo");
    QDomElement elemSd = doc.createElement("streamdetails");

    QDomElement elemVideo = doc.createElement("video");
    QMapIterator<StreamDetails::VideoDetails, QString> itVideo(streamDetails->videoDetails());
    while (itVideo.hasNext()) {
        itVideo.next();
        if (itVideo.key() == StreamDetails::VideoDetails::Width && itVideo.value().toInt() == 0) {
            continue;
        }
        if (itVideo.key() == StreamDetails::VideoDetails::Height && itVideo.value().toInt() == 0) {
            continue;
        }
        if (itVideo.key() == StreamDetails::VideoDetails::DurationInSeconds && itVideo.value().toInt() == 0) {
            continue;
        }
        if (itVideo.value().isEmpty()) {
            continue;
        }

        QString value = itVideo.value();

        if (itVideo.key() == StreamDetails::VideoDetails::Aspect) {
            value = value.replace(",", ".");
        }

        QDomElement elem = doc.createElement(StreamDetails::detailToString(itVideo.key()));
        elem.appendChild(doc.createTextNode(value));
        elemVideo.appendChild(elem);
    }
    elemSd.appendChild(elemVideo);

    for (int i = 0, n = streamDetails->audioDetails().count(); i < n; ++i) {
        QDomElement elemAudio = doc.createElement("audio");
        QMapIterator<StreamDetails::AudioDetails, QString> itAudio(streamDetails->audioDetails().at(i));
        while (itAudio.hasNext()) {
            itAudio.next();
            if (itAudio.value().isEmpty()) {
                continue;
            }

            QDomElement elem = doc.createElement(StreamDetails::detailToString(itAudio.key()));
            elem.appendChild(doc.createTextNode(itAudio.value()));
            elemAudio.appendChild(elem);
        }
        elemSd.appendChild(elemAudio);
    }

    for (int i = 0, n = streamDetails->subtitleDetails().count(); i < n; ++i) {
        QDomElement elemSubtitle = doc.createElement("subtitle");
        QMapIterator<StreamDetails::SubtitleDetails, QString> itSubtitle(streamDetails->subtitleDetails().at(i));
        while (itSubtitle.hasNext()) {
            itSubtitle.next();
            if (itSubtitle.value().isEmpty()) {
                continue;
            }

            QDomElement elem = doc.createElement(StreamDetails::detailToString(itSubtitle.key()));
            elem.appendChild(doc.createTextNode(itSubtitle.value()));
            elemSubtitle.appendChild(elem);
        }
        elemSd.appendChild(elemSubtitle);
    }

    for (Subtitle* subtitle : subtitles) {
        QDomElement elemSubtitle = doc.createElement("subtitle");
        QDomElement elem = doc.createElement("language");
        elem.appendChild(doc.createTextNode(subtitle->language()));
        elemSubtitle.appendChild(elem);

        QDomElement elem2 = doc.createElement("file");
        elem2.appendChild(doc.createTextNode(subtitle->files().first()));
        elemSubtitle.appendChild(elem2);

        elemSd.appendChild(elemSubtitle);
    }

    elemFi.appendChild(elemSd);
    appendXmlNode(doc, elemFi);
}

/**
 * @brief Get the path to the actor image
 * @param actor Actor
 * @return Path to actor image
 */
QString KodiXml::actorImageName(Movie* movie, Actor actor)
{
    if (movie->files().isEmpty()) {
        return QString();
    }
    QFileInfo fi(movie->files().at(0));
    QString actorName = actor.name;
    actorName = actorName.replace(" ", "_");
    QString path = fi.absolutePath() + "/" + ".actors" + "/" + actorName + ".jpg";
    fi.setFile(path);
    if (fi.isFile()) {
        return path;
    }
    return QString();
}

QByteArray KodiXml::getConcertXml(Concert* concert)
{
    using namespace mediaelch;
    // @todo(bugwelle):
    // I'm fully aware that this is bad coding style but writing this clean
    // requires so much refactoring that writing this whole feature would be easier.
    // It's on my todo list to refactor this. Maybe into a Kodi factory.
    std::unique_ptr<kodi::ConcertXmlWriter> writer;
    switch (m_version.version()) {
    case KodiVersion::v16: writer = std::make_unique<kodi::ConcertXmlWriterV16>(*concert); break;
    case KodiVersion::v17: writer = std::make_unique<kodi::ConcertXmlWriterV17>(*concert); break;
    case KodiVersion::v18: writer = std::make_unique<kodi::ConcertXmlWriterV18>(*concert); break;
    }
    return writer->getConcertXml();
}

/**
 * @brief Saves a concert (including images)
 * @param concert Concert to save
 * @return Saving success
 * @see KodiXml::writeConcertXml
 */
bool KodiXml::saveConcert(Concert* concert)
{
    QByteArray xmlContent = getConcertXml(concert);

    if (concert->files().empty()) {
        qWarning() << "[KodiXml] Concert has no files";
        return false;
    }

    concert->setNfoContent(xmlContent);
    Manager::instance()->database()->update(concert);

    bool saved = false;
    QFileInfo fi(concert->files().at(0));
    for (DataFile dataFile : Settings::instance()->dataFiles(DataFileType::ConcertNfo)) {
        QString saveFileName =
            dataFile.saveFileName(fi.fileName(), SeasonNumber::NoSeason, concert->files().count() > 1);
        QString saveFilePath = fi.absolutePath() + "/" + saveFileName;
        QDir saveFileDir = QFileInfo(saveFilePath).dir();
        if (!saveFileDir.exists()) {
            saveFileDir.mkpath(".");
        }
        QFile file(saveFilePath);
        qDebug() << "[KodiXml] Saving to" << file.fileName();
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "[KodiXml] File could not be openend";
        } else {
            file.write(xmlContent);
            file.close();
            saved = true;
        }
    }
    if (!saved) {
        return false;
    }

    for (const auto imageType : Concert::imageTypes()) {
        DataFileType dataFileType = DataFile::dataFileTypeForImageType(imageType);
        if (concert->imageHasChanged(imageType) && !concert->image(imageType).isNull()) {
            for (DataFile dataFile : Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName =
                    dataFile.saveFileName(fi.fileName(), SeasonNumber::NoSeason, concert->files().count() > 1);
                if (imageType == ImageType::ConcertPoster
                    && (concert->discType() == DiscType::BluRay || concert->discType() == DiscType::Dvd)) {
                    saveFileName = "poster.jpg";
                }
                if (imageType == ImageType::ConcertBackdrop
                    && (concert->discType() == DiscType::BluRay || concert->discType() == DiscType::Dvd)) {
                    saveFileName = "fanart.jpg";
                }
                QString path = getPath(concert);
                saveFile(path + "/" + saveFileName, concert->image(imageType));
            }
        }
        if (concert->imagesToRemove().contains(imageType)) {
            for (DataFile dataFile : Settings::instance()->dataFiles(imageType)) {
                QString saveFileName =
                    dataFile.saveFileName(fi.fileName(), SeasonNumber::NoSeason, concert->files().count() > 1);
                if (imageType == ImageType::ConcertPoster
                    && (concert->discType() == DiscType::BluRay || concert->discType() == DiscType::Dvd)) {
                    saveFileName = "poster.jpg";
                }
                if (imageType == ImageType::ConcertBackdrop
                    && (concert->discType() == DiscType::BluRay || concert->discType() == DiscType::Dvd)) {
                    saveFileName = "fanart.jpg";
                }
                QString path = getPath(concert);
                QFile(path + "/" + saveFileName).remove();
            }
        }
    }

    if (concert->inSeparateFolder() && !concert->files().isEmpty()) {
        for (const QString& file : concert->extraFanartsToRemove()) {
            QFile::remove(file);
        }
        QDir dir(QFileInfo(concert->files().first()).absolutePath() + "/extrafanart");
        if (!dir.exists() && !concert->extraFanartImagesToAdd().isEmpty()) {
            QDir(QFileInfo(concert->files().first()).absolutePath()).mkdir("extrafanart");
        }
        for (const QByteArray& img : concert->extraFanartImagesToAdd()) {
            int num = 1;
            while (QFileInfo(dir.absolutePath() + "/" + QString("fanart%1.jpg").arg(num)).exists()) {
                ++num;
            }
            saveFile(dir.absolutePath() + "/" + QString("fanart%1.jpg").arg(num), img);
        }
    }

    return true;
}

/**
 * @brief Loads concert infos (except images)
 * @param concert Concert to load
 * @return Loading success
 */
bool KodiXml::loadConcert(Concert* concert, QString initialNfoContent)
{
    concert->clear();
    concert->setChanged(false);

    QString nfoContent;
    if (initialNfoContent.isEmpty()) {
        QString nfoFile = nfoFilePath(concert);
        if (nfoFile.isEmpty()) {
            return false;
        }

        QFile file(nfoFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "[KodiXml] File" << nfoFile << "could not be opened for reading";
            return false;
        }
        nfoContent = QString::fromUtf8(file.readAll());
        concert->setNfoContent(nfoContent);
        file.close();
    } else {
        nfoContent = initialNfoContent;
    }

    QDomDocument domDoc;
    domDoc.setContent(nfoContent);

    mediaelch::kodi::ConcertXmlReader reader(*concert);
    reader.parseNfoDom(domDoc);

    concert->setStreamDetailsLoaded(loadStreamDetails(concert->streamDetails(), domDoc));

    // Existence of images
    if (initialNfoContent.isEmpty()) {
        for (const ImageType imageType : Concert::imageTypes()) {
            concert->setHasImage(imageType, !imageFileName(concert, imageType).isEmpty());
        }
        concert->setHasExtraFanarts(!extraFanartNames(concert).isEmpty());
    }

    return true;
}

/**
 * @brief Get path to actor image
 * @return Path to actor image
 */
QString KodiXml::actorImageName(TvShow* show, Actor actor)
{
    if (show->dir().isEmpty()) {
        return QString();
    }
    QString actorName = actor.name;
    actorName = actorName.replace(" ", "_");
    QString fileName = show->dir() + "/" + ".actors" + "/" + actorName + ".jpg";
    QFileInfo fi(fileName);
    if (fi.isFile()) {
        return fileName;
    }
    return QString();
}

QString KodiXml::actorImageName(TvShowEpisode* episode, Actor actor)
{
    if (episode->files().isEmpty()) {
        return QString();
    }
    QFileInfo fi(episode->files().at(0));
    QString actorName = actor.name;
    actorName = actorName.replace(" ", "_");
    QString path = fi.absolutePath() + "/" + ".actors" + "/" + actorName + ".jpg";
    fi.setFile(path);
    if (fi.isFile()) {
        return path;
    }
    return QString();
}

/**
 * @brief Loads TV show information
 * @param show Show to load
 * @return Loading success
 */
bool KodiXml::loadTvShow(TvShow* show, QString initialNfoContent)
{
    show->clear();
    show->setChanged(false);

    QString nfoContent;
    if (initialNfoContent.isEmpty()) {
        if (show->dir().isEmpty()) {
            return false;
        }

        QString nfoFile;
        for (DataFile dataFile : Settings::instance()->dataFiles(DataFileType::TvShowNfo)) {
            QString file = dataFile.saveFileName("");
            QFileInfo nfoFi(show->dir() + "/" + file);
            if (nfoFi.exists()) {
                nfoFile = show->dir() + "/" + file;
                break;
            }
        }
        QFile file(nfoFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "[KodiXml] Nfo file could not be opened for reading" << nfoFile;
            return false;
        }
        nfoContent = QString::fromUtf8(file.readAll());
        show->setNfoContent(nfoContent);
        file.close();
    } else {
        nfoContent = initialNfoContent;
    }

    QDomDocument domDoc;
    domDoc.setContent(nfoContent);

    mediaelch::kodi::TvShowXmlReader reader(*show);
    reader.parseNfoDom(domDoc);

    return true;
}

/**
 * @brief Loads TV show episode information
 * @param episode Episode to load infos for
 * @return Loading success
 */
bool KodiXml::loadTvShowEpisode(TvShowEpisode* episode, QString initialNfoContent)
{
    if (episode == nullptr) {
        qWarning() << "[KodiXml] Passed an empty (null) episode to loadTvShowEpisode";
        return false;
    }
    episode->clear();
    episode->setChanged(false);

    QString nfoContent;
    if (initialNfoContent.isEmpty()) {
        QString nfoFile = nfoFilePath(episode);
        if (nfoFile.isEmpty()) {
            return false;
        }

        QFile file(nfoFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "[KodiXml] File" << nfoFile << "could not be opened for reading";
            return false;
        }
        nfoContent = QString::fromUtf8(file.readAll());
        episode->setNfoContent(nfoContent);
        file.close();
    } else {
        nfoContent = initialNfoContent;
    }

    QDomDocument domDoc;
    domDoc.setContent(mediaelch::kodi::EpisodeXmlReader::makeValidEpisodeXml(nfoContent));

    QDomNodeList episodeDetailsList = domDoc.elementsByTagName("episodedetails");
    if (episodeDetailsList.isEmpty()) {
        return false;
    }

    QDomElement episodeDetails;
    if (episodeDetailsList.count() > 1) {
        bool found = false;
        for (int i = 0, n = episodeDetailsList.count(); i < n; ++i) {
            episodeDetails = episodeDetailsList.at(i).toElement();
            if (!episodeDetails.elementsByTagName("season").isEmpty()
                && episodeDetails.elementsByTagName("season").at(0).toElement().text().toInt()
                       == episode->season().toInt()
                && !episodeDetails.elementsByTagName("episode").isEmpty()
                && episodeDetails.elementsByTagName("episode").at(0).toElement().text().toInt()
                       == episode->episode().toInt()) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }

    } else {
        episodeDetails = episodeDetailsList.at(0).toElement();
    }

    mediaelch::kodi::EpisodeXmlReader reader(*episode);
    reader.parseNfoDom(episodeDetails);

    if (episodeDetails.elementsByTagName("streamdetails").count() > 0) {
        loadStreamDetails(
            episode->streamDetails(), episodeDetails.elementsByTagName("streamdetails").at(0).toElement());
        episode->setStreamDetailsLoaded(true);
    } else {
        episode->setStreamDetailsLoaded(false);
    }

    return true;
}

/**
 * @brief Saves a TV show
 * @param show Show to save
 * @return Saving success
 * @see KodiXml::writeTvShowXml
 */
bool KodiXml::saveTvShow(TvShow* show)
{
    QByteArray xmlContent = getTvShowXml(show);

    if (show->dir().isEmpty()) {
        return false;
    }

    show->setNfoContent(xmlContent);
    Manager::instance()->database()->update(show);

    for (DataFile dataFile : Settings::instance()->dataFiles(DataFileType::TvShowNfo)) {
        QString saveFilePath = show->dir() + "/" + dataFile.saveFileName("");
        QDir saveFileDir = QFileInfo(saveFilePath).dir();
        if (!saveFileDir.exists()) {
            saveFileDir.mkpath(".");
        }
        QFile file(saveFilePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "[KodiXml] Nfo file could not be openend for writing" << file.fileName();
            return false;
        }
        file.write(xmlContent);
        file.close();
    }

    for (const auto imageType : TvShow::imageTypes()) {
        DataFileType dataFileType = DataFile::dataFileTypeForImageType(imageType);
        if (show->imageHasChanged(imageType) && !show->image(imageType).isNull()) {
            for (auto dataFile : Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName = dataFile.saveFileName("");
                saveFile(show->dir() + "/" + saveFileName, show->image(imageType));
            }
        }
        if (show->imagesToRemove().contains(imageType)) {
            for (auto dataFile : Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName = dataFile.saveFileName("");
                QFile(show->dir() + "/" + saveFileName).remove();
            }
        }
    }

    for (const auto imageType : TvShow::seasonImageTypes()) {
        DataFileType dataFileType = DataFile::dataFileTypeForImageType(imageType);
        for (const SeasonNumber& season : show->seasons()) {
            if (show->seasonImageHasChanged(season, imageType) && !show->seasonImage(season, imageType).isNull()) {
                for (DataFile dataFile : Settings::instance()->dataFiles(dataFileType)) {
                    QString saveFileName = dataFile.saveFileName("", season);
                    saveFile(show->dir() + "/" + saveFileName, show->seasonImage(season, imageType));
                }
            }
            if (show->imagesToRemove().contains(imageType)
                && show->imagesToRemove().value(imageType).contains(season)) {
                for (DataFile dataFile : Settings::instance()->dataFiles(dataFileType)) {
                    QString saveFileName = dataFile.saveFileName("", season);
                    QFile(show->dir() + "/" + saveFileName).remove();
                }
            }
        }
    }

    if (!show->dir().isEmpty()) {
        for (const QString& file : show->extraFanartsToRemove()) {
            QFile::remove(file);
        }
        QDir dir(show->dir() + "/extrafanart");
        if (!dir.exists() && !show->extraFanartImagesToAdd().isEmpty()) {
            QDir(show->dir()).mkdir("extrafanart");
        }
        for (const QByteArray& img : show->extraFanartImagesToAdd()) {
            int num = 1;
            while (QFileInfo(dir.absolutePath() + "/" + QString("fanart%1.jpg").arg(num)).exists()) {
                ++num;
            }
            saveFile(dir.absolutePath() + "/" + QString("fanart%1.jpg").arg(num), img);
        }
    }

    for (const Actor* actor : show->actors()) {
        if (!actor->image.isNull()) {
            QDir dir;
            dir.mkdir(show->dir() + "/" + ".actors");
            QString actorName = actor->name;
            actorName = actorName.replace(" ", "_");
            saveFile(show->dir() + "/" + ".actors" + "/" + actorName + ".jpg", actor->image);
        }
    }

    return true;
}

/**
 * @brief Saves a TV show episode
 * @param episode Episode to save
 * @return Saving success
 */
bool KodiXml::saveTvShowEpisode(TvShowEpisode* episode)
{
    // Multi-Episode handling
    QVector<TvShowEpisode*> episodes;
    for (TvShowEpisode* subEpisode : episode->tvShow()->episodes()) {
        if (subEpisode->isDummy()) {
            continue;
        }
        if (episode->files() == subEpisode->files()) {
            episodes.append(subEpisode);
        }
    }

    if (episode->files().isEmpty()) {
        qWarning() << "[KodiXml] Episode has no files";
        return false;
    }

    const QByteArray xmlContent = getEpisodeXml(episodes);
    for (TvShowEpisode* subEpisode : episodes) {
        subEpisode->setNfoContent(xmlContent);
        subEpisode->setSyncNeeded(true);
        subEpisode->setChanged(false);
        Manager::instance()->database()->update(subEpisode);
    }

    QFileInfo fi(episode->files().at(0));
    for (DataFile dataFile : Settings::instance()->dataFiles(DataFileType::TvShowEpisodeNfo)) {
        QString saveFileName =
            dataFile.saveFileName(fi.fileName(), SeasonNumber::NoSeason, episode->files().count() > 1);
        QString saveFilePath = fi.absolutePath() + "/" + saveFileName;
        QDir saveFileDir = QFileInfo(saveFilePath).dir();
        if (!saveFileDir.exists()) {
            saveFileDir.mkpath(".");
        }
        QFile file(saveFilePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "[KodiXml] Nfo file could not be opened for writing" << saveFileName;
            return false;
        }
        file.write(xmlContent);
        file.close();
    }

    fi.setFile(episode->files().at(0));
    if (episode->thumbnailImageChanged() && !episode->thumbnailImage().isNull()) {
        if (helper::isBluRay(episode->files().at(0)) || helper::isDvd(episode->files().at(0))) {
            QDir dir = fi.dir();
            dir.cdUp();
            saveFile(dir.absolutePath() + "/thumb.jpg", episode->thumbnailImage());
        } else if (helper::isDvd(episode->files().at(0), true)) {
            saveFile(fi.dir().absolutePath() + "/thumb.jpg", episode->thumbnailImage());
        } else {
            for (DataFile dataFile : Settings::instance()->dataFiles(DataFileType::TvShowEpisodeThumb)) {
                QString saveFileName =
                    dataFile.saveFileName(fi.fileName(), SeasonNumber::NoSeason, episode->files().count() > 1);
                saveFile(fi.absolutePath() + "/" + saveFileName, episode->thumbnailImage());
            }
        }
    }

    fi.setFile(episode->files().at(0));
    if (episode->imagesToRemove().contains(ImageType::TvShowEpisodeThumb)) {
        if (helper::isBluRay(episode->files().at(0)) || helper::isDvd(episode->files().at(0))) {
            QDir dir = fi.dir();
            dir.cdUp();
            QFile(dir.absolutePath() + "/thumb.jpg").remove();
        } else if (helper::isDvd(episode->files().at(0), true)) {
            QFile(fi.dir().absolutePath() + "/thumb.jpg").remove();
        } else {
            for (DataFile dataFile : Settings::instance()->dataFiles(DataFileType::TvShowEpisodeThumb)) {
                QString saveFileName =
                    dataFile.saveFileName(fi.fileName(), SeasonNumber::NoSeason, episode->files().count() > 1);
                QFile(fi.absolutePath() + "/" + saveFileName).remove();
            }
        }
    }

    fi.setFile(episode->files().at(0));
    for (const Actor* actor : episode->actors()) {
        if (!actor->image.isNull()) {
            QDir dir;
            dir.mkdir(fi.absolutePath() + "/" + ".actors");
            QString actorName = actor->name;
            actorName = actorName.replace(" ", "_");
            saveFile(fi.absolutePath() + "/" + ".actors" + "/" + actorName + ".jpg", actor->image);
        }
    }

    return true;
}

QByteArray KodiXml::getTvShowXml(TvShow* show)
{
    using namespace mediaelch;
    std::unique_ptr<kodi::TvShowXmlWriter> writer;
    switch (m_version.version()) {
    case KodiVersion::v16: writer = std::make_unique<kodi::TvShowXmlWriterV16>(*show); break;
    case KodiVersion::v17: writer = std::make_unique<kodi::TvShowXmlWriterV17>(*show); break;
    case KodiVersion::v18: writer = std::make_unique<kodi::TvShowXmlWriterV18>(*show); break;
    }
    return writer->getTvShowXml();
}

/// @brief Get an NFO document for the given episode(s). If episodes.length() > 1,
///        then we do multi-episode handling which means we write multiple <episodedetails>
///        to the same document to merge information.
QByteArray KodiXml::getEpisodeXml(const QVector<TvShowEpisode*>& episodes)
{
    using namespace mediaelch;
    std::unique_ptr<kodi::EpisodeXmlWriter> writer;
    switch (m_version.version()) {
    case KodiVersion::v16: writer = std::make_unique<kodi::EpisodeXmlWriterV16>(episodes); break;
    case KodiVersion::v17: writer = std::make_unique<kodi::EpisodeXmlWriterV17>(episodes); break;
    case KodiVersion::v18: writer = std::make_unique<kodi::EpisodeXmlWriterV18>(episodes); break;
    }
    return writer->getEpisodeXml();
}

QStringList KodiXml::extraFanartNames(Movie* movie)
{
    if (movie->files().isEmpty() || !movie->inSeparateFolder()) {
        return QStringList();
    }
    QFileInfo fi(movie->files().first());
    QDir dir(fi.absolutePath() + "/extrafanart");
    QStringList filters = {"*.jpg", "*.jpeg", "*.JPEG", "*.Jpeg", "*.JPeg"};
    QStringList files;
    for (const QString& file : dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name)) {
        files << QDir::toNativeSeparators(dir.path() + "/" + file);
    }
    return files;
}

QStringList KodiXml::extraFanartNames(Concert* concert)
{
    if (concert->files().isEmpty() || !concert->inSeparateFolder()) {
        return QStringList();
    }
    QFileInfo fi(concert->files().first());
    QDir dir(fi.absolutePath() + "/extrafanart");
    QStringList filters = {"*.jpg", "*.jpeg", "*.JPEG", "*.Jpeg", "*.JPeg"};
    QStringList files;
    for (const QString& file : dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name)) {
        files << QDir::toNativeSeparators(dir.path() + "/" + file);
    }
    return files;
}

QStringList KodiXml::extraFanartNames(TvShow* show)
{
    if (show->dir().isEmpty()) {
        return QStringList();
    }
    QDir dir(show->dir() + "/extrafanart");
    QStringList filters = {"*.jpg", "*.jpeg", "*.JPEG", "*.Jpeg", "*.JPeg"};
    QStringList files;
    for (const QString& file : dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name)) {
        files << QDir::toNativeSeparators(dir.path() + "/" + file);
    }
    return files;
}

QStringList KodiXml::extraFanartNames(Artist* artist)
{
    QDir dir(artist->path() + "/extrafanart");
    QStringList filters = {"*.jpg", "*.jpeg", "*.JPEG", "*.Jpeg", "*.JPeg"};
    QStringList files;
    for (const QString& file : dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name)) {
        files << QDir::toNativeSeparators(dir.path() + "/" + file);
    }
    return files;
}

QImage KodiXml::movieSetPoster(QString setName)
{
    for (DataFile dataFile : Settings::instance()->dataFiles(DataFileType::MovieSetPoster)) {
        QString fileName = movieSetFileName(setName, &dataFile);
        QFileInfo fi(fileName);
        if (fi.exists()) {
            return QImage(fi.absoluteFilePath());
        }
    }
    return QImage();
}

QImage KodiXml::movieSetBackdrop(QString setName)
{
    for (DataFile dataFile : Settings::instance()->dataFiles(DataFileType::MovieSetBackdrop)) {
        QString fileName = movieSetFileName(setName, &dataFile);
        QFileInfo fi(fileName);
        if (fi.exists()) {
            return QImage(fi.absoluteFilePath());
        }
    }
    return QImage();
}

/**
 * @brief Save movie set poster
 */
void KodiXml::saveMovieSetPoster(QString setName, QImage poster)
{
    for (DataFile dataFile : Settings::instance()->dataFiles(DataFileType::MovieSetPoster)) {
        QString fileName = movieSetFileName(setName, &dataFile);
        if (!fileName.isEmpty()) {
            poster.save(fileName, "jpg", 100);
        }
    }
}

/**
 * @brief Save movie set backdrop
 */
void KodiXml::saveMovieSetBackdrop(QString setName, QImage backdrop)
{
    for (DataFile dataFile : Settings::instance()->dataFiles(DataFileType::MovieSetBackdrop)) {
        QString fileName = movieSetFileName(setName, &dataFile);
        if (!fileName.isEmpty()) {
            backdrop.save(fileName, "jpg", 100);
        }
    }
}

bool KodiXml::saveFile(QString filename, QByteArray data)
{
    QDir saveFileDir = QFileInfo(filename).dir();
    if (!saveFileDir.exists()) {
        saveFileDir.mkpath(".");
    }
    QFile file(filename);

    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
        return true;
    }
    return false;
}

QString KodiXml::getPath(const Movie* movie)
{
    if (movie->files().isEmpty()) {
        return QString();
    }
    QFileInfo fi(movie->files().first());
    if (movie->discType() == DiscType::BluRay) {
        QDir dir = fi.dir();
        if (QString::compare(dir.dirName(), "BDMV", Qt::CaseInsensitive) == 0) {
            dir.cdUp();
        }
        return dir.absolutePath();
    }
    if (movie->discType() == DiscType::Dvd) {
        QDir dir = fi.dir();
        if (QString::compare(dir.dirName(), "VIDEO_TS", Qt::CaseInsensitive) == 0) {
            dir.cdUp();
        }
        return dir.absolutePath();
    }
    return fi.absolutePath();
}

QString KodiXml::getPath(const Concert* concert)
{
    if (concert->files().isEmpty()) {
        return QString();
    }
    QFileInfo fi(concert->files().first());
    if (concert->discType() == DiscType::BluRay) {
        QDir dir = fi.dir();
        if (QString::compare(dir.dirName(), "BDMV", Qt::CaseInsensitive) == 0) {
            dir.cdUp();
        }
        return dir.absolutePath();
    }
    if (concert->discType() == DiscType::Dvd) {
        QDir dir = fi.dir();
        if (QString::compare(dir.dirName(), "VIDEO_TS", Qt::CaseInsensitive) == 0) {
            dir.cdUp();
        }
        return dir.absolutePath();
    }
    return fi.absolutePath();
}

QString KodiXml::movieSetFileName(QString setName, DataFile* dataFile)
{
    if (Settings::instance()->movieSetArtworkType() == MovieSetArtworkType::SingleArtworkFolder) {
        QDir dir(Settings::instance()->movieSetArtworkDirectory());
        QString fileName = dataFile->saveFileName(setName);
        return dir.absolutePath() + "/" + fileName;
    }
    if (Settings::instance()->movieSetArtworkType() == MovieSetArtworkType::SingleSetFolder) {
        for (Movie* movie : Manager::instance()->movieModel()->movies()) {
            if (movie->set().name == setName && !movie->files().isEmpty()) {
                QFileInfo fi(movie->files().first());
                QDir dir = fi.dir();
                if (movie->inSeparateFolder()) {
                    dir.cdUp();
                }
                if (movie->discType() == DiscType::Dvd || movie->discType() == DiscType::BluRay) {
                    dir.cdUp();
                }
                return dir.absolutePath() + "/" + dataFile->saveFileName(setName);
            }
        }
    }

    return QString();
}

QString KodiXml::imageFileName(const Movie* movie, ImageType type, QVector<DataFile> dataFiles, bool constructName)
{
    DataFileType fileType = [type]() {
        switch (type) {
        case ImageType::MoviePoster: return DataFileType::MoviePoster;
        case ImageType::MovieBackdrop: return DataFileType::MovieBackdrop;
        case ImageType::MovieLogo: return DataFileType::MovieLogo;
        case ImageType::MovieBanner: return DataFileType::MovieBanner;
        case ImageType::MovieThumb: return DataFileType::MovieThumb;
        case ImageType::MovieClearArt: return DataFileType::MovieClearArt;
        case ImageType::MovieCdArt: return DataFileType::MovieCdArt;
        default: return DataFileType::NoType;
        }
    }();

    if (fileType == DataFileType::NoType) {
        return "";
    }

    if (movie->files().empty()) {
        qWarning() << "Movie has no files";
        return "";
    }

    if (!constructName) {
        dataFiles = Settings::instance()->dataFiles(fileType);
    }

    QString fileName;
    QFileInfo fi(movie->files().at(0));
    for (DataFile dataFile : dataFiles) {
        QString file = dataFile.saveFileName(fi.fileName(), SeasonNumber::NoSeason, movie->files().count() > 1);
        if (movie->discType() == DiscType::BluRay || movie->discType() == DiscType::Dvd) {
            if (type == ImageType::MoviePoster) {
                file = "poster.jpg";
            } else if (type == ImageType::MovieBackdrop) {
                file = "fanart.jpg";
            }
        }
        QString path = getPath(movie);
        QFileInfo pFi(path + "/" + file);
        if (pFi.isFile() || constructName) {
            fileName = path + "/" + file;
            break;
        }
    }

    return fileName;
}

QString KodiXml::imageFileName(const Concert* concert, ImageType type, QVector<DataFile> dataFiles, bool constructName)
{
    DataFileType fileType;
    switch (type) {
    case ImageType::ConcertPoster: fileType = DataFileType::ConcertPoster; break;
    case ImageType::ConcertBackdrop: fileType = DataFileType::ConcertBackdrop; break;
    case ImageType::ConcertLogo: fileType = DataFileType::ConcertLogo; break;
    case ImageType::ConcertClearArt: fileType = DataFileType::ConcertClearArt; break;
    case ImageType::ConcertCdArt: fileType = DataFileType::ConcertCdArt; break;
    default: return "";
    }

    if (concert->files().empty()) {
        qWarning() << "[KodiXml] Concert has no files";
        return "";
    }

    if (!constructName) {
        dataFiles = Settings::instance()->dataFiles(fileType);
    }

    QString fileName;
    QFileInfo fi(concert->files().at(0));
    for (DataFile dataFile : dataFiles) {
        QString file = dataFile.saveFileName(fi.fileName(), SeasonNumber::NoSeason, concert->files().count() > 1);
        if (concert->discType() == DiscType::BluRay || concert->discType() == DiscType::Dvd) {
            if (type == ImageType::ConcertPoster) {
                file = "poster.jpg";
            }
            if (type == ImageType::ConcertBackdrop) {
                file = "fanart.jpg";
            }
        }
        QString path = getPath(concert);
        QFileInfo pFi(path + "/" + file);
        if (pFi.isFile() || constructName) {
            fileName = path + "/" + file;
            break;
        }
    }

    return fileName;
}

QString KodiXml::imageFileName(const TvShow* show,
    ImageType type,
    SeasonNumber season,
    QVector<DataFile> dataFiles,
    bool constructName)
{
    DataFileType fileType;
    switch (type) {
    case ImageType::TvShowPoster: fileType = DataFileType::TvShowPoster; break;
    case ImageType::TvShowBackdrop: fileType = DataFileType::TvShowBackdrop; break;
    case ImageType::TvShowLogos: fileType = DataFileType::TvShowLogo; break;
    case ImageType::TvShowBanner: fileType = DataFileType::TvShowBanner; break;
    case ImageType::TvShowThumb: fileType = DataFileType::TvShowThumb; break;
    case ImageType::TvShowClearArt: fileType = DataFileType::TvShowClearArt; break;
    case ImageType::TvShowCharacterArt: fileType = DataFileType::TvShowCharacterArt; break;
    case ImageType::TvShowSeasonPoster: fileType = DataFileType::TvShowSeasonPoster; break;
    case ImageType::TvShowSeasonBackdrop: fileType = DataFileType::TvShowSeasonBackdrop; break;
    case ImageType::TvShowSeasonBanner: fileType = DataFileType::TvShowSeasonBanner; break;
    case ImageType::TvShowSeasonThumb: fileType = DataFileType::TvShowSeasonThumb; break;
    default: return "";
    }

    if (show->dir().isEmpty()) {
        return QString();
    }

    if (!constructName) {
        dataFiles = Settings::instance()->dataFiles(fileType);
    }

    QString fileName;
    for (DataFile dataFile : dataFiles) {
        QString loadFileName = dataFile.saveFileName("", season);
        QFileInfo fi(show->dir() + "/" + loadFileName);
        if (fi.isFile() || constructName) {
            fileName = show->dir() + "/" + loadFileName;
            break;
        }
    }
    return fileName;
}

QString saveDataFiles(QString basePath, QString fileName, const QVector<DataFile>& dataFiles, bool constructName)
{
    for (DataFile dataFile : dataFiles) {
        QString file = dataFile.saveFileName(fileName);
        QFileInfo pFi(basePath + "/" + file);
        if (pFi.isFile() || constructName) {
            return basePath + "/" + file;
        }
    }
    return QStringLiteral();
}

QString
KodiXml::imageFileName(const TvShowEpisode* episode, ImageType type, QVector<DataFile> dataFiles, bool constructName)
{
    DataFileType fileType;
    switch (type) {
    case ImageType::TvShowEpisodeThumb: fileType = DataFileType::TvShowEpisodeThumb; break;
    default: return "";
    }

    if (episode->files().isEmpty()) {
        return "";
    }
    QFileInfo fi(episode->files().at(0));

    if (helper::isBluRay(episode->files().at(0)) || helper::isDvd(episode->files().at(0))) {
        QDir dir = fi.dir();
        dir.cdUp();
        fi.setFile(dir.absolutePath() + "/thumb.jpg");
        return fi.exists() ? fi.absoluteFilePath() : "";
    }

    if (helper::isDvd(episode->files().at(0), true)) {
        fi.setFile(fi.dir().absolutePath() + "/thumb.jpg");
        return fi.exists() ? fi.absoluteFilePath() : "";
    }

    if (!constructName) {
        dataFiles = Settings::instance()->dataFiles(fileType);
    }

    return saveDataFiles(fi.absolutePath(), fi.fileName(), dataFiles, constructName);
}

bool KodiXml::loadArtist(Artist* artist, QString initialNfoContent)
{
    artist->clear();
    artist->setHasChanged(false);

    QString nfoContent;
    if (initialNfoContent.isEmpty()) {
        QString nfoFile = nfoFilePath(artist);
        if (nfoFile.isEmpty()) {
            return false;
        }

        QFile file(nfoFile);
        if (!file.exists()) {
            return false;
        }
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "[KodiXml] File" << nfoFile << "could not be opened for reading";
            return false;
        }
        nfoContent = QString::fromUtf8(file.readAll());
        artist->setNfoContent(nfoContent);
        file.close();
    } else {
        nfoContent = initialNfoContent;
    }

    QDomDocument domDoc;
    domDoc.setContent(nfoContent);

    mediaelch::kodi::ArtistXmlReader reader(*artist);
    reader.parseNfoDom(domDoc);

    return true;
}

bool KodiXml::loadAlbum(Album* album, QString initialNfoContent)
{
    if (album == nullptr) {
        return false;
    }
    album->clear();
    album->setHasChanged(false);

    QString nfoContent;
    if (initialNfoContent.isEmpty()) {
        QString nfoFile = nfoFilePath(album);
        if (nfoFile.isEmpty()) {
            return false;
        }

        QFile file(nfoFile);
        if (!file.exists()) {
            return false;
        }
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "[KodiXml] File" << nfoFile << "could not be opened for reading";
            return false;
        }
        nfoContent = QString::fromUtf8(file.readAll());
        album->setNfoContent(nfoContent);
        file.close();
    } else {
        nfoContent = initialNfoContent;
    }

    QDomDocument domDoc;
    domDoc.setContent(nfoContent);

    mediaelch::kodi::AlbumXmlReader reader(*album);
    reader.parseNfoDom(domDoc);

    return true;
}

QString KodiXml::imageFileName(const Artist* artist, ImageType type, QVector<DataFile> dataFiles, bool constructName)
{
    DataFileType fileType;
    switch (type) {
    case ImageType::ArtistThumb: fileType = DataFileType::ArtistThumb; break;
    case ImageType::ArtistFanart: fileType = DataFileType::ArtistFanart; break;
    case ImageType::ArtistLogo: fileType = DataFileType::ArtistLogo; break;
    default: return "";
    }

    if (artist->path().isEmpty()) {
        return QString();
    }

    if (!constructName) {
        dataFiles = Settings::instance()->dataFiles(fileType);
    }

    return saveDataFiles(artist->path(), "", dataFiles, constructName);
}

QString KodiXml::imageFileName(const Album* album, ImageType type, QVector<DataFile> dataFiles, bool constructName)
{
    DataFileType fileType;
    switch (type) {
    case ImageType::AlbumThumb: fileType = DataFileType::AlbumThumb; break;
    case ImageType::AlbumCdArt: fileType = DataFileType::AlbumCdArt; break;
    default: return "";
    }

    if (album->path().isEmpty()) {
        return QString();
    }

    if (!constructName) {
        dataFiles = Settings::instance()->dataFiles(fileType);
    }

    return saveDataFiles(album->path(), "", dataFiles, constructName);
}

QString KodiXml::nfoFilePath(Artist* artist)
{
    if (artist->path().isEmpty()) {
        return QString();
    }

    return artist->path() + "/artist.nfo";
}

QString KodiXml::nfoFilePath(Album* album)
{
    if (album->path().isEmpty()) {
        return QString();
    }

    return album->path() + "/album.nfo";
}

bool KodiXml::saveArtist(Artist* artist)
{
    QByteArray xmlContent = getArtistXml(artist);

    if (artist->path().isEmpty()) {
        return false;
    }

    artist->setNfoContent(xmlContent);
    Manager::instance()->database()->update(artist);

    QString fileName = nfoFilePath(artist);
    if (fileName.isEmpty()) {
        return false;
    }

    QDir saveFileDir = QFileInfo(fileName).dir();
    if (!saveFileDir.exists()) {
        saveFileDir.mkpath(".");
    }
    {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "[KodiXml] File could not be openend";
            return false;
        }
        file.write(xmlContent);
        file.close();
    }
    for (const auto imageType : Artist::imageTypes()) {
        DataFileType dataFileType = DataFile::dataFileTypeForImageType(imageType);

        if (artist->imagesToRemove().contains(imageType)) {
            for (DataFile dataFile : Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName = dataFile.saveFileName(QString());
                if (!saveFileName.isEmpty()) {
                    QFile(artist->path() + "/" + saveFileName).remove();
                }
            }
        }

        if (!artist->rawImage(imageType).isNull()) {
            for (DataFile dataFile : Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName = dataFile.saveFileName(QString());
                saveFile(artist->path() + "/" + saveFileName, artist->rawImage(imageType));
            }
        }
    }

    for (const QString& file : artist->extraFanartsToRemove()) {
        QFile::remove(file);
    }
    QDir dir(artist->path() + "/extrafanart");
    if (!dir.exists() && !artist->extraFanartImagesToAdd().isEmpty()) {
        QDir(artist->path()).mkdir("extrafanart");
    }
    for (const QByteArray& img : artist->extraFanartImagesToAdd()) {
        int num = 1;
        while (QFileInfo(dir.absolutePath() + "/" + QString("fanart%1.jpg").arg(num)).exists()) {
            ++num;
        }
        saveFile(dir.absolutePath() + "/" + QString("fanart%1.jpg").arg(num), img);
    }

    return true;
}

bool KodiXml::saveAlbum(Album* album)
{
    QByteArray xmlContent = getAlbumXml(album);

    if (album->path().isEmpty()) {
        return false;
    }

    album->setNfoContent(xmlContent);
    Manager::instance()->database()->update(album);

    QString nfoFileName = nfoFilePath(album);
    if (nfoFileName.isEmpty()) {
        return false;
    }

    QDir saveFileDir = QFileInfo(nfoFileName).dir();
    if (!saveFileDir.exists()) {
        saveFileDir.mkpath(".");
    }
    QFile nfo(nfoFileName);
    if (!nfo.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "[KodiXml] File could not be openend";
        return false;
    }
    nfo.write(xmlContent);
    nfo.close();

    for (const auto imageType : Album::imageTypes()) {
        DataFileType dataFileType = DataFile::dataFileTypeForImageType(imageType);

        if (album->imagesToRemove().contains(imageType)) {
            for (DataFile dataFile : Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName = dataFile.saveFileName(QString());
                if (!saveFileName.isEmpty()) {
                    QFile(album->path() + "/" + saveFileName).remove();
                }
            }
        }

        if (!album->rawImage(imageType).isNull()) {
            for (DataFile dataFile : Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName = dataFile.saveFileName(QString());
                saveFile(album->path() + "/" + saveFileName, album->rawImage(imageType));
            }
        }
    }

    if (album->bookletModel()->hasChanged()) {
        QDir dir(album->path() + "/booklet");
        if (!dir.exists()) {
            QDir(album->path()).mkdir("booklet");
        }

        // @todo: get filename from settings
        for (Image* image : album->bookletModel()->images()) {
            if (image->deletion() && !image->fileName().isEmpty()) {
                QFile::remove(image->fileName());
            } else if (!image->deletion()) {
                image->load();
            }
        }
        int bookletNum = 1;
        for (Image* image : album->bookletModel()->images()) {
            if (!image->deletion()) {
                QString imageFileName =
                    album->path() + "/booklet/booklet" + QString("%1").arg(bookletNum, 2, 10, QChar('0')) + ".jpg";
                QFile file(imageFileName);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(image->rawData());
                    file.close();
                }
                bookletNum++;
            }
        }
    }

    return true;
}

QByteArray KodiXml::getArtistXml(Artist* artist)
{
    using namespace mediaelch;
    // @todo(bugwelle):
    // I'm fully aware that this is bad coding style but writing this clean
    // requires so much refactoring that writing this whole feature would be easier.
    // It's on my todo list to refactor this. Maybe into a Kodi factory.
    std::unique_ptr<kodi::ArtistXmlWriter> writer;
    switch (m_version.version()) {
    case KodiVersion::v16: writer = std::make_unique<kodi::ArtistXmlWriterV16>(*artist); break;
    case KodiVersion::v17: writer = std::make_unique<kodi::ArtistXmlWriterV17>(*artist); break;
    case KodiVersion::v18: writer = std::make_unique<kodi::ArtistXmlWriterV18>(*artist); break;
    }
    return writer->getArtistXml();
}

QByteArray KodiXml::getAlbumXml(Album* album)
{
    using namespace mediaelch;
    // @todo(bugwelle):
    // I'm fully aware that this is bad coding style but writing this clean
    // requires so much refactoring that writing this whole feature would be easier.
    // It's on my todo list to refactor this. Maybe into a Kodi factory.
    std::unique_ptr<kodi::AlbumXmlWriter> writer;
    switch (m_version.version()) {
    case KodiVersion::v16: writer = std::make_unique<kodi::AlbumXmlWriterV16>(*album); break;
    case KodiVersion::v17: writer = std::make_unique<kodi::AlbumXmlWriterV17>(*album); break;
    case KodiVersion::v18: writer = std::make_unique<kodi::AlbumXmlWriterV18>(*album); break;
    }
    return writer->getAlbumXml();
}

QDomElement KodiXml::setTextValue(QDomDocument& doc, const QString& name, const QString& value)
{
    if (!doc.elementsByTagName(name).isEmpty()) {
        if (!doc.elementsByTagName(name).at(0).firstChild().isText()) {
            QDomText t = doc.createTextNode(value);
            doc.elementsByTagName(name).at(0).appendChild(t);
            return doc.elementsByTagName(name).at(0).toElement();
        }
        doc.elementsByTagName(name).at(0).firstChild().setNodeValue(value);
        return doc.elementsByTagName(name).at(0).toElement();
    }
    return addTextValue(doc, name, value);
}

void KodiXml::setListValue(QDomDocument& doc, const QString& name, const QStringList& values)
{
    QDomNode rootNode = doc.firstChild();
    while ((rootNode.nodeName() == "xml" || rootNode.isComment()) && !rootNode.isNull()) {
        rootNode = rootNode.nextSibling();
    }
    QDomNodeList childNodes = rootNode.childNodes();
    QVector<QDomNode> nodesToRemove;
    for (int i = 0, n = childNodes.count(); i < n; ++i) {
        if (childNodes.at(i).nodeName() == name) {
            nodesToRemove.append(childNodes.at(i));
        }
    }
    for (QDomNode& node : nodesToRemove) {
        rootNode.removeChild(node);
    }
    for (const QString& style : values) {
        addTextValue(doc, name, style);
    }
}

QDomElement KodiXml::addTextValue(QDomDocument& doc, const QString& name, const QString& value)
{
    QDomElement elem = doc.createElement(name);
    elem.appendChild(doc.createTextNode(value));
    appendXmlNode(doc, elem);
    return elem;
}

void KodiXml::appendXmlNode(QDomDocument& doc, QDomNode& node)
{
    QDomNode rootNode = doc.firstChild();
    while ((rootNode.nodeName() == "xml" || rootNode.isComment()) && !rootNode.isNull()) {
        rootNode = rootNode.nextSibling();
    }
    rootNode.appendChild(node);
}

void KodiXml::removeChildNodes(QDomDocument& doc, const QString& name)
{
    QDomNode rootNode = doc.firstChild();
    while ((rootNode.nodeName() == "xml" || rootNode.isComment()) && !rootNode.isNull()) {
        rootNode = rootNode.nextSibling();
    }
    QDomNodeList childNodes = rootNode.childNodes();
    QVector<QDomNode> nodesToRemove;
    for (int i = 0, n = childNodes.count(); i < n; ++i) {
        if (childNodes.at(i).nodeName() == name) {
            nodesToRemove.append(childNodes.at(i));
        }
    }
    for (const QDomNode& node : nodesToRemove) {
        rootNode.removeChild(node);
    }
}

void KodiXml::loadBooklets(Album* album)
{
    // @todo: get filename from settings
    if (!album->bookletModel()->images().isEmpty()) {
        return;
    }

    QDir dir(album->path() + "/booklet");
    QStringList filters{"*.jpg", "*.jpeg", "*.JPEG", "*.Jpeg", "*.JPeg"};
    for (const QString& file : dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name)) {
        auto img = new Image;
        img->setFileName(QDir::toNativeSeparators(dir.path() + "/" + file));
        album->bookletModel()->addImage(img);
    }
    album->bookletModel()->setHasChanged(false);
}
