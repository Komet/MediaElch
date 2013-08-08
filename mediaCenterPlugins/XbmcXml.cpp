#include "XbmcXml.h"

#include <QApplication>
#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QXmlStreamWriter>

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "settings/Settings.h"

/**
 * @brief XbmcXml::XbmcXml
 * @param parent
 */
XbmcXml::XbmcXml(QObject *parent)
{
    setParent(parent);
}

/**
 * @brief XbmcXml::~XbmcXml
 */
XbmcXml::~XbmcXml()
{
}

/**
 * @brief Checks if our MediaCenterPlugin supports a feature
 * @param feature Feature to check
 * @return Feature is supported or not
 */
bool XbmcXml::hasFeature(int feature)
{
    Q_UNUSED(feature);
    return true;
}

/**
 * @brief Writes movie elements to an xml stream
 * @param xml XML stream
 * @param movie Movie to save
 */
void XbmcXml::writeMovieXml(QXmlStreamWriter &xml, Movie *movie)
{
    qDebug() << "Entered, movie=" << movie->name();
    xml.writeStartElement("movie");
    xml.writeTextElement("title", movie->name());
    xml.writeTextElement("originaltitle", movie->originalName());
    xml.writeTextElement("rating", QString("%1").arg(movie->rating()));
    xml.writeTextElement("votes", QString::number(movie->votes()));
    xml.writeTextElement("top250", QString::number(movie->top250()));
    xml.writeTextElement("year", movie->released().toString("yyyy"));
    xml.writeTextElement("plot", movie->overview());
    xml.writeTextElement("outline", movie->outline());
    xml.writeTextElement("tagline", movie->tagline());
    if (movie->runtime() > 0)
        xml.writeTextElement("runtime", QString("%1").arg(movie->runtime()));
    xml.writeTextElement("mpaa", movie->certification());
    xml.writeTextElement("credits", movie->writer());
    xml.writeTextElement("director", movie->director());
    xml.writeTextElement("playcount", QString("%1").arg(movie->playcount()));
    if (!movie->lastPlayed().isNull())
        xml.writeTextElement("lastplayed", movie->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    if (!movie->dateAdded().isNull())
        xml.writeTextElement("dateadded", movie->dateAdded().toString("yyyy-MM-dd HH:mm:ss"));
    xml.writeTextElement("id", movie->id());
    xml.writeTextElement("tmdbid", movie->tmdbId());
    xml.writeTextElement("set", movie->set());
    xml.writeTextElement("sorttitle", movie->sortTitle());
    xml.writeTextElement("trailer", Helper::formatTrailerUrl(movie->trailer().toString()));
    xml.writeTextElement("watched", (movie->watched()) ? "true" : "false");
    foreach (const QString &studio, movie->studios()) {
        xml.writeTextElement("studio", studio);
        if (Settings::instance()->advanced()->useFirstStudioOnly())
            break;
    }
    foreach (const QString &genre, movie->genres())
        xml.writeTextElement("genre", genre);
    foreach (const QString &country, movie->countries())
        xml.writeTextElement("country", country);
    foreach (const QString &tag, movie->tags())
        xml.writeTextElement("tag", tag);
    foreach (const Actor &actor, movie->actors()) {
        xml.writeStartElement("actor");
        xml.writeTextElement("name", actor.name);
        xml.writeTextElement("role", actor.role);
        if (!actor.thumb.isEmpty())
            xml.writeTextElement("thumb", actor.thumb);
        xml.writeEndElement();
    }
    foreach (const Poster &poster, movie->posters()) {
        xml.writeStartElement("thumb");
        xml.writeAttribute("preview", poster.thumbUrl.toString());
        xml.writeCharacters(poster.originalUrl.toString());
        xml.writeEndElement();
    }
    xml.writeStartElement("fanart");
    foreach (const Poster &poster, movie->backdrops()) {
        xml.writeStartElement("thumb");
        xml.writeAttribute("preview", poster.thumbUrl.toString());
        xml.writeCharacters(poster.originalUrl.toString());
        xml.writeEndElement();
    }
    xml.writeEndElement();

    writeStreamDetails(xml, movie->streamDetails());

    xml.writeEndElement();
}

/**
 * @brief Saves a movie (including images)
 * @param movie Movie to save
 * @return Saving success
 * @see XbmcXml::writeMovieXml
 */
bool XbmcXml::saveMovie(Movie *movie)
{
    qDebug() << "Entered, movie=" << movie->name();
    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);
    writeMovieXml(xml, movie);
    xml.writeEndDocument();

    if (movie->files().size() == 0) {
        qWarning() << "Movie has no files";
        return false;
    }

    movie->setNfoContent(xmlContent);
    Manager::instance()->database()->update(movie);

    bool saved = false;
    QFileInfo fi(movie->files().at(0));
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MovieNfo)) {
        QString saveFileName = dataFile.saveFileName(fi.fileName(), -1, movie->files().count() > 1);
        QFile file(fi.absolutePath() + "/" + saveFileName);
        qDebug() << "Saving to" << file.fileName();
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "File could not be openend";
        } else {
            file.write(xmlContent);
            file.close();
            saved = true;
        }
    }
    if (!saved)
        return false;

    foreach (const int &imageType, Movie::imageTypes()) {
        int dataFileType = DataFile::dataFileTypeForImageType(imageType);
        if (movie->imageHasChanged(imageType) && !movie->image(imageType).isNull()) {
            foreach (DataFile dataFile, Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName = dataFile.saveFileName(fi.fileName(), -1, movie->files().count() > 1);
                if (imageType == ImageType::MoviePoster && (movie->discType() == DiscBluRay || movie->discType() == DiscDvd))
                    saveFileName = "poster.jpg";
                if (imageType == ImageType::MovieBackdrop && (movie->discType() == DiscBluRay || movie->discType() == DiscDvd))
                    saveFileName = "fanart.jpg";
                QString path = getPath(movie);
                saveFile(path + "/" + saveFileName, movie->image(imageType));
            }
        }

        if (movie->imagesToRemove().contains(imageType)) {
            foreach (DataFile dataFile, Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName = dataFile.saveFileName(fi.fileName(), -1, movie->files().count() > 1);
                if (imageType == ImageType::MoviePoster && (movie->discType() == DiscBluRay || movie->discType() == DiscDvd))
                    saveFileName = "poster.jpg";
                if (imageType == ImageType::MovieBackdrop && (movie->discType() == DiscBluRay || movie->discType() == DiscDvd))
                    saveFileName = "fanart.jpg";
                QString path = getPath(movie);
                QFile(path + "/" + saveFileName).remove();
            }
        }
    }

    if (movie->inSeparateFolder() && !movie->files().isEmpty()) {
        foreach (const QString &file, movie->extraFanartsToRemove())
            QFile::remove(file);
        QDir dir(QFileInfo(movie->files().first()).absolutePath() + "/extrafanart");
        if (!dir.exists() && !movie->extraFanartImagesToAdd().isEmpty())
            QDir(QFileInfo(movie->files().first()).absolutePath()).mkdir("extrafanart");
        foreach (QByteArray img, movie->extraFanartImagesToAdd()) {
            int num = 1;
            while (QFileInfo(dir.absolutePath() + "/" + QString("fanart%1.jpg").arg(num)).exists())
                ++num;
            saveFile(dir.absolutePath() + "/" + QString("fanart%1.jpg").arg(num), img);
        }
    }


    foreach (const Actor &actor, movie->actors()) {
        if (!actor.image.isNull()) {
            QDir dir;
            dir.mkdir(fi.absolutePath() + "/" + ".actors");
            QString actorName = actor.name;
            actorName = actorName.replace(" ", "_");
            saveFile(fi.absolutePath() + "/" + ".actors" + "/" + actorName + ".jpg", actor.image);
        }
    }

    return true;
}

/**
 * @brief Tries to find an nfo file for the movie
 * @param movie Movie
 * @return Path to nfo file, if none found returns an empty string
 */
QString XbmcXml::nfoFilePath(Movie *movie)
{
    QString nfoFile;
    if (movie->files().size() == 0) {
        qWarning() << "Movie has no files";
        return nfoFile;
    }
    QFileInfo fi(movie->files().at(0));
    if (!fi.isFile() ) {
        qWarning() << "First file of the movie is not readable" << movie->files().at(0);
        return nfoFile;
    }

    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MovieNfo)) {
        QString file = dataFile.saveFileName(fi.fileName(), -1, movie->files().count() > 1);
        QFileInfo nfoFi(fi.absolutePath() + "/" + file);
        if (nfoFi.exists()) {
            nfoFile = fi.absolutePath() + "/" + file;
            break;
        }
    }

    return nfoFile;
}

QString XbmcXml::nfoFilePath(TvShowEpisode *episode)
{
    QString nfoFile;
    if (episode->files().size() == 0) {
        qWarning() << "Episode has no files";
        return nfoFile;
    }
    QFileInfo fi(episode->files().at(0));
    if (!fi.isFile() ) {
        qWarning() << "First file of the episode is not readable" << episode->files().at(0);
        return nfoFile;
    }

    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowEpisodeNfo)) {
        QString file = dataFile.saveFileName(fi.fileName(), -1, episode->files().count() > 1);
        QFileInfo nfoFi(fi.absolutePath() + "/" + file);
        if (nfoFi.exists()) {
            nfoFile = fi.absolutePath() + "/" + file;
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
QString XbmcXml::nfoFilePath(Concert *concert)
{
    QString nfoFile;
    if (concert->files().size() == 0) {
        qWarning() << "Concert has no files";
        return nfoFile;
    }
    QFileInfo fi(concert->files().at(0));
    if (!fi.isFile() ) {
        qWarning() << "First file of the concert is not readable" << concert->files().at(0);
        return nfoFile;
    }

    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::ConcertNfo)) {
        QString file = dataFile.saveFileName(fi.fileName(), -1, concert->files().count() > 1);
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
bool XbmcXml::loadMovie(Movie *movie, QString initialNfoContent)
{
    movie->clear();
    movie->setChanged(false);

    QString nfoContent;
    if (initialNfoContent.isEmpty()) {
        QString nfoFile = nfoFilePath(movie);
        if (nfoFile.isEmpty())
            return false;

        QFile file(nfoFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "File" << nfoFile << "could not be opened for reading";
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
    if (!domDoc.elementsByTagName("title").isEmpty() )
        movie->setName(domDoc.elementsByTagName("title").at(0).toElement().text());
    if (!domDoc.elementsByTagName("originaltitle").isEmpty())
        movie->setOriginalName(domDoc.elementsByTagName("originaltitle").at(0).toElement().text());
    if (!domDoc.elementsByTagName("rating").isEmpty())
        movie->setRating(domDoc.elementsByTagName("rating").at(0).toElement().text().toFloat());
    if (!domDoc.elementsByTagName("votes").isEmpty())
        movie->setVotes(domDoc.elementsByTagName("votes").at(0).toElement().text().replace(",", "").replace(".", "").toInt());
    if (!domDoc.elementsByTagName("top250").isEmpty())
        movie->setTop250(domDoc.elementsByTagName("top250").at(0).toElement().text().toInt());
    if (!domDoc.elementsByTagName("year").isEmpty())
        movie->setReleased(QDate::fromString(domDoc.elementsByTagName("year").at(0).toElement().text(), "yyyy"));
    if (!domDoc.elementsByTagName("plot").isEmpty())
        movie->setOverview(domDoc.elementsByTagName("plot").at(0).toElement().text());
    if (!domDoc.elementsByTagName("outline").isEmpty())
        movie->setOutline(domDoc.elementsByTagName("outline").at(0).toElement().text());
    if (!domDoc.elementsByTagName("tagline").isEmpty())
        movie->setTagline(domDoc.elementsByTagName("tagline").at(0).toElement().text());
    if (!domDoc.elementsByTagName("runtime").isEmpty())
        movie->setRuntime(domDoc.elementsByTagName("runtime").at(0).toElement().text().toInt());
    if (!domDoc.elementsByTagName("mpaa").isEmpty())
        movie->setCertification(domDoc.elementsByTagName("mpaa").at(0).toElement().text());
    if (!domDoc.elementsByTagName("credits").isEmpty())
        movie->setWriter(domDoc.elementsByTagName("credits").at(0).toElement().text());
    if (!domDoc.elementsByTagName("director").isEmpty())
        movie->setDirector(domDoc.elementsByTagName("director").at(0).toElement().text());
    if (!domDoc.elementsByTagName("playcount").isEmpty())
        movie->setPlayCount(domDoc.elementsByTagName("playcount").at(0).toElement().text().toInt());
    if (!domDoc.elementsByTagName("lastplayed").isEmpty()) {
        QDateTime lastPlayed = QDateTime::fromString(domDoc.elementsByTagName("lastplayed").at(0).toElement().text(), "yyyy-MM-dd HH:mm:ss");
        if (!lastPlayed.isValid())
            lastPlayed = QDateTime::fromString(domDoc.elementsByTagName("lastplayed").at(0).toElement().text(), "yyyy-MM-dd");
        movie->setLastPlayed(lastPlayed);
    }
    if (!domDoc.elementsByTagName("dateadded").isEmpty())
        movie->setDateAdded(QDateTime::fromString(domDoc.elementsByTagName("dateadded").at(0).toElement().text(), "yyyy-MM-dd HH:mm:ss"));
    if (!domDoc.elementsByTagName("id").isEmpty())
        movie->setId(domDoc.elementsByTagName("id").at(0).toElement().text());
    if (!domDoc.elementsByTagName("tmdbid").isEmpty())
        movie->setTmdbId(domDoc.elementsByTagName("tmdbid").at(0).toElement().text());
    if (!domDoc.elementsByTagName("set").isEmpty())
        movie->setSet(domDoc.elementsByTagName("set").at(0).toElement().text());
    if (!domDoc.elementsByTagName("sorttitle").isEmpty())
        movie->setSortTitle(domDoc.elementsByTagName("sorttitle").at(0).toElement().text());
    if (!domDoc.elementsByTagName("trailer").isEmpty())
        movie->setTrailer(QUrl(domDoc.elementsByTagName("trailer").at(0).toElement().text()));
    if (!domDoc.elementsByTagName("watched").isEmpty()) {
        movie->setWatched(domDoc.elementsByTagName("watched").at(0).toElement().text() == "true" ? true : false);
    } else {
        movie->setWatched(movie->playcount() > 0);
    }

    for (int i=0, n=domDoc.elementsByTagName("studio").size() ; i<n ; i++)
        movie->addStudio(domDoc.elementsByTagName("studio").at(i).toElement().text());
    for (int i=0, n=domDoc.elementsByTagName("genre").size() ; i<n ; i++)
        movie->addGenre(domDoc.elementsByTagName("genre").at(i).toElement().text());
    for (int i=0, n=domDoc.elementsByTagName("country").size() ; i<n ; i++)
        movie->addCountry(domDoc.elementsByTagName("country").at(i).toElement().text());
    for (int i=0, n=domDoc.elementsByTagName("tag").size() ; i<n ; i++)
        movie->addTag(domDoc.elementsByTagName("tag").at(i).toElement().text());
    for (int i=0, n=domDoc.elementsByTagName("actor").size() ; i<n ; i++) {
        Actor a;
        a.imageHasChanged = false;
        if (!domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("name").isEmpty())
            a.name = domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("name").at(0).toElement().text();
        if (!domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("role").isEmpty())
            a.role = domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("role").at(0).toElement().text();
        if (!domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("thumb").isEmpty())
            a.thumb = domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("thumb").at(0).toElement().text();
        movie->addActor(a);
    }
    for (int i=0, n=domDoc.elementsByTagName("thumb").size() ; i<n ; i++) {
        QString parentTag = domDoc.elementsByTagName("thumb").at(i).parentNode().toElement().tagName();
        if (parentTag == "movie") {
            Poster p;
            p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
            p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
            movie->addPoster(p);
        } else if (parentTag == "fanart") {
            Poster p;
            p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
            p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
            movie->addBackdrop(p);
        }
    }

    movie->setStreamDetailsLoaded(loadStreamDetails(movie->streamDetails(), domDoc));

    // Existence of images
    if (initialNfoContent.isEmpty()) {
        foreach (const int &imageType, Movie::imageTypes())
            movie->setHasImage(imageType, !imageFileName(movie, imageType).isEmpty());
        movie->setHasExtraFanarts(!extraFanartNames(movie).isEmpty());
    }

    return true;
}

/**
 * @brief Loads the stream details from the dom document
 * @param streamDetails StreamDetails object
 * @param domDoc Nfo document
 * @return Infos loaded
 */
bool XbmcXml::loadStreamDetails(StreamDetails* streamDetails, QDomDocument domDoc)
{
    streamDetails->clear();
    if (!domDoc.elementsByTagName("streamdetails").isEmpty()) {
        QDomElement elem = domDoc.elementsByTagName("streamdetails").at(0).toElement();
        loadStreamDetails(streamDetails, elem);
        return true;
    }
    return false;
}

void XbmcXml::loadStreamDetails(StreamDetails* streamDetails, QDomElement elem)
{
    if (!elem.elementsByTagName("video").isEmpty()) {
        QDomElement videoElem = elem.elementsByTagName("video").at(0).toElement();
        QStringList details = (QStringList() << "codec" << "aspect" << "width" << "height" << "durationinseconds" << "scantype");
        foreach (const QString &detail, details) {
            if (!videoElem.elementsByTagName(detail).isEmpty())
                streamDetails->setVideoDetail(detail, videoElem.elementsByTagName(detail).at(0).toElement().text());
        }
    }
    if (!elem.elementsByTagName("audio").isEmpty()) {
        for (int i=0, n=elem.elementsByTagName("audio").count() ; i<n ; ++i) {
            QStringList details = QStringList() << "codec" << "language" << "channels";
            QDomElement audioElem = elem.elementsByTagName("audio").at(i).toElement();
            foreach (const QString &detail, details) {
                if (!audioElem.elementsByTagName(detail).isEmpty())
                    streamDetails->setAudioDetail(i, detail, audioElem.elementsByTagName(detail).at(0).toElement().text());
            }
        }
    }
    if (!elem.elementsByTagName("subtitle").isEmpty()) {
        for (int i=0, n=elem.elementsByTagName("subtitle").count() ; i<n ; ++i) {
            QStringList details = QStringList() << "language";
            QDomElement subtitleElem = elem.elementsByTagName("subtitle").at(i).toElement();
            foreach (const QString &detail, details) {
                if (!subtitleElem.elementsByTagName(detail).isEmpty())
                    streamDetails->setSubtitleDetail(i, detail, subtitleElem.elementsByTagName(detail).at(0).toElement().text());
            }
        }
    }
}

/**
 * @brief Writes streamdetails to xml stream
 * @param xml XML Stream
 * @param streamDetails Stream Details object
 */
void XbmcXml::writeStreamDetails(QXmlStreamWriter &xml, StreamDetails *streamDetails)
{
    if (streamDetails->videoDetails().isEmpty() && streamDetails->audioDetails().isEmpty() && streamDetails->subtitleDetails().isEmpty())
        return;

    xml.writeStartElement("fileinfo");
    xml.writeStartElement("streamdetails");

    xml.writeStartElement("video");
    QMapIterator<QString, QString> itVideo(streamDetails->videoDetails());
    while (itVideo.hasNext()) {
        itVideo.next();
        if (itVideo.key() == "width" && itVideo.value().toInt() == 0)
            continue;
        if (itVideo.key() == "height" && itVideo.value().toInt() == 0)
            continue;
        if (itVideo.key() == "durationinseconds" && itVideo.value().toInt() == 0)
            continue;
        if (itVideo.value() == "")
            continue;
        xml.writeTextElement(itVideo.key(), itVideo.value());
    }
    xml.writeEndElement();

    for (int i=0, n=streamDetails->audioDetails().count() ; i<n ; ++i) {
        xml.writeStartElement("audio");
        QMapIterator<QString, QString> itAudio(streamDetails->audioDetails().at(i));
        while (itAudio.hasNext()) {
            itAudio.next();
            if (itAudio.value() == "")
                continue;
            xml.writeTextElement(itAudio.key(), itAudio.value());
        }
        xml.writeEndElement();
    }

    for (int i=0, n=streamDetails->subtitleDetails().count() ; i<n ; ++i) {
        xml.writeStartElement("subtitle");
        QMapIterator<QString, QString> itSubtitle(streamDetails->subtitleDetails().at(i));
        while (itSubtitle.hasNext()) {
            itSubtitle.next();
            if (itSubtitle.value() == "")
                continue;
            xml.writeTextElement(itSubtitle.key(), itSubtitle.value());
        }
        xml.writeEndElement();
    }

    xml.writeEndElement();
    xml.writeEndElement();
}

/**
 * @brief Get the path to the actor image
 * @param movie
 * @param actor Actor
 * @return Path to actor image
 */
QString XbmcXml::actorImageName(Movie *movie, Actor actor)
{
    if (movie->files().isEmpty())
        return QString();
    QFileInfo fi(movie->files().at(0));
    QString actorName = actor.name;
    actorName = actorName.replace(" ", "_");
    QString path = fi.absolutePath() + "/" + ".actors" + "/" + actorName + ".jpg";
    fi.setFile(path);
    if (fi.isFile())
        return path;
    return QString();
}

/**
 * @brief Writes concert elements to an xml stream
 * @param xml XML stream
 * @param concert Concert to save
 */
void XbmcXml::writeConcertXml(QXmlStreamWriter &xml, Concert *concert)
{
    qDebug() << "Entered, concert=" << concert->name();
    xml.writeStartElement("musicvideo");
    xml.writeTextElement("title", concert->name());
    xml.writeTextElement("artist", concert->artist());
    xml.writeTextElement("album", concert->album());
    xml.writeTextElement("id", concert->id());
    xml.writeTextElement("tmdbid", concert->tmdbId());
    xml.writeTextElement("rating", QString("%1").arg(concert->rating()));
    xml.writeTextElement("year", concert->released().toString("yyyy"));
    xml.writeTextElement("plot", concert->overview());
    xml.writeTextElement("outline", concert->overview());
    xml.writeTextElement("tagline", concert->tagline());
    if (concert->runtime() > 0)
        xml.writeTextElement("runtime", QString("%1").arg(concert->runtime()));
    xml.writeTextElement("mpaa", concert->certification());
    xml.writeTextElement("playcount", QString("%1").arg(concert->playcount()));
    xml.writeTextElement("lastplayed", concert->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    xml.writeTextElement("trailer", Helper::formatTrailerUrl(concert->trailer().toString()));
    xml.writeTextElement("watched", (concert->watched()) ? "true" : "false");
    foreach (const QString &genre, concert->genres())
        xml.writeTextElement("genre", genre);
    foreach (const QString &tag, concert->tags())
        xml.writeTextElement("tag", tag);
    foreach (const Poster &poster, concert->posters()) {
        xml.writeStartElement("thumb");
        xml.writeAttribute("preview", poster.thumbUrl.toString());
        xml.writeCharacters(poster.originalUrl.toString());
        xml.writeEndElement();
    }
    xml.writeStartElement("fanart");
    foreach (const Poster &poster, concert->backdrops()) {
        xml.writeStartElement("thumb");
        xml.writeAttribute("preview", poster.thumbUrl.toString());
        xml.writeCharacters(poster.originalUrl.toString());
        xml.writeEndElement();
    }
    xml.writeEndElement();

    writeStreamDetails(xml, concert->streamDetails());

    xml.writeEndElement();
}

/**
 * @brief Saves a concert (including images)
 * @param concert Concert to save
 * @return Saving success
 * @see XbmcXml::writeConcertXml
 */
bool XbmcXml::saveConcert(Concert *concert)
{
    qDebug() << "Entered, concert=" << concert->name();
    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);
    writeConcertXml(xml, concert);
    xml.writeEndDocument();

    if (concert->files().size() == 0) {
        qWarning() << "Concert has no files";
        return false;
    }

    concert->setNfoContent(xmlContent);
    Manager::instance()->database()->update(concert);

    bool saved = false;
    QFileInfo fi(concert->files().at(0));
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::ConcertNfo)) {
        QString saveFileName = dataFile.saveFileName(fi.fileName(), -1, concert->files().count() > 1);
        QFile file(fi.absolutePath() + "/" + saveFileName);
        qDebug() << "Saving to" << file.fileName();
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "File could not be openend";
        } else {
            file.write(xmlContent);
            file.close();
            saved = true;
        }
    }
    if (!saved)
        return false;

    foreach (const int &imageType, Concert::imageTypes()) {
        int dataFileType = DataFile::dataFileTypeForImageType(imageType);
        if (concert->imageHasChanged(imageType) && !concert->image(imageType).isNull()) {
            foreach (DataFile dataFile, Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName = dataFile.saveFileName(fi.fileName(), -1, concert->files().count() > 1);
                if (imageType == ImageType::ConcertPoster && (concert->discType() == DiscBluRay || concert->discType() == DiscDvd))
                    saveFileName = "poster.jpg";
                if (imageType == ImageType::ConcertBackdrop && (concert->discType() == DiscBluRay || concert->discType() == DiscDvd))
                    saveFileName = "fanart.jpg";
                QString path = getPath(concert);
                saveFile(path + "/" + saveFileName, concert->image(imageType));
            }
        }
        if (concert->imagesToRemove().contains(imageType)) {
            foreach (DataFile dataFile, Settings::instance()->dataFiles(imageType)) {
                QString saveFileName = dataFile.saveFileName(fi.fileName(), -1, concert->files().count() > 1);
                if (imageType == ImageType::ConcertPoster && (concert->discType() == DiscBluRay || concert->discType() == DiscDvd))
                    saveFileName = "poster.jpg";
                if (imageType == ImageType::ConcertBackdrop && (concert->discType() == DiscBluRay || concert->discType() == DiscDvd))
                    saveFileName = "fanart.jpg";
                QString path = getPath(concert);
                QFile(path + "/" + saveFileName).remove();
            }
        }
    }

    if (concert->inSeparateFolder() && !concert->files().isEmpty()) {
        foreach (const QString &file, concert->extraFanartsToRemove())
            QFile::remove(file);
        QDir dir(QFileInfo(concert->files().first()).absolutePath() + "/extrafanart");
        if (!dir.exists() && !concert->extraFanartImagesToAdd().isEmpty())
            QDir(QFileInfo(concert->files().first()).absolutePath()).mkdir("extrafanart");
        foreach (QByteArray img, concert->extraFanartImagesToAdd()) {
            int num = 1;
            while (QFileInfo(dir.absolutePath() + "/" + QString("fanart%1.jpg").arg(num)).exists())
                ++num;
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
bool XbmcXml::loadConcert(Concert *concert, QString initialNfoContent)
{
    concert->clear();
    concert->setChanged(false);

    QString nfoContent;
    if (initialNfoContent.isEmpty()) {
        QString nfoFile = nfoFilePath(concert);
        if (nfoFile.isEmpty())
            return false;

        QFile file(nfoFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "File" << nfoFile << "could not be opened for reading";
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
    if (!domDoc.elementsByTagName("id").isEmpty() )
        concert->setId(domDoc.elementsByTagName("id").at(0).toElement().text());
    if (!domDoc.elementsByTagName("tmdbid").isEmpty() )
        concert->setTmdbId(domDoc.elementsByTagName("tmdbid").at(0).toElement().text());
    if (!domDoc.elementsByTagName("title").isEmpty() )
        concert->setName(domDoc.elementsByTagName("title").at(0).toElement().text());
    if (!domDoc.elementsByTagName("artist").isEmpty() )
        concert->setArtist(domDoc.elementsByTagName("artist").at(0).toElement().text());
    if (!domDoc.elementsByTagName("album").isEmpty() )
        concert->setAlbum(domDoc.elementsByTagName("album").at(0).toElement().text());
    if (!domDoc.elementsByTagName("rating").isEmpty())
        concert->setRating(domDoc.elementsByTagName("rating").at(0).toElement().text().toFloat());
    if (!domDoc.elementsByTagName("year").isEmpty())
        concert->setReleased(QDate::fromString(domDoc.elementsByTagName("year").at(0).toElement().text(), "yyyy"));
    if (!domDoc.elementsByTagName("plot").isEmpty())
        concert->setOverview(domDoc.elementsByTagName("plot").at(0).toElement().text());
    if (!domDoc.elementsByTagName("tagline").isEmpty())
        concert->setTagline(domDoc.elementsByTagName("tagline").at(0).toElement().text());
    if (!domDoc.elementsByTagName("runtime").isEmpty())
        concert->setRuntime(domDoc.elementsByTagName("runtime").at(0).toElement().text().toInt());
    if (!domDoc.elementsByTagName("mpaa").isEmpty())
        concert->setCertification(domDoc.elementsByTagName("mpaa").at(0).toElement().text());
    if (!domDoc.elementsByTagName("playcount").isEmpty())
        concert->setPlayCount(domDoc.elementsByTagName("playcount").at(0).toElement().text().toInt());
    if (!domDoc.elementsByTagName("lastplayed").isEmpty())
        concert->setLastPlayed(QDateTime::fromString(domDoc.elementsByTagName("lastplayed").at(0).toElement().text(), "yyyy-MM-dd HH:mm:ss"));
    if (!domDoc.elementsByTagName("trailer").isEmpty())
        concert->setTrailer(QUrl(domDoc.elementsByTagName("trailer").at(0).toElement().text()));
    if (!domDoc.elementsByTagName("watched").isEmpty())
        concert->setWatched(domDoc.elementsByTagName("watched").at(0).toElement().text() == "true" ? true : false);

    for (int i=0, n=domDoc.elementsByTagName("genre").size() ; i<n ; i++)
        concert->addGenre(domDoc.elementsByTagName("genre").at(i).toElement().text());
    for (int i=0, n=domDoc.elementsByTagName("tag").size() ; i<n ; i++)
        concert->addTag(domDoc.elementsByTagName("tag").at(i).toElement().text());
    for (int i=0, n=domDoc.elementsByTagName("thumb").size() ; i<n ; i++) {
        QString parentTag = domDoc.elementsByTagName("thumb").at(i).parentNode().toElement().tagName();
        if (parentTag == "movie") {
            Poster p;
            p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
            p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
            concert->addPoster(p);
        } else if (parentTag == "fanart") {
            Poster p;
            p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
            p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
            concert->addBackdrop(p);
        }
    }

    concert->setStreamDetailsLoaded(loadStreamDetails(concert->streamDetails(), domDoc));

    return true;
}

/**
 * @brief Get path to actor image
 * @param show
 * @param actor
 * @return Path to actor image
 */
QString XbmcXml::actorImageName(TvShow *show, Actor actor)
{
    if (show->dir().isEmpty())
        return QString();
    QString actorName = actor.name;
    actorName = actorName.replace(" ", "_");
    QString fileName = show->dir() + "/" + ".actors" + "/" + actorName + ".jpg";
    QFileInfo fi(fileName);
    if (fi.isFile())
        return fileName;
    return QString();
}

/**
 * @brief Loads tv show information
 * @param show Show to load
 * @return Loading success
 */
bool XbmcXml::loadTvShow(TvShow *show, QString initialNfoContent)
{
    show->clear();
    show->setChanged(false);

    QString nfoContent;
    if (initialNfoContent.isEmpty()) {
        if (show->dir().isEmpty())
            return false;

        QString nfoFile;
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowNfo)) {
            QString file = dataFile.saveFileName("");
            QFileInfo nfoFi(show->dir() + "/" + file);
            if (nfoFi.exists()) {
                nfoFile = show->dir() + "/" + file;
                break;
            }
        }
        QFile file(nfoFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Nfo file could not be opened for reading" << nfoFile;
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
    if (!domDoc.elementsByTagName("id").isEmpty() )
        show->setId(domDoc.elementsByTagName("id").at(0).toElement().text());
    if (!domDoc.elementsByTagName("tvdbid").isEmpty() )
        show->setTvdbId(domDoc.elementsByTagName("tvdbid").at(0).toElement().text());
    if (!domDoc.elementsByTagName("imdbid").isEmpty() )
        show->setImdbId(domDoc.elementsByTagName("imdbid").at(0).toElement().text());
    if (!domDoc.elementsByTagName("title").isEmpty() )
        show->setName(domDoc.elementsByTagName("title").at(0).toElement().text());
    if (!domDoc.elementsByTagName("sorttitle").isEmpty() )
        show->setSortTitle(domDoc.elementsByTagName("sorttitle").at(0).toElement().text());
    if (!domDoc.elementsByTagName("showtitle").isEmpty() )
        show->setShowTitle(domDoc.elementsByTagName("showtitle").at(0).toElement().text());
    if (!domDoc.elementsByTagName("rating").isEmpty())
        show->setRating(domDoc.elementsByTagName("rating").at(0).toElement().text().toFloat());
    if (!domDoc.elementsByTagName("plot").isEmpty())
        show->setOverview(domDoc.elementsByTagName("plot").at(0).toElement().text());
    if (!domDoc.elementsByTagName("mpaa").isEmpty())
        show->setCertification(domDoc.elementsByTagName("mpaa").at(0).toElement().text());
    if (!domDoc.elementsByTagName("premiered").isEmpty())
        show->setFirstAired(QDate::fromString(domDoc.elementsByTagName("premiered").at(0).toElement().text(), "yyyy-MM-dd"));
    if (!domDoc.elementsByTagName("studio").isEmpty())
        show->setNetwork(domDoc.elementsByTagName("studio").at(0).toElement().text());
    if (!domDoc.elementsByTagName("episodeguide").isEmpty() && !domDoc.elementsByTagName("episodeguide").at(0).toElement().elementsByTagName("url").isEmpty())
        show->setEpisodeGuideUrl(domDoc.elementsByTagName("episodeguide").at(0).toElement().elementsByTagName("url").at(0).toElement().text());
    if (!domDoc.elementsByTagName("runtime").isEmpty())
        show->setRuntime(domDoc.elementsByTagName("runtime").at(0).toElement().text().toInt());

    for (int i=0, n=domDoc.elementsByTagName("genre").size() ; i<n ; i++)
        show->addGenre(domDoc.elementsByTagName("genre").at(i).toElement().text());
    for (int i=0, n=domDoc.elementsByTagName("tag").size() ; i<n ; i++)
        show->addTag(domDoc.elementsByTagName("tag").at(i).toElement().text());
    for (int i=0, n=domDoc.elementsByTagName("actor").size() ; i<n ; i++) {
        Actor a;
        a.imageHasChanged = false;
        if (!domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("name").isEmpty())
            a.name = domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("name").at(0).toElement().text();
        if (!domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("role").isEmpty())
            a.role = domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("role").at(0).toElement().text();
        if (!domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("thumb").isEmpty())
            a.thumb = domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("thumb").at(0).toElement().text();
        show->addActor(a);
    }
    for (int i=0, n=domDoc.elementsByTagName("thumb").size() ; i<n ; i++) {
        QString parentTag = domDoc.elementsByTagName("thumb").at(i).parentNode().toElement().tagName();
        if (parentTag == "tvshow") {
            QDomElement elem = domDoc.elementsByTagName("thumb").at(i).toElement();
            Poster p;
            p.originalUrl = QUrl(elem.text());
            p.thumbUrl = QUrl(elem.text());
            if (elem.hasAttribute("type") && elem.attribute("type") == "season") {
                int season = elem.attribute("season").toInt();
                if (season >= 0)
                    show->addSeasonPoster(season, p);
            } else {
                show->addPoster(p);
            }
        } else if (parentTag == "fanart") {
            QString url = domDoc.elementsByTagName("thumb").at(i).parentNode().toElement().attribute("url");
            Poster p;
            p.originalUrl = QUrl(url + domDoc.elementsByTagName("thumb").at(i).toElement().text());
            p.thumbUrl = QUrl(url + domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
            show->addBackdrop(p);
        }
    }

    QFileInfo fi(show->dir() + "/theme.mp3");
    show->setHasTune(fi.isFile());

    return true;
}

/**
 * @brief Loads tv show episode information
 * @param episode Episode to load infos for
 * @return Loading success
 */
bool XbmcXml::loadTvShowEpisode(TvShowEpisode *episode, QString initialNfoContent)
{
    episode->clear();
    episode->setChanged(false);

    QString nfoContent;
    if (initialNfoContent.isEmpty()) {
        QString nfoFile = nfoFilePath(episode);
        if (nfoFile.isEmpty())
            return false;

        QFile file(nfoFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "File" << nfoFile << "could not be opened for reading";
            return false;
        }
        nfoContent = QString::fromUtf8(file.readAll());
        episode->setNfoContent(nfoContent);
        file.close();
    } else {
        nfoContent = initialNfoContent;
    }

    QString def;
    QStringList baseNfoContent;
    foreach (const QString &line, nfoContent.split("\n")) {
        if (!line.startsWith("<?xml"))
            baseNfoContent << line;
        else
            def = line;
    }
    QString nfoContentWithRoot = QString("%1\n<root>%2</root>").arg(def).arg(baseNfoContent.join("\n"));
    QDomDocument domDoc;
    domDoc.setContent(nfoContentWithRoot);

    QDomNodeList episodeDetailsList = domDoc.elementsByTagName("episodedetails");
    if (episodeDetailsList.isEmpty())
        return false;

    QDomElement episodeDetails;
    if (episodeDetailsList.count() > 1) {
        bool found = false;
        for (int i=0, n=episodeDetailsList.count() ; i<n ; ++i) {
            episodeDetails = episodeDetailsList.at(i).toElement();
            if (!episodeDetails.elementsByTagName("season").isEmpty() &&
                    episodeDetails.elementsByTagName("season").at(0).toElement().text().toInt() == episode->season() &&
                    !episodeDetails.elementsByTagName("episode").isEmpty() &&
                    episodeDetails.elementsByTagName("episode").at(0).toElement().text().toInt() == episode->episode()) {
                found = true;
                break;
            }
        }
        if (!found)
            return false;

    } else {
        episodeDetails = episodeDetailsList.at(0).toElement();
    }

    if (!episodeDetails.elementsByTagName("title").isEmpty() )
        episode->setName(episodeDetails.elementsByTagName("title").at(0).toElement().text());
    if (!episodeDetails.elementsByTagName("showtitle").isEmpty() )
        episode->setShowTitle(episodeDetails.elementsByTagName("showtitle").at(0).toElement().text());
    if (!episodeDetails.elementsByTagName("season").isEmpty())
        episode->setSeason(episodeDetails.elementsByTagName("season").at(0).toElement().text().toInt());
    if (!episodeDetails.elementsByTagName("episode").isEmpty())
        episode->setEpisode(episodeDetails.elementsByTagName("episode").at(0).toElement().text().toInt());
    if (!episodeDetails.elementsByTagName("displayseason").isEmpty())
        episode->setDisplaySeason(episodeDetails.elementsByTagName("displayseason").at(0).toElement().text().toInt());
    if (!episodeDetails.elementsByTagName("displayepisode").isEmpty())
        episode->setDisplayEpisode(episodeDetails.elementsByTagName("displayepisode").at(0).toElement().text().toInt());
    if (!episodeDetails.elementsByTagName("rating").isEmpty())
        episode->setRating(episodeDetails.elementsByTagName("rating").at(0).toElement().text().toFloat());
    if (!episodeDetails.elementsByTagName("plot").isEmpty())
        episode->setOverview(episodeDetails.elementsByTagName("plot").at(0).toElement().text());
    if (!episodeDetails.elementsByTagName("mpaa").isEmpty())
        episode->setCertification(episodeDetails.elementsByTagName("mpaa").at(0).toElement().text());
    if (!episodeDetails.elementsByTagName("aired").isEmpty())
        episode->setFirstAired(QDate::fromString(episodeDetails.elementsByTagName("aired").at(0).toElement().text(), "yyyy-MM-dd"));
    if (!episodeDetails.elementsByTagName("playcount").isEmpty())
        episode->setPlayCount(episodeDetails.elementsByTagName("playcount").at(0).toElement().text().toInt());
    if (!episodeDetails.elementsByTagName("epbookmark").isEmpty())
        episode->setEpBookmark(QTime().addSecs(episodeDetails.elementsByTagName("epbookmark").at(0).toElement().text().toInt()));
    if (!episodeDetails.elementsByTagName("lastplayed").isEmpty())
        episode->setLastPlayed(QDateTime::fromString(episodeDetails.elementsByTagName("lastplayed").at(0).toElement().text(), "yyyy-MM-dd HH:mm:ss"));
    if (!episodeDetails.elementsByTagName("studio").isEmpty())
        episode->setNetwork(episodeDetails.elementsByTagName("studio").at(0).toElement().text());
    if (!episodeDetails.elementsByTagName("thumb").isEmpty())
        episode->setThumbnail(QUrl(episodeDetails.elementsByTagName("thumb").at(0).toElement().text()));
    for (int i=0, n=episodeDetails.elementsByTagName("credits").size() ; i<n ; i++)
        episode->addWriter(episodeDetails.elementsByTagName("credits").at(i).toElement().text());
    for (int i=0, n=episodeDetails.elementsByTagName("director").size() ; i<n ; i++)
        episode->addDirector(episodeDetails.elementsByTagName("director").at(i).toElement().text());

    if (episodeDetails.elementsByTagName("streamdetails").count() > 0) {
        loadStreamDetails(episode->streamDetails(), episodeDetails.elementsByTagName("streamdetails").at(0).toElement());
        episode->setStreamDetailsLoaded(true);
    } else {
        episode->setStreamDetailsLoaded(false);
    }

    return true;
}

/**
 * @brief Saves a tv show
 * @param show Show to save
 * @return Saving success
 * @see XbmcXml::writeTvShowXml
 */
bool XbmcXml::saveTvShow(TvShow *show)
{
    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);
    writeTvShowXml(xml, show);
    xml.writeEndDocument();

    if (show->dir().isEmpty())
        return false;

    show->setNfoContent(xmlContent);
    Manager::instance()->database()->update(show);

    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowNfo)) {
        QFile file(show->dir() + "/" + dataFile.saveFileName(""));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Nfo file could not be openend for writing" << file.fileName();
            return false;
        }
        file.write(xmlContent);
        file.close();
    }

    foreach (const int &imageType, TvShow::imageTypes()) {
        int dataFileType = DataFile::dataFileTypeForImageType(imageType);
        if (show->imageHasChanged(imageType) && !show->image(imageType).isNull()) {
            foreach (DataFile dataFile, Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName = dataFile.saveFileName("");
                saveFile(show->dir() + "/" + saveFileName, show->image(imageType));
            }
        }
        if (show->imagesToRemove().contains(imageType)) {
            foreach (DataFile dataFile, Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName = dataFile.saveFileName("");
                QFile(show->dir() + "/" + saveFileName).remove();
            }
        }
    }

    foreach (const int &imageType, TvShow::seasonImageTypes()) {
        int dataFileType = DataFile::dataFileTypeForImageType(imageType);
        foreach (int season, show->seasons()) {
            if (show->seasonImageHasChanged(season, imageType) && !show->seasonImage(season, imageType).isNull()) {
                foreach (DataFile dataFile, Settings::instance()->dataFiles(dataFileType)) {
                    QString saveFileName = dataFile.saveFileName("", season);
                    saveFile(show->dir() + "/" + saveFileName, show->seasonImage(season, imageType));
                }
            }
            if (show->imagesToRemove().contains(imageType) && show->imagesToRemove().value(imageType).contains(season)) {
                foreach (DataFile dataFile, Settings::instance()->dataFiles(dataFileType)) {
                    QString saveFileName = dataFile.saveFileName("", season);
                    QFile(show->dir() + "/" + saveFileName).remove();
                }
            }
        }
    }

    if (!show->dir().isEmpty()) {
        foreach (const QString &file, show->extraFanartsToRemove())
            QFile::remove(file);
        QDir dir(show->dir() + "/extrafanart");
        if (!dir.exists() && !show->extraFanartImagesToAdd().isEmpty())
            QDir(show->dir()).mkdir("extrafanart");
        foreach (QByteArray img, show->extraFanartImagesToAdd()) {
            int num = 1;
            while (QFileInfo(dir.absolutePath() + "/" + QString("fanart%1.jpg").arg(num)).exists())
                ++num;
            saveFile(dir.absolutePath() + "/" + QString("fanart%1.jpg").arg(num), img);
        }
    }

    foreach (const Actor &actor, show->actors()) {
        if (!actor.image.isNull()) {
            QDir dir;
            dir.mkdir(show->dir() + "/" + ".actors");
            QString actorName = actor.name;
            actorName = actorName.replace(" ", "_");
            saveFile(show->dir() + "/" + ".actors" + "/" + actorName + ".jpg", actor.image);
        }
    }

    return true;
}

/**
 * @brief Saves a tv show episode
 * @param episode Episode to save
 * @return Saving success
 * @see XbmcXml::writeTvShowEpisodeXml
 */
bool XbmcXml::saveTvShowEpisode(TvShowEpisode *episode)
{
    qDebug() << "Entered, episode=" << episode->name();

    // Multi-Episode handling
    QList<TvShowEpisode*> episodes;
    foreach (TvShowEpisode *subEpisode, episode->tvShow()->episodes()) {
        if (episode->files() == subEpisode->files())
            episodes.append(subEpisode);
    }

    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);
    foreach (TvShowEpisode *subEpisode, episodes) {
        writeTvShowEpisodeXml(xml, subEpisode);
        subEpisode->setChanged(false);
        subEpisode->setSyncNeeded(true);
    }
    xml.writeEndDocument();

    if (episode->files().isEmpty()) {
        qWarning() << "Episode has no files";
        return false;
    }

    foreach (TvShowEpisode *subEpisode, episodes) {
        subEpisode->setNfoContent(xmlContent);
        Manager::instance()->database()->update(subEpisode);
    }

    QFileInfo fi(episode->files().at(0));
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowEpisodeNfo)) {
        QString saveFileName = dataFile.saveFileName(fi.fileName(), -1, episode->files().count() > 1);
        QFile file(fi.absolutePath() + "/" + saveFileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Nfo file could not be opened for writing" << saveFileName;
            return false;
        }
        file.write(xmlContent);
        file.close();
    }

    fi.setFile(episode->files().at(0));
    if (episode->thumbnailImageChanged() && !episode->thumbnailImage().isNull()) {
        if (Helper::isBluRay(episode->files().at(0)) || Helper::isDvd(episode->files().at(0))) {
            QDir dir = fi.dir();
            dir.cdUp();
            saveFile(dir.absolutePath() + "/thumb.jpg", episode->thumbnailImage());
        } else if (Helper::isDvd(episode->files().at(0), true)) {
            saveFile(fi.dir().absolutePath() + "/thumb.jpg", episode->thumbnailImage());
        } else {
            foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowEpisodeThumb)) {
                QString saveFileName = dataFile.saveFileName(fi.fileName(), -1, episode->files().count() > 1);
                saveFile(fi.absolutePath() + "/" + saveFileName, episode->thumbnailImage());
            }
        }
    }

    fi.setFile(episode->files().at(0));
    if (episode->imagesToRemove().contains(ImageType::TvShowEpisodeThumb)) {
        if (Helper::isBluRay(episode->files().at(0)) || Helper::isDvd(episode->files().at(0))) {
            QDir dir = fi.dir();
            dir.cdUp();
            QFile(dir.absolutePath() + "/thumb.jpg").remove();
        } else if (Helper::isDvd(episode->files().at(0), true)) {
            QFile(fi.dir().absolutePath() + "/thumb.jpg").remove();
        } else {
            foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowEpisodeThumb)) {
                QString saveFileName = dataFile.saveFileName(fi.fileName(), -1, episode->files().count() > 1);
                QFile(fi.absolutePath() + "/" + saveFileName).remove();
            }
        }
    }

    return true;
}

/**
 * @brief Writes tv show elements to an xml stream
 * @param xml XML stream
 * @param show Tv show to save
 */
void XbmcXml::writeTvShowXml(QXmlStreamWriter &xml, TvShow *show)
{
    qDebug() << "Entered, show=" << show->name();
    xml.writeStartElement("tvshow");
    xml.writeTextElement("title", show->name());
    xml.writeTextElement("showtitle", show->showTitle());
    if (!show->sortTitle().isEmpty()) {
        xml.writeStartElement("sorttitle");
        xml.writeAttribute("clear", "true");
        xml.writeCharacters(show->sortTitle());
        xml.writeEndElement();
    }
    xml.writeTextElement("rating", QString("%1").arg(show->rating()));
    xml.writeTextElement("episode", QString("%1").arg(show->episodes().count()));
    xml.writeTextElement("plot", show->overview());
    xml.writeTextElement("outline", show->overview());
    xml.writeTextElement("mpaa", QString("%1").arg(show->certification()));
    xml.writeTextElement("premiered", show->firstAired().toString("yyyy-MM-dd"));
    xml.writeTextElement("studio", show->network());
    xml.writeTextElement("tvdbid", show->tvdbId());
    xml.writeTextElement("id", show->id());
    xml.writeTextElement("imdbId", show->imdbId());
    if (show->runtime() > 0)
        xml.writeTextElement("runtime", QString("%1").arg(show->runtime()));

    if (!show->episodeGuideUrl().isEmpty()) {
        xml.writeStartElement("episodeguide");
        xml.writeTextElement("url", show->episodeGuideUrl());
        xml.writeEndElement();
    }

    foreach (const QString &genre, show->genres())
        xml.writeTextElement("genre", genre);
    foreach (const QString &tag, show->tags())
        xml.writeTextElement("tag", tag);

    foreach (const Actor &actor, show->actors()) {
        xml.writeStartElement("actor");
        xml.writeTextElement("name", actor.name);
        xml.writeTextElement("role", actor.role);
        if (!actor.thumb.isEmpty())
            xml.writeTextElement("thumb", actor.thumb);
        xml.writeEndElement();
    }

    foreach (const Poster &poster, show->posters()) {
        xml.writeStartElement("thumb");
        xml.writeCharacters(poster.originalUrl.toString());
        xml.writeEndElement();
        xml.writeStartElement("thumb");
        xml.writeAttribute("type", "season");
        xml.writeAttribute("season", "-1");
        xml.writeCharacters(poster.originalUrl.toString());
        xml.writeEndElement();
    }

    xml.writeStartElement("fanart");
    foreach (const Poster &poster, show->backdrops()) {
        xml.writeStartElement("thumb");
        xml.writeAttribute("preview", poster.thumbUrl.toString());
        xml.writeCharacters(poster.originalUrl.toString());
        xml.writeEndElement();
    }
    xml.writeEndElement();

    foreach (int season, show->seasons()) {
        foreach (const Poster &poster, show->seasonPosters(season)) {
            xml.writeStartElement("thumb");
            xml.writeAttribute("type", "season");
            xml.writeAttribute("season", QString("%1").arg(season));
            xml.writeCharacters(poster.originalUrl.toString());
            xml.writeEndElement();
        }
    }

    xml.writeEndElement();
}

/**
 * @brief Writes tv show episode elements to an xml stream
 * @param xml XML stream
 * @param episode Episode to save
 */
void XbmcXml::writeTvShowEpisodeXml(QXmlStreamWriter &xml, TvShowEpisode *episode)
{
    qDebug() << "Entered, episode=" << episode->name();
    xml.writeStartElement("episodedetails");
    xml.writeTextElement("title", episode->name());
    xml.writeTextElement("showtitle", episode->showTitle());
    xml.writeTextElement("rating", QString("%1").arg(episode->rating()));
    xml.writeTextElement("season", QString("%1").arg(episode->season()));
    xml.writeTextElement("episode", QString("%1").arg(episode->episode()));
    if (episode->displaySeason() > -1)
        xml.writeTextElement("displayseason", QString("%1").arg(episode->displaySeason()));
    if (episode->displayEpisode() > -1)
        xml.writeTextElement("displayepisode", QString("%1").arg(episode->displayEpisode()));
    xml.writeTextElement("plot", episode->overview());
    xml.writeTextElement("outline", episode->overview());
    xml.writeTextElement("mpaa", episode->certification());
    xml.writeTextElement("playcount", QString("%1").arg(episode->playCount()));
    xml.writeTextElement("lastplayed", episode->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    xml.writeTextElement("aired", episode->firstAired().toString("yyyy-MM-dd"));
    xml.writeTextElement("studio", episode->network());
    if (!episode->epBookmark().isNull() && QTime().secsTo(episode->epBookmark()) > 0)
        xml.writeTextElement("epbookmark", QString("%1").arg(QTime().secsTo(episode->epBookmark())));
    foreach (const QString &writer, episode->writers())
        xml.writeTextElement("credits", writer);
    foreach (const QString &director, episode->directors())
        xml.writeTextElement("director", director);
    if (!episode->thumbnail().isEmpty())
        xml.writeTextElement("thumb", episode->thumbnail().toString());

    if (episode->tvShow() != 0) {
        foreach (const Actor &actor, episode->tvShow()->actors()) {
            xml.writeStartElement("actor");
            xml.writeTextElement("name", actor.name);
            xml.writeTextElement("role", actor.role);
            xml.writeTextElement("thumb", actor.thumb);
            xml.writeEndElement();
        }
    }

    writeStreamDetails(xml, episode->streamDetails());

    xml.writeEndElement();
}

QStringList XbmcXml::extraFanartNames(Movie *movie)
{
    if (movie->files().isEmpty() || !movie->inSeparateFolder())
        return QStringList();
    QFileInfo fi(movie->files().first());
    QDir dir(fi.absolutePath() + "/extrafanart");
    QStringList files;
    QStringList filters = QStringList() << "*.jpg" << "*.jpeg" << "*.JPEG" << "*.Jpeg" << "*.JPeg";
    foreach (const QString &file, dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name))
        files << QDir::toNativeSeparators(dir.path() + "/" + file);
    return files;
}

QStringList XbmcXml::extraFanartNames(Concert *concert)
{
    if (concert->files().isEmpty() || !concert->inSeparateFolder())
        return QStringList();
    QFileInfo fi(concert->files().first());
    QDir dir(fi.absolutePath() + "/extrafanart");
    QStringList files;
    QStringList filters = QStringList() << "*.jpg" << "*.jpeg" << "*.JPEG" << "*.Jpeg" << "*.JPeg";
    foreach (const QString &file, dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name))
        files << QDir::toNativeSeparators(dir.path() + "/" + file);
    return files;
}

QStringList XbmcXml::extraFanartNames(TvShow *show)
{
    if (show->dir().isEmpty())
        return QStringList();
    QDir dir(show->dir() + "/extrafanart");
    QStringList files;
    QStringList filters = QStringList() << "*.jpg" << "*.jpeg" << "*.JPEG" << "*.Jpeg" << "*.JPeg";
    foreach (const QString &file, dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name))
        files << QDir::toNativeSeparators(dir.path() + "/" + file);
    return files;
}

QImage XbmcXml::movieSetPoster(QString setName)
{
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MovieSetPoster)) {
        QString fileName = movieSetFileName(setName, &dataFile);
        QFileInfo fi(fileName);
        if (fi.exists())
            return QImage(fi.absoluteFilePath());
    }
    return QImage();
}

QImage XbmcXml::movieSetBackdrop(QString setName)
{
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MovieSetBackdrop)) {
        QString fileName = movieSetFileName(setName, &dataFile);
        QFileInfo fi(fileName);
        if (fi.exists())
            return QImage(fi.absoluteFilePath());
    }
    return QImage();
}

/**
 * @brief Save movie set poster
 * @param setName
 * @param poster
 */
void XbmcXml::saveMovieSetPoster(QString setName, QImage poster)
{
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MovieSetPoster)) {
        QString fileName = movieSetFileName(setName, &dataFile);
        if (!fileName.isEmpty())
            poster.save(fileName, "jpg", 100);
    }
}

/**
 * @brief Save movie set backdrop
 * @param setName
 * @param backdrop
 */
void XbmcXml::saveMovieSetBackdrop(QString setName, QImage backdrop)
{
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MovieSetBackdrop)) {
        QString fileName = movieSetFileName(setName, &dataFile);
        if (!fileName.isEmpty())
            backdrop.save(fileName, "jpg", 100);
    }
}

bool XbmcXml::saveFile(QString filename, QByteArray data)
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
        return true;
    }
    return false;
}

QString XbmcXml::getPath(Movie *movie)
{
    if (movie->files().isEmpty())
        return QString();
    QFileInfo fi(movie->files().first());
    if (movie->discType() == DiscBluRay) {
        QDir dir = fi.dir();
        if (QString::compare(dir.dirName(), "BDMV", Qt::CaseInsensitive) == 0)
            dir.cdUp();
        return dir.absolutePath();
    } else if (movie->discType() == DiscDvd) {
        QDir dir = fi.dir();
        if (QString::compare(dir.dirName(), "VIDEO_TS", Qt::CaseInsensitive) == 0)
            dir.cdUp();
        return dir.absolutePath();
    }
    return fi.absolutePath();
}

QString XbmcXml::getPath(Concert *concert)
{
    if (concert->files().isEmpty())
        return QString();
    QFileInfo fi(concert->files().first());
    if (concert->discType() == DiscBluRay) {
        QDir dir = fi.dir();
        if (QString::compare(dir.dirName(), "BDMV", Qt::CaseInsensitive) == 0)
            dir.cdUp();
        return dir.absolutePath();
    } else if (concert->discType() == DiscDvd) {
        QDir dir = fi.dir();
        if (QString::compare(dir.dirName(), "VIDEO_TS", Qt::CaseInsensitive) == 0)
            dir.cdUp();
        return dir.absolutePath();
    }
    return fi.absolutePath();
}

QString XbmcXml::movieSetFileName(QString setName, DataFile *dataFile)
{
    if (Settings::instance()->movieSetArtworkType() == MovieSetArtworkSingleArtworkFolder) {
        QDir dir(Settings::instance()->movieSetArtworkDirectory());
        QString fileName = dataFile->saveFileName(setName);
        return dir.absolutePath() + "/" + fileName;
    } else if (Settings::instance()->movieSetArtworkType() == MovieSetArtworkSingleSetFolder) {
        foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
            if (movie->set() == setName && !movie->files().isEmpty()) {
                QFileInfo fi(movie->files().first());
                QDir dir = fi.dir();
                if (movie->inSeparateFolder())
                    dir.cdUp();
                if (movie->discType() == DiscDvd || movie->discType() == DiscBluRay)
                    dir.cdUp();
                return dir.absolutePath() + "/" + dataFile->saveFileName(setName);
            }
        }
    }

    return QString();
}

QString XbmcXml::imageFileName(Movie *movie, int type, QList<DataFile> dataFiles, bool constructName)
{
    int fileType;
    switch (type) {
    case ImageType::MoviePoster:
        fileType = DataFileType::MoviePoster;
        break;
    case ImageType::MovieBackdrop:
        fileType = DataFileType::MovieBackdrop;
        break;
    case ImageType::MovieLogo:
        fileType = DataFileType::MovieLogo;
        break;
    case ImageType::MovieBanner:
        fileType = DataFileType::MovieBanner;
        break;
    case ImageType::MovieThumb:
        fileType = DataFileType::MovieThumb;
        break;
    case ImageType::MovieClearArt:
        fileType = DataFileType::MovieClearArt;
        break;
    case ImageType::MovieCdArt:
        fileType = DataFileType::MovieCdArt;
        break;
    default:
        return "";
    }

    QString fileName;
    if (movie->files().size() == 0) {
        qWarning() << "Movie has no files";
        return fileName;
    }
    QFileInfo fi(movie->files().at(0));

    if (!constructName)
        dataFiles = Settings::instance()->dataFiles(fileType);

    foreach (DataFile dataFile, dataFiles) {
        QString file = dataFile.saveFileName(fi.fileName(), -1, movie->files().count() > 1);
        if (type == ImageType::MoviePoster && (movie->discType() == DiscBluRay || movie->discType() == DiscDvd))
            file = "poster.jpg";
        if (type == ImageType::MovieBackdrop && (movie->discType() == DiscBluRay || movie->discType() == DiscDvd))
            file = "fanart.jpg";
        QString path = getPath(movie);
        QFileInfo pFi(path + "/" + file);
        if (pFi.isFile() || constructName) {
            fileName = path + "/" + file;
            break;
        }
    }

    return fileName;
}

QString XbmcXml::imageFileName(Concert *concert, int type, QList<DataFile> dataFiles, bool constructName)
{
    int fileType;
    switch (type) {
    case ImageType::ConcertPoster:
        fileType = DataFileType::ConcertPoster;
        break;
    case ImageType::ConcertBackdrop:
        fileType = DataFileType::ConcertBackdrop;
        break;
    case ImageType::ConcertLogo:
        fileType = DataFileType::ConcertLogo;
        break;
    case ImageType::ConcertClearArt:
        fileType = DataFileType::ConcertClearArt;
        break;
    case ImageType::ConcertCdArt:
        fileType = DataFileType::ConcertCdArt;
        break;
    default:
        return "";
    }

    QString fileName;
    if (concert->files().size() == 0) {
        qWarning() << "Concert has no files";
        return fileName;
    }
    QFileInfo fi(concert->files().at(0));

    if (!constructName)
        dataFiles = Settings::instance()->dataFiles(fileType);

    foreach (DataFile dataFile, dataFiles) {
        QString file = dataFile.saveFileName(fi.fileName(), -1, concert->files().count() > 1);
        if (type == ImageType::ConcertPoster && (concert->discType() == DiscBluRay || concert->discType() == DiscDvd))
            file = "poster.jpg";
        if (type == ImageType::ConcertBackdrop && (concert->discType() == DiscBluRay || concert->discType() == DiscDvd))
            file = "fanart.jpg";
        QString path = getPath(concert);
        QFileInfo pFi(path + "/" + file);
        if (pFi.isFile() || constructName) {
            fileName = path + "/" + file;
            break;
        }
    }

    return fileName;
}

QString XbmcXml::imageFileName(TvShow *show, int type, int season, QList<DataFile> dataFiles, bool constructName)
{
    int fileType;
    switch (type) {
    case ImageType::TvShowPoster:
        fileType = DataFileType::TvShowPoster;
        break;
    case ImageType::TvShowBackdrop:
        fileType = DataFileType::TvShowBackdrop;
        break;
    case ImageType::TvShowLogos:
        fileType = DataFileType::TvShowLogo;
        break;
    case ImageType::TvShowBanner:
        fileType = DataFileType::TvShowBanner;
        break;
    case ImageType::TvShowThumb:
        fileType = DataFileType::TvShowThumb;
        break;
    case ImageType::TvShowClearArt:
        fileType = DataFileType::TvShowClearArt;
        break;
    case ImageType::TvShowCharacterArt:
        fileType = DataFileType::TvShowCharacterArt;
        break;
    case ImageType::TvShowSeasonPoster:
        fileType = DataFileType::TvShowSeasonPoster;
        break;
    case ImageType::TvShowSeasonBackdrop:
        fileType = DataFileType::TvShowSeasonBackdrop;
        break;
    case ImageType::TvShowSeasonBanner:
        fileType = DataFileType::TvShowSeasonBanner;
        break;
    case ImageType::TvShowSeasonThumb:
        fileType = DataFileType::TvShowSeasonThumb;
        break;
    default:
        return "";
    }

    if (show->dir().isEmpty())
        return QString();

    if (!constructName)
        dataFiles = Settings::instance()->dataFiles(fileType);

    QString fileName;
    foreach (DataFile dataFile, dataFiles) {
        QString loadFileName = dataFile.saveFileName("", season);
        QFileInfo fi(show->dir() + "/" + loadFileName);
        if (fi.isFile() || constructName) {
            fileName = show->dir() + "/" + loadFileName;
            break;
        }
    }
    return fileName;
}

QString XbmcXml::imageFileName(TvShowEpisode *episode, int type, QList<DataFile> dataFiles, bool constructName)
{
    int fileType;
    switch (type) {
    case ImageType::TvShowEpisodeThumb:
        fileType = DataFileType::TvShowEpisodeThumb;
        break;
    default:
        return "";
    }

    if (episode->files().isEmpty())
        return "";

    QString fileName;
    QFileInfo fi(episode->files().at(0));

    if (Helper::isBluRay(episode->files().at(0)) || Helper::isDvd(episode->files().at(0))) {
        QDir dir = fi.dir();
        dir.cdUp();
        fi.setFile(dir.absolutePath() + "/thumb.jpg");
        return fi.exists() ? fi.absoluteFilePath() : "";
    }

    if (Helper::isDvd(episode->files().at(0), true)) {
        fi.setFile(fi.dir().absolutePath() + "/thumb.jpg");
        return fi.exists() ? fi.absoluteFilePath() : "";
    }

    if (!constructName)
        dataFiles = Settings::instance()->dataFiles(fileType);

    foreach (DataFile dataFile, dataFiles) {
        QString file = dataFile.saveFileName(fi.fileName());
        QFileInfo pFi(fi.absolutePath() + "/" + file);
        if (pFi.isFile() || constructName) {
            fileName = fi.absolutePath() + "/" + file;
            break;
        }
    }

    return fileName;
}
