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
#include "image/Image.h"
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

QByteArray XbmcXml::getMovieXml(Movie *movie)
{
    QDomDocument doc;
    doc.setContent(movie->nfoContent());
    if (movie->nfoContent().isEmpty()) {
        QDomNode node = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"");
        doc.insertBefore(node, doc.firstChild());
        doc.appendChild(doc.createElement("movie"));
    }

    QDomElement movieElem = doc.elementsByTagName("movie").at(0).toElement();

    setTextValue(doc, "title", movie->name());
    setTextValue(doc, "originaltitle", movie->originalName());
    setTextValue(doc, "rating", QString("%1").arg(movie->rating()));
    setTextValue(doc, "votes", QString::number(movie->votes()));
    setTextValue(doc, "top250", QString::number(movie->top250()));
    setTextValue(doc, "year", movie->released().toString("yyyy"));
    setTextValue(doc, "plot", movie->overview());
    setTextValue(doc, "outline", movie->outline());
    setTextValue(doc, "tagline", movie->tagline());
    if (movie->runtime() > 0)
        setTextValue(doc, "runtime", QString("%1").arg(movie->runtime()));
    else
        removeChildNodes(doc, "runtime");
    setTextValue(doc, "mpaa", movie->certification());
    setTextValue(doc, "playcount", QString("%1").arg(movie->playcount()));
    if (!movie->lastPlayed().isNull())
        setTextValue(doc, "lastplayed", movie->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    else
        removeChildNodes(doc, "lastplayed");
    if (!movie->dateAdded().isNull())
        setTextValue(doc, "dateadded", movie->dateAdded().toString("yyyy-MM-dd HH:mm:ss"));
    else
        removeChildNodes(doc, "dateadded");
    setTextValue(doc, "id", movie->id());
    setTextValue(doc, "tmdbid", movie->tmdbId());
    setTextValue(doc, "set", movie->set());
    setTextValue(doc, "sorttitle", movie->sortTitle());
    setTextValue(doc, "trailer", Helper::instance()->formatTrailerUrl(movie->trailer().toString()));
    setTextValue(doc, "watched", (movie->watched()) ? "true" : "false");

    QStringList writers;
    foreach (const QString &credit, movie->writer().split(","))
        writers << credit.trimmed();
    setListValue(doc, "credits", writers);

    QStringList directors;
    foreach (const QString &director, movie->director().split(","))
        directors << director.trimmed();
    setListValue(doc, "director", directors);

    setListValue(doc, "studio", Settings::instance()->advanced()->useFirstStudioOnly() && !movie->studios().isEmpty() ? movie->studios().mid(0, 1) : movie->studios());
    setListValue(doc, "genre", movie->genres());
    setListValue(doc, "country", movie->countries());
    setListValue(doc, "tag", movie->tags());

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        removeChildNodes(doc, "thumb");
        removeChildNodes(doc, "fanart");

        foreach (const Poster &poster, movie->posters()) {
            QDomElement elem = doc.createElement("thumb");
            elem.setAttribute("preview", poster.thumbUrl.toString());
            elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            appendXmlNode(doc, elem);
        }

        if (!movie->backdrops().isEmpty()) {
            QDomElement fanartElem = doc.createElement("fanart");
            foreach (const Poster &poster, movie->backdrops()) {
                QDomElement elem = doc.createElement("thumb");
                elem.setAttribute("preview", poster.thumbUrl.toString());
                elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                fanartElem.appendChild(elem);
            }
            appendXmlNode(doc, fanartElem);
        }
    }

    removeChildNodes(doc, "actor");

    foreach (const Actor &actor, movie->actors()) {
        QDomElement elem = doc.createElement("actor");
        QDomElement elemName = doc.createElement("name");
        QDomElement elemRole = doc.createElement("role");
        elemName.appendChild(doc.createTextNode(actor.name));
        elemRole.appendChild(doc.createTextNode(actor.role));
        elem.appendChild(elemName);
        elem.appendChild(elemRole);
        if (!actor.thumb.isEmpty() && Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
            QDomElement elemThumb = doc.createElement("thumb");
            elemThumb.appendChild(doc.createTextNode(actor.thumb));
            elem.appendChild(elemThumb);
        }
        appendXmlNode(doc, elem);
    }

    writeStreamDetails(doc, movie->streamDetails(), movie->subtitles());

    return doc.toByteArray(4);
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
    QByteArray xmlContent = getMovieXml(movie);

    if (movie->files().size() == 0) {
        qWarning() << "Movie has no files";
        return false;
    }

    movie->setNfoContent(xmlContent);

    bool saved = false;
    QFileInfo fi(movie->files().at(0));
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MovieNfo)) {
        QString saveFileName = dataFile.saveFileName(fi.fileName(), -1, movie->files().count() > 1);
        QString saveFilePath = fi.absolutePath() + "/" + saveFileName;
        QDir saveFileDir = QFileInfo(saveFilePath).dir();
        if (!saveFileDir.exists())
            saveFileDir.mkpath(".");
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

    foreach (Subtitle *subtitle, movie->subtitles()) {
        if (subtitle->changed()) {
            QString subFileName = fi.completeBaseName();
            if (!subtitle->language().isEmpty())
                subFileName.append("." + subtitle->language());
            if (subtitle->forced())
                subFileName.append(".forced");

            QStringList newFiles;
            foreach (const QString &subFile, subtitle->files()) {
                QFileInfo subFi(fi.absolutePath() + "/" + subFile);
                QString newFileName = subFileName + "." + subFi.suffix();
                QFile f(fi.absolutePath() + "/" + subFile);
                if (f.rename(fi.absolutePath() + "/" + newFileName)) {
                    newFiles << newFileName;
                } else {
                    qWarning() << "Could not rename" << subFi.absoluteFilePath() << "to" << fi.absolutePath() + "/" + newFileName;
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

QString XbmcXml::nfoFilePath(TvShow *show)
{
    QString nfoFile;
    if (show->dir().isEmpty()) {
        qWarning() << "Show dir is empty";
        return nfoFile;
    }

    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowNfo)) {
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
    if (!domDoc.elementsByTagName("title").isEmpty())
        movie->setName(domDoc.elementsByTagName("title").at(0).toElement().text());
    if (!domDoc.elementsByTagName("originaltitle").isEmpty())
        movie->setOriginalName(domDoc.elementsByTagName("originaltitle").at(0).toElement().text());
    if (!domDoc.elementsByTagName("rating").isEmpty())
        movie->setRating(domDoc.elementsByTagName("rating").at(0).toElement().text().replace(",", ".").toFloat());
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

    QStringList writers;
    for (int i=0, n=domDoc.elementsByTagName("credits").size() ; i<n ; i++) {
        foreach (const QString &writer, domDoc.elementsByTagName("credits").at(i).toElement().text().split(",", QString::SkipEmptyParts))
            writers.append(writer.trimmed());
    }
    movie->setWriter(writers.join(", "));

    QStringList directors;
    for (int i=0, n=domDoc.elementsByTagName("director").size() ; i<n ; i++) {
        foreach (const QString &director, domDoc.elementsByTagName("director").at(i).toElement().text().split(",", QString::SkipEmptyParts))
            directors.append(director.trimmed());
    }
    movie->setDirector(directors.join(", "));

    for (int i=0, n=domDoc.elementsByTagName("studio").size() ; i<n ; i++) {
        foreach (const QString &studio, domDoc.elementsByTagName("studio").at(i).toElement().text().split("/", QString::SkipEmptyParts))
            movie->addStudio(studio.trimmed());
    }

    for (int i=0, n=domDoc.elementsByTagName("genre").size() ; i<n ; i++) {
        foreach (const QString &genre, domDoc.elementsByTagName("genre").at(i).toElement().text().split("/", QString::SkipEmptyParts))
            movie->addGenre(genre.trimmed());
    }

    for (int i=0, n=domDoc.elementsByTagName("country").size() ; i<n ; i++) {
        foreach (const QString &country, domDoc.elementsByTagName("country").at(i).toElement().text().split(" / ", QString::SkipEmptyParts))
            movie->addCountry(country.trimmed());
    }

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
        QStringList details = (QStringList() << "codec" << "aspect" << "width" << "height" << "durationinseconds" << "scantype" << "stereomode");
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
            if (!subtitleElem.elementsByTagName("file").isEmpty())
                continue;
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

        QString value = itVideo.value();

        if (itVideo.key() == "aspect")
            value = value.replace(",", ".");

        xml.writeTextElement(itVideo.key(), value);
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

void XbmcXml::writeStreamDetails(QDomDocument &doc, StreamDetails *streamDetails, QList<Subtitle*> subtitles)
{
    if (streamDetails->videoDetails().isEmpty() && streamDetails->audioDetails().isEmpty() && streamDetails->subtitleDetails().isEmpty() && subtitles.isEmpty())
        return;

    removeChildNodes(doc, "fileinfo");
    QDomElement elemFi = doc.createElement("fileinfo");
    QDomElement elemSd = doc.createElement("streamdetails");

    QDomElement elemVideo = doc.createElement("video");
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

        QString value = itVideo.value();

        if (itVideo.key() == "aspect")
            value = value.replace(",", ".");

        QDomElement elem = doc.createElement(itVideo.key());
        elem.appendChild(doc.createTextNode(value));
        elemVideo.appendChild(elem);
    }
    elemSd.appendChild(elemVideo);

    for (int i=0, n=streamDetails->audioDetails().count() ; i<n ; ++i) {
        QDomElement elemAudio = doc.createElement("audio");
        QMapIterator<QString, QString> itAudio(streamDetails->audioDetails().at(i));
        while (itAudio.hasNext()) {
            itAudio.next();
            if (itAudio.value() == "")
                continue;

            QDomElement elem = doc.createElement(itAudio.key());
            elem.appendChild(doc.createTextNode(itAudio.value()));
            elemAudio.appendChild(elem);
        }
        elemSd.appendChild(elemAudio);
    }

    for (int i=0, n=streamDetails->subtitleDetails().count() ; i<n ; ++i) {
        QDomElement elemSubtitle = doc.createElement("subtitle");
        QMapIterator<QString, QString> itSubtitle(streamDetails->subtitleDetails().at(i));
        while (itSubtitle.hasNext()) {
            itSubtitle.next();
            if (itSubtitle.value() == "")
                continue;

            QDomElement elem = doc.createElement(itSubtitle.key());
            elem.appendChild(doc.createTextNode(itSubtitle.value()));
            elemSubtitle.appendChild(elem);
        }
        elemSd.appendChild(elemSubtitle);
    }

    foreach (Subtitle *subtitle, subtitles) {
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

QByteArray XbmcXml::getConcertXml(Concert *concert)
{
    QDomDocument doc;
    doc.setContent(concert->nfoContent());
    if (concert->nfoContent().isEmpty()) {
        QDomNode node = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"");
        doc.insertBefore(node, doc.firstChild());
        doc.appendChild(doc.createElement("musicvideo"));
    }

    QDomElement concertElem = doc.elementsByTagName("musicvideo").at(0).toElement();

    setTextValue(doc, "title", concert->name());
    setTextValue(doc, "artist", concert->artist());
    setTextValue(doc, "album", concert->album());
    setTextValue(doc, "id", concert->id());
    setTextValue(doc, "tmdbid", concert->tmdbId());
    setTextValue(doc, "rating", QString("%1").arg(concert->rating()));
    setTextValue(doc, "year", concert->released().toString("yyyy"));
    setTextValue(doc, "plot", concert->overview());
    setTextValue(doc, "outline", concert->overview());
    setTextValue(doc, "tagline", concert->tagline());
    if (concert->runtime() > 0)
        setTextValue(doc, "runtime", QString("%1").arg(concert->runtime()));
    setTextValue(doc, "mpaa", concert->certification());
    setTextValue(doc, "playcount", QString("%1").arg(concert->playcount()));
    setTextValue(doc, "lastplayed", concert->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    setTextValue(doc, "trailer", Helper::instance()->formatTrailerUrl(concert->trailer().toString()));
    setTextValue(doc, "watched", (concert->watched()) ? "true" : "false");
    setTextValue(doc, "genre", concert->genres().join(" / "));
    setListValue(doc, "tag", concert->tags());

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        removeChildNodes(doc, "thumb");
        removeChildNodes(doc, "fanart");

        foreach (const Poster &poster, concert->posters()) {
            QDomElement elem = doc.createElement("thumb");
            elem.setAttribute("preview", poster.thumbUrl.toString());
            elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            appendXmlNode(doc, elem);
        }

        if (!concert->backdrops().isEmpty()) {
            QDomElement fanartElem = doc.createElement("fanart");
            foreach (const Poster &poster, concert->backdrops()) {
                QDomElement elem = doc.createElement("thumb");
                elem.setAttribute("preview", poster.thumbUrl.toString());
                elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                fanartElem.appendChild(elem);
            }
            appendXmlNode(doc, fanartElem);
        }
    }

    writeStreamDetails(doc, concert->streamDetails());

    return doc.toByteArray(4);
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
    QByteArray xmlContent = getConcertXml(concert);

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
        QString saveFilePath = fi.absolutePath() + "/" + saveFileName;
        QDir saveFileDir = QFileInfo(saveFilePath).dir();
        if (!saveFileDir.exists())
            saveFileDir.mkpath(".");
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
        concert->setRating(domDoc.elementsByTagName("rating").at(0).toElement().text().replace(",", ".").toFloat());
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

    for (int i=0, n=domDoc.elementsByTagName("genre").size() ; i<n ; i++) {
        foreach (const QString &genre, domDoc.elementsByTagName("genre").at(i).toElement().text().split(" / ", QString::SkipEmptyParts))
            concert->addGenre(genre);
    }
    for (int i=0, n=domDoc.elementsByTagName("tag").size() ; i<n ; i++)
        concert->addTag(domDoc.elementsByTagName("tag").at(i).toElement().text());
    for (int i=0, n=domDoc.elementsByTagName("thumb").size() ; i<n ; i++) {
        QString parentTag = domDoc.elementsByTagName("thumb").at(i).parentNode().toElement().tagName();
        if (parentTag == "musicvideo") {
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

    // Existence of images
    if (initialNfoContent.isEmpty()) {
        foreach (const int &imageType, Concert::imageTypes())
            concert->setHasImage(imageType, !imageFileName(concert, imageType).isEmpty());
        concert->setHasExtraFanarts(!extraFanartNames(concert).isEmpty());
    }

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

QString XbmcXml::actorImageName(TvShowEpisode *episode, Actor actor)
{
    if (episode->files().isEmpty())
        return QString();
    QFileInfo fi(episode->files().at(0));
    QString actorName = actor.name;
    actorName = actorName.replace(" ", "_");
    QString path = fi.absolutePath() + "/" + ".actors" + "/" + actorName + ".jpg";
    fi.setFile(path);
    if (fi.isFile())
        return path;
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
        show->setRating(domDoc.elementsByTagName("rating").at(0).toElement().text().replace(",", ".").toFloat());
    if (!domDoc.elementsByTagName("votes").isEmpty())
        show->setVotes(domDoc.elementsByTagName("votes").at(0).toElement().text().replace(",", "").replace(".", "").toInt());
    if (!domDoc.elementsByTagName("top250").isEmpty())
        show->setTop250(domDoc.elementsByTagName("top250").at(0).toElement().text().toInt());
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
    if (!domDoc.elementsByTagName("status").isEmpty())
        show->setStatus(domDoc.elementsByTagName("status").at(0).toElement().text());

    for (int i=0, n=domDoc.elementsByTagName("genre").size() ; i<n ; i++) {
        foreach (const QString &genre, domDoc.elementsByTagName("genre").at(i).toElement().text().split(" / ", QString::SkipEmptyParts))
            show->addGenre(genre);
    }
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

    if (!episodeDetails.elementsByTagName("imdbid").isEmpty())
        episode->setImdbId(episodeDetails.elementsByTagName("imdbid").at(0).toElement().text());
    if (!episodeDetails.elementsByTagName("title").isEmpty())
        episode->setName(episodeDetails.elementsByTagName("title").at(0).toElement().text());
    if (!episodeDetails.elementsByTagName("showtitle").isEmpty())
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
        episode->setRating(episodeDetails.elementsByTagName("rating").at(0).toElement().text().replace(",", ".").toFloat());
    if (!domDoc.elementsByTagName("votes").isEmpty())
        episode->setVotes(domDoc.elementsByTagName("votes").at(0).toElement().text().replace(",", "").replace(".", "").toInt());
    if (!domDoc.elementsByTagName("top250").isEmpty())
        episode->setTop250(domDoc.elementsByTagName("top250").at(0).toElement().text().toInt());
    if (!episodeDetails.elementsByTagName("plot").isEmpty())
        episode->setOverview(episodeDetails.elementsByTagName("plot").at(0).toElement().text());
    if (!episodeDetails.elementsByTagName("mpaa").isEmpty())
        episode->setCertification(episodeDetails.elementsByTagName("mpaa").at(0).toElement().text());
    if (!episodeDetails.elementsByTagName("aired").isEmpty())
        episode->setFirstAired(QDate::fromString(episodeDetails.elementsByTagName("aired").at(0).toElement().text(), "yyyy-MM-dd"));
    if (!episodeDetails.elementsByTagName("playcount").isEmpty())
        episode->setPlayCount(episodeDetails.elementsByTagName("playcount").at(0).toElement().text().toInt());
    if (!episodeDetails.elementsByTagName("epbookmark").isEmpty())
        episode->setEpBookmark(QTime(0, 0, 0).addSecs(episodeDetails.elementsByTagName("epbookmark").at(0).toElement().text().toInt()));
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
    for (int i=0, n=domDoc.elementsByTagName("actor").size() ; i<n ; i++) {
        Actor a;
        a.imageHasChanged = false;
        if (!domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("name").isEmpty())
            a.name = domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("name").at(0).toElement().text();
        if (!domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("role").isEmpty())
            a.role = domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("role").at(0).toElement().text();
        if (!domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("thumb").isEmpty())
            a.thumb = domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("thumb").at(0).toElement().text();
        episode->addActor(a);
    }

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
    QByteArray xmlContent = getTvShowXml(show);

    if (show->dir().isEmpty())
        return false;

    show->setNfoContent(xmlContent);
    Manager::instance()->database()->update(show);

    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowNfo)) {
        QString saveFilePath = show->dir() + "/" + dataFile.saveFileName("");
        QDir saveFileDir = QFileInfo(saveFilePath).dir();
        if (!saveFileDir.exists())
            saveFileDir.mkpath(".");
        QFile file(saveFilePath);
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
        if (subEpisode->isDummy())
            continue;
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
        QString saveFilePath = fi.absolutePath() + "/" + saveFileName;
        QDir saveFileDir = QFileInfo(saveFilePath).dir();
        if (!saveFileDir.exists())
            saveFileDir.mkpath(".");
        QFile file(saveFilePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Nfo file could not be opened for writing" << saveFileName;
            return false;
        }
        file.write(xmlContent);
        file.close();
    }

    fi.setFile(episode->files().at(0));
    if (episode->thumbnailImageChanged() && !episode->thumbnailImage().isNull()) {
        if (Helper::instance()->isBluRay(episode->files().at(0)) || Helper::instance()->isDvd(episode->files().at(0))) {
            QDir dir = fi.dir();
            dir.cdUp();
            saveFile(dir.absolutePath() + "/thumb.jpg", episode->thumbnailImage());
        } else if (Helper::instance()->isDvd(episode->files().at(0), true)) {
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
        if (Helper::instance()->isBluRay(episode->files().at(0)) || Helper::instance()->isDvd(episode->files().at(0))) {
            QDir dir = fi.dir();
            dir.cdUp();
            QFile(dir.absolutePath() + "/thumb.jpg").remove();
        } else if (Helper::instance()->isDvd(episode->files().at(0), true)) {
            QFile(fi.dir().absolutePath() + "/thumb.jpg").remove();
        } else {
            foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowEpisodeThumb)) {
                QString saveFileName = dataFile.saveFileName(fi.fileName(), -1, episode->files().count() > 1);
                QFile(fi.absolutePath() + "/" + saveFileName).remove();
            }
        }
    }

    fi.setFile(episode->files().at(0));
    foreach (const Actor &actor, episode->actors()) {
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

QByteArray XbmcXml::getTvShowXml(TvShow *show)
{
    QDomDocument doc;
    doc.setContent(show->nfoContent());
    if (show->nfoContent().isEmpty()) {
        QDomNode node = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"");
        doc.insertBefore(node, doc.firstChild());
        doc.appendChild(doc.createElement("tvshow"));
    }

    QDomElement showElem = doc.elementsByTagName("tvshow").at(0).toElement();

    setTextValue(doc, "title", show->name());
    setTextValue(doc, "showtitle", show->showTitle());
    if (!show->sortTitle().isEmpty()) {
        QDomElement elem = setTextValue(doc, "sorttitle", show->sortTitle());
        elem.setAttribute("clear", "true");
    } else {
        removeChildNodes(doc, "sorttitle");
    }
    setTextValue(doc, "rating", QString("%1").arg(show->rating()));
    setTextValue(doc, "votes", QString::number(show->votes()));
    setTextValue(doc, "top250", QString::number(show->top250()));
    setTextValue(doc, "episode", QString("%1").arg(show->episodes().count()));
    setTextValue(doc, "plot", show->overview());
    setTextValue(doc, "outline", show->overview());
    setTextValue(doc, "mpaa", QString("%1").arg(show->certification()));
    setTextValue(doc, "premiered", show->firstAired().toString("yyyy-MM-dd"));
    setTextValue(doc, "studio", show->network());
    setTextValue(doc, "tvdbid", show->tvdbId());
    setTextValue(doc, "id", show->id());
    setTextValue(doc, "imdbid", show->imdbId());
    if (show->status().isEmpty())
        setTextValue(doc, "status", show->status());
    else
        removeChildNodes(doc, "status");
    if (show->runtime() > 0)
        setTextValue(doc, "runtime", QString("%1").arg(show->runtime()));
    else if (!showElem.elementsByTagName("runtime").isEmpty())
        showElem.removeChild(showElem.elementsByTagName("runtime").at(0));

    if (!show->episodeGuideUrl().isEmpty()) {
        QDomNodeList childNodes = showElem.childNodes();
        for (int i=0, n=childNodes.count() ; i<n ; ++i) {
            if (childNodes.at(i).nodeName() == "episodeguide") {
                showElem.removeChild(childNodes.at(i));
                break;
            }
        }
        QDomElement elem = doc.createElement("episodeguide");
        QDomElement elemUrl = doc.createElement("url");
        elemUrl.appendChild(doc.createTextNode(show->episodeGuideUrl()));
        elem.appendChild(elemUrl);
        appendXmlNode(doc, elem);
    } else {
        removeChildNodes(doc, "episodeguide");
    }

    setTextValue(doc, "genre", show->genres().join(" / "));
    setListValue(doc, "tag", show->tags());

    removeChildNodes(doc, "actor");

    foreach (const Actor &actor, show->actors()) {
        QDomElement elem = doc.createElement("actor");
        QDomElement elemName = doc.createElement("name");
        QDomElement elemRole = doc.createElement("role");
        elemName.appendChild(doc.createTextNode(actor.name));
        elemRole.appendChild(doc.createTextNode(actor.role));
        elem.appendChild(elemName);
        elem.appendChild(elemRole);
        if (!actor.thumb.isEmpty() && Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
            QDomElement elemThumb = doc.createElement("thumb");
            elemThumb.appendChild(doc.createTextNode(actor.thumb));
            elem.appendChild(elemThumb);
        }
        appendXmlNode(doc, elem);
    }

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        removeChildNodes(doc, "thumb");
        removeChildNodes(doc, "fanart");

        foreach (const Poster &poster, show->posters()) {
            QDomElement elem = doc.createElement("thumb");
            elem.setAttribute("preview", poster.thumbUrl.toString());
            elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            appendXmlNode(doc, elem);

            QDomElement elemSeason = doc.createElement("thumb");
            elemSeason.setAttribute("type", "season");
            elemSeason.setAttribute("season", "-1");
            elemSeason.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            appendXmlNode(doc, elemSeason);
        }

        if (!show->backdrops().isEmpty()) {
            QDomElement fanartElem = doc.createElement("fanart");
            foreach (const Poster &poster, show->backdrops()) {
                QDomElement elem = doc.createElement("thumb");
                elem.setAttribute("preview", poster.thumbUrl.toString());
                elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                fanartElem.appendChild(elem);
            }
            appendXmlNode(doc, fanartElem);
        }

        foreach (int season, show->seasons()) {
            foreach (const Poster &poster, show->seasonPosters(season)) {
                QDomElement elemSeason = doc.createElement("thumb");
                elemSeason.setAttribute("type", "season");
                elemSeason.setAttribute("season", QString("%1").arg(season));
                elemSeason.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                appendXmlNode(doc, elemSeason);
            }
        }
    }

    return doc.toByteArray(4);
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
    xml.writeTextElement("imdbid", episode->imdbId());
    xml.writeTextElement("title", episode->name());
    xml.writeTextElement("showtitle", episode->showTitle());
    xml.writeTextElement("rating", QString("%1").arg(episode->rating()));
    xml.writeTextElement("votes", QString("%1").arg(episode->votes()));
    xml.writeTextElement("top250", QString("%1").arg(episode->top250()));
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
    if (!episode->epBookmark().isNull() && QTime(0, 0, 0).secsTo(episode->epBookmark()) > 0)
        xml.writeTextElement("epbookmark", QString("%1").arg(QTime(0, 0, 0).secsTo(episode->epBookmark())));
    foreach (const QString &writer, episode->writers())
        xml.writeTextElement("credits", writer);
    foreach (const QString &director, episode->directors())
        xml.writeTextElement("director", director);
    if (!episode->thumbnail().isEmpty())
        xml.writeTextElement("thumb", episode->thumbnail().toString());

    foreach (const Actor &actor, episode->actors()) {
        xml.writeStartElement("actor");
        xml.writeTextElement("name", actor.name);
        xml.writeTextElement("role", actor.role);
        if (!actor.thumb.isEmpty() && Settings::instance()->advanced()->writeThumbUrlsToNfo())
            xml.writeTextElement("thumb", actor.thumb);
        xml.writeEndElement();
    }

    XbmcXml::writeStreamDetails(xml, episode->streamDetails());

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

QStringList XbmcXml::extraFanartNames(Artist *artist)
{
    QDir dir(artist->path() + "/extrafanart");
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
    QDir saveFileDir = QFileInfo(filename).dir();
    if (!saveFileDir.exists())
        saveFileDir.mkpath(".");
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

    if (Helper::instance()->isBluRay(episode->files().at(0)) || Helper::instance()->isDvd(episode->files().at(0))) {
        QDir dir = fi.dir();
        dir.cdUp();
        fi.setFile(dir.absolutePath() + "/thumb.jpg");
        return fi.exists() ? fi.absoluteFilePath() : "";
    }

    if (Helper::instance()->isDvd(episode->files().at(0), true)) {
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

bool XbmcXml::loadArtist(Artist *artist, QString initialNfoContent)
{
    artist->clear();
    artist->setHasChanged(false);

    QString nfoContent;
    if (initialNfoContent.isEmpty()) {
        QString nfoFile = nfoFilePath(artist);
        if (nfoFile.isEmpty())
            return false;

        QFile file(nfoFile);
        if (!file.exists())
            return false;
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "File" << nfoFile << "could not be opened for reading";
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
    if (!domDoc.elementsByTagName("musicBrainzArtistID").isEmpty())
        artist->setMbId(domDoc.elementsByTagName("musicBrainzArtistID").at(0).toElement().text());
    if (!domDoc.elementsByTagName("allmusicid").isEmpty())
        artist->setAllMusicId(domDoc.elementsByTagName("allmusicid").at(0).toElement().text());
    if (!domDoc.elementsByTagName("name").isEmpty())
        artist->setName(domDoc.elementsByTagName("name").at(0).toElement().text());
    if (!domDoc.elementsByTagName("genre").isEmpty())
        artist->setGenres(domDoc.elementsByTagName("genre").at(0).toElement().text().split(" / ", QString::SkipEmptyParts));
    for (int i=0, n=domDoc.elementsByTagName("style").size() ; i<n ; i++)
        artist->addStyle(domDoc.elementsByTagName("style").at(i).toElement().text());
    for (int i=0, n=domDoc.elementsByTagName("mood").size() ; i<n ; i++)
        artist->addMood(domDoc.elementsByTagName("mood").at(i).toElement().text());
    if (!domDoc.elementsByTagName("yearsactive").isEmpty())
        artist->setYearsActive(domDoc.elementsByTagName("yearsactive").at(0).toElement().text());
    if (!domDoc.elementsByTagName("formed").isEmpty())
        artist->setFormed(domDoc.elementsByTagName("formed").at(0).toElement().text());
    if (!domDoc.elementsByTagName("biography").isEmpty())
        artist->setBiography(domDoc.elementsByTagName("biography").at(0).toElement().text());
    if (!domDoc.elementsByTagName("born").isEmpty())
        artist->setBorn(domDoc.elementsByTagName("born").at(0).toElement().text());
    if (!domDoc.elementsByTagName("died").isEmpty())
        artist->setDied(domDoc.elementsByTagName("died").at(0).toElement().text());
    if (!domDoc.elementsByTagName("disbanded").isEmpty())
        artist->setDisbanded(domDoc.elementsByTagName("disbanded").at(0).toElement().text());

    for (int i=0, n=domDoc.elementsByTagName("thumb").size() ; i<n ; i++) {
        QString parentTag = domDoc.elementsByTagName("thumb").at(i).parentNode().toElement().tagName();
        if (parentTag == "artist") {
            Poster p;
            p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
            if (!domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview").isEmpty())
                p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
            else
                p.thumbUrl = p.originalUrl;
            artist->addImage(ImageType::ArtistThumb, p);
        } else if (parentTag == "fanart") {
            Poster p;
            p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
            if (!domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview").isEmpty())
                p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
            else
                p.thumbUrl = p.originalUrl;
            artist->addImage(ImageType::ArtistFanart, p);
        }
    }

    for (int i=0, n=domDoc.elementsByTagName("album").size() ; i<n ; i++) {
        DiscographyAlbum a;
        if (!domDoc.elementsByTagName("album").at(i).toElement().elementsByTagName("title").isEmpty())
            a.title = domDoc.elementsByTagName("album").at(i).toElement().elementsByTagName("title").at(0).toElement().text();
        if (!domDoc.elementsByTagName("album").at(i).toElement().elementsByTagName("year").isEmpty())
            a.year = domDoc.elementsByTagName("album").at(i).toElement().elementsByTagName("year").at(0).toElement().text();
        artist->addDiscographyAlbum(a);
    }

    artist->setHasChanged(false);

    return true;
}

bool XbmcXml::loadAlbum(Album *album, QString initialNfoContent)
{
    album->clear();
    album->setHasChanged(false);

    QString nfoContent;
    if (initialNfoContent.isEmpty()) {
        QString nfoFile = nfoFilePath(album);
        if (nfoFile.isEmpty())
            return false;

        QFile file(nfoFile);
        if (!file.exists())
            return false;
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "File" << nfoFile << "could not be opened for reading";
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

    if (!domDoc.elementsByTagName("musicBrainzReleaseGroupID").isEmpty())
        album->setMbReleaseGroupId(domDoc.elementsByTagName("musicBrainzReleaseGroupID").at(0).toElement().text());
    if (!domDoc.elementsByTagName("musicBrainzAlbumID").isEmpty())
        album->setMbAlbumId(domDoc.elementsByTagName("musicBrainzAlbumID").at(0).toElement().text());
    if (!domDoc.elementsByTagName("allmusicid").isEmpty())
        album->setAllMusicId(domDoc.elementsByTagName("allmusicid").at(0).toElement().text());
    if (!domDoc.elementsByTagName("title").isEmpty())
        album->setTitle(domDoc.elementsByTagName("title").at(0).toElement().text());
    if (!domDoc.elementsByTagName("artist").isEmpty())
        album->setArtist(domDoc.elementsByTagName("artist").at(0).toElement().text());
    if (!domDoc.elementsByTagName("genre").isEmpty())
        album->setGenres(domDoc.elementsByTagName("genre").at(0).toElement().text().split(" / ", QString::SkipEmptyParts));
    for (int i=0, n=domDoc.elementsByTagName("style").size() ; i<n ; i++)
        album->addStyle(domDoc.elementsByTagName("style").at(i).toElement().text());
    for (int i=0, n=domDoc.elementsByTagName("mood").size() ; i<n ; i++)
        album->addMood(domDoc.elementsByTagName("mood").at(i).toElement().text());
    if (!domDoc.elementsByTagName("review").isEmpty())
        album->setReview(domDoc.elementsByTagName("review").at(0).toElement().text());
    if (!domDoc.elementsByTagName("label").isEmpty())
        album->setLabel(domDoc.elementsByTagName("label").at(0).toElement().text());
    if (!domDoc.elementsByTagName("releasedate").isEmpty())
        album->setReleaseDate(domDoc.elementsByTagName("releasedate").at(0).toElement().text());
    if (!domDoc.elementsByTagName("year").isEmpty())
        album->setYear(domDoc.elementsByTagName("year").at(0).toElement().text().toInt());
    if (!domDoc.elementsByTagName("rating").isEmpty())
        album->setRating(domDoc.elementsByTagName("rating").at(0).toElement().text().replace(",", ".").toFloat());
    for (int i=0, n=domDoc.elementsByTagName("thumb").size() ; i<n ; i++) {
        Poster p;
        p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
        if (!domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview").isEmpty())
            p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
        else
            p.thumbUrl = p.originalUrl;
        album->addImage(ImageType::AlbumThumb, p);
    }

    album->setHasChanged(false);

    return true;
}

QString XbmcXml::imageFileName(Artist *artist, int type, QList<DataFile> dataFiles, bool constructName)
{
    int fileType;
    switch (type) {
    case ImageType::ArtistThumb:
        fileType = DataFileType::ArtistThumb;
        break;
    case ImageType::ArtistFanart:
        fileType = DataFileType::ArtistFanart;
        break;
    case ImageType::ArtistLogo:
        fileType = DataFileType::ArtistLogo;
        break;
    default:
        return "";
    }

    QString fileName;
    if (artist->path().isEmpty())
        return QString();

    if (!constructName)
        dataFiles = Settings::instance()->dataFiles(fileType);

    foreach (DataFile dataFile, dataFiles) {
        QString file = dataFile.saveFileName(QString());
        QFileInfo pFi(artist->path() + "/" + file);
        if (pFi.isFile() || constructName) {
            fileName = artist->path() + "/" + file;
            break;
        }
    }

    return fileName;
}

QString XbmcXml::imageFileName(Album *album, int type, QList<DataFile> dataFiles, bool constructName)
{
    int fileType;
    switch (type) {
    case ImageType::AlbumThumb:
        fileType = DataFileType::AlbumThumb;
        break;
    case ImageType::AlbumCdArt:
        fileType = DataFileType::AlbumCdArt;
        break;
    default:
        return "";
    }

    QString fileName;
    if (album->path().isEmpty())
        return QString();

    if (!constructName)
        dataFiles = Settings::instance()->dataFiles(fileType);

    foreach (DataFile dataFile, dataFiles) {
        QString file = dataFile.saveFileName(QString());
        QFileInfo pFi(album->path() + "/" + file);
        if (pFi.isFile() || constructName) {
            fileName = album->path() + "/" + file;
            break;
        }
    }

    return fileName;
}

QString XbmcXml::nfoFilePath(Artist *artist)
{
    if (artist->path().isEmpty())
        return QString();

    return artist->path() + "/artist.nfo";
}

QString XbmcXml::nfoFilePath(Album *album)
{
    if (album->path().isEmpty())
        return QString();

    return album->path() + "/album.nfo";
}

bool XbmcXml::saveArtist(Artist *artist)
{
    QByteArray xmlContent = getArtistXml(artist);

    if (artist->path().isEmpty())
        return false;

    artist->setNfoContent(xmlContent);
    Manager::instance()->database()->update(artist);

    QString fileName = nfoFilePath(artist);
    if (fileName.isEmpty())
        return false;

    QDir saveFileDir = QFileInfo(fileName).dir();
    if (!saveFileDir.exists())
        saveFileDir.mkpath(".");
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "File could not be openend";
        return false;
    }
    file.write(xmlContent);
    file.close();

    foreach (const int &imageType, Artist::imageTypes()) {
        int dataFileType = DataFile::dataFileTypeForImageType(imageType);

        if (artist->imagesToRemove().contains(imageType)) {
            foreach (DataFile dataFile, Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName = dataFile.saveFileName(QString());
                if (!saveFileName.isEmpty())
                    QFile(artist->path() + "/" + saveFileName).remove();
            }
        }

        if (!artist->rawImage(imageType).isNull()) {
            foreach (DataFile dataFile, Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName = dataFile.saveFileName(QString());
                saveFile(artist->path() + "/" + saveFileName, artist->rawImage(imageType));
            }
        }
    }

    foreach (const QString &file, artist->extraFanartsToRemove())
        QFile::remove(file);
    QDir dir(artist->path() + "/extrafanart");
    if (!dir.exists() && !artist->extraFanartImagesToAdd().isEmpty())
        QDir(artist->path()).mkdir("extrafanart");
    foreach (QByteArray img, artist->extraFanartImagesToAdd()) {
        int num = 1;
        while (QFileInfo(dir.absolutePath() + "/" + QString("fanart%1.jpg").arg(num)).exists())
            ++num;
        saveFile(dir.absolutePath() + "/" + QString("fanart%1.jpg").arg(num), img);
    }

    return true;
}

bool XbmcXml::saveAlbum(Album *album)
{
    QByteArray xmlContent = getAlbumXml(album);

    if (album->path().isEmpty())
        return false;

    album->setNfoContent(xmlContent);
    Manager::instance()->database()->update(album);

    QString fileName = nfoFilePath(album);
    if (fileName.isEmpty())
        return false;

    QDir saveFileDir = QFileInfo(fileName).dir();
    if (!saveFileDir.exists())
        saveFileDir.mkpath(".");
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "File could not be openend";
        return false;
    }
    file.write(xmlContent);
    file.close();

    foreach (const int &imageType, Album::imageTypes()) {
        int dataFileType = DataFile::dataFileTypeForImageType(imageType);

        if (album->imagesToRemove().contains(imageType)) {
            foreach (DataFile dataFile, Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName = dataFile.saveFileName(QString());
                if (!saveFileName.isEmpty())
                    QFile(album->path() + "/" + saveFileName).remove();
            }
        }

        if (!album->rawImage(imageType).isNull()) {
            foreach (DataFile dataFile, Settings::instance()->dataFiles(dataFileType)) {
                QString saveFileName = dataFile.saveFileName(QString());
                saveFile(album->path() + "/" + saveFileName, album->rawImage(imageType));
            }
        }
    }

    if (album->bookletModel()->hasChanged()) {
        QDir dir(album->path() + "/booklet");
        if (!dir.exists())
            QDir(album->path()).mkdir("booklet");

        // @todo: get filename from settings
        foreach (Image *image, album->bookletModel()->images()) {
            if (image->deletion() && !image->fileName().isEmpty())
                QFile::remove(image->fileName());
            else if (!image->deletion())
                image->load();
        }
        int bookletNum = 1;
        foreach (Image *image, album->bookletModel()->images()) {
            if (!image->deletion()) {
                QString fileName = album->path() + "/booklet/booklet" + QString("%1").arg(bookletNum, 2, 10, QChar('0'))  + ".jpg";
                QFile file(fileName);
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

QByteArray XbmcXml::getArtistXml(Artist *artist)
{
    QDomDocument doc;
    doc.setContent(artist->nfoContent());
    if (artist->nfoContent().isEmpty()) {
        QDomNode node = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"");
        doc.insertBefore(node, doc.firstChild());
        doc.appendChild(doc.createElement("artist"));
    }

    QDomElement artistElem = doc.elementsByTagName("artist").at(0).toElement();

    if (!artist->mbId().isEmpty())
        setTextValue(doc, "musicBrainzArtistID", artist->mbId());
    else
        removeChildNodes(doc, "musicBrainzArtistID");
    if (!artist->allMusicId().isEmpty())
        setTextValue(doc, "allmusicid", artist->allMusicId());
    else
        removeChildNodes(doc, "allmusicid");
    setTextValue(doc, "name", artist->name());
    setTextValue(doc, "genre", artist->genres().join(" / "));
    setListValue(doc, "style", artist->styles());
    setListValue(doc, "mood", artist->moods());
    setTextValue(doc, "yearsactive", artist->yearsActive());
    setTextValue(doc, "formed", artist->formed());
    setTextValue(doc, "biography", artist->biography());
    setTextValue(doc, "born", artist->born());
    setTextValue(doc, "died", artist->died());
    setTextValue(doc, "disbanded", artist->disbanded());

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        removeChildNodes(doc, "thumb");
        removeChildNodes(doc, "fanart");

        foreach (const Poster &poster, artist->images(ImageType::ArtistThumb)) {
            QDomElement elem = doc.createElement("thumb");
            elem.setAttribute("preview", poster.thumbUrl.toString());
            elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            appendXmlNode(doc, elem);
        }

        if (!artist->images(ImageType::ArtistFanart).isEmpty()) {
            QDomElement fanartElem = doc.createElement("fanart");
            foreach (const Poster &poster, artist->images(ImageType::ArtistFanart)) {
                QDomElement elem = doc.createElement("thumb");
                elem.setAttribute("preview", poster.thumbUrl.toString());
                elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                fanartElem.appendChild(elem);
            }
            appendXmlNode(doc, fanartElem);
        }
    }

    QList<QDomNode> albumNodes;
    QDomNodeList childNodes = artistElem.childNodes();
    for (int i=0, n=childNodes.count() ; i<n ; ++i) {
        if (childNodes.at(i).nodeName() == "album")
            albumNodes.append(childNodes.at(i));
    }

    foreach (const DiscographyAlbum &album, artist->discographyAlbums()) {
        bool nodeFound = false;
        foreach (QDomNode node, albumNodes) {
            if (!node.toElement().elementsByTagName("title").isEmpty() && node.toElement().elementsByTagName("title").at(0).toElement().text() == album.title) {
                albumNodes.removeOne(node);
                if (!node.toElement().elementsByTagName("year").isEmpty()) {
                    if (!node.toElement().elementsByTagName("year").at(0).firstChild().isText()) {
                        QDomText t = doc.createTextNode(album.year);
                        node.toElement().elementsByTagName("year").at(0).appendChild(t);
                    } else {
                        node.toElement().elementsByTagName("year").at(0).firstChild().setNodeValue(album.year);
                    }
                } else {
                    QDomElement elem = doc.createElement("year");
                    elem.appendChild(doc.createTextNode(album.year));
                    node.appendChild(elem);
                }
                nodeFound = true;
                break;
            }
        }
        if (!nodeFound) {
            QDomElement elem = doc.createElement("album");
            QDomElement elemTitle = doc.createElement("title");
            QDomElement elemYear = doc.createElement("year");
            elemTitle.appendChild(doc.createTextNode(album.title));
            elemYear.appendChild(doc.createTextNode(album.year));
            elem.appendChild(elemTitle);
            elem.appendChild(elemYear);
            appendXmlNode(doc, elem);
        }
    }
    foreach (QDomNode node, albumNodes)
        artistElem.removeChild(node);

    return doc.toByteArray(4);
}

QByteArray XbmcXml::getAlbumXml(Album *album)
{
    QDomDocument doc;
    doc.setContent(album->nfoContent());
    if (album->nfoContent().isEmpty()) {
        QDomNode node = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"");
        doc.insertBefore(node, doc.firstChild());
        doc.appendChild(doc.createElement("album"));
    }

    QDomElement albumElem = doc.elementsByTagName("album").at(0).toElement();

    if (!album->mbReleaseGroupId().isEmpty())
        setTextValue(doc, "musicBrainzReleaseGroupID", album->mbReleaseGroupId());
    else
        removeChildNodes(doc, "musicBrainzReleaseGroupID");
    if (!album->mbAlbumId().isEmpty())
        setTextValue(doc, "musicBrainzAlbumID", album->mbAlbumId());
    else
        removeChildNodes(doc, "musicBrainzAlbumID");
    if (!album->allMusicId().isEmpty())
        setTextValue(doc, "allmusicid", album->allMusicId());
    else
        removeChildNodes(doc, "allmusicid");
    setTextValue(doc, "title", album->title());
    setTextValue(doc, "artist", album->artist());
    setTextValue(doc, "genre", album->genres().join(" / "));
    setListValue(doc, "style", album->styles());
    setListValue(doc, "mood", album->moods());
    setTextValue(doc, "review", album->review());
    setTextValue(doc, "label", album->label());
    setTextValue(doc, "releasedate", album->releaseDate());
    if (album->rating() > 0)
        setTextValue(doc, "rating", QString("%1").arg(album->rating()));
    else
        removeChildNodes(doc, "rating");
    if (album->year() > 0)
        setTextValue(doc, "year", QString("%1").arg(album->year()));
    else
        removeChildNodes(doc, "year");

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        removeChildNodes(doc, "thumb");

        foreach (const Poster &poster, album->images(ImageType::AlbumThumb)) {
            QDomElement elem = doc.createElement("thumb");
            elem.setAttribute("preview", poster.thumbUrl.toString());
            elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            appendXmlNode(doc, elem);
        }
    }

    return doc.toByteArray(4);
}

QDomElement XbmcXml::setTextValue(QDomDocument &doc, const QString &name, const QString &value)
{
    if (!doc.elementsByTagName(name).isEmpty()) {
        if (!doc.elementsByTagName(name).at(0).firstChild().isText()) {
            QDomText t = doc.createTextNode(value);
            doc.elementsByTagName(name).at(0).appendChild(t);
            return doc.elementsByTagName(name).at(0).toElement();
        } else {
            doc.elementsByTagName(name).at(0).firstChild().setNodeValue(value);
            return doc.elementsByTagName(name).at(0).toElement();
        }
    } else {
        return addTextValue(doc, name, value);
    }
}

void XbmcXml::setListValue(QDomDocument &doc, const QString &name, const QStringList &values)
{
    QDomNode rootNode = doc.firstChild();
    while ((rootNode.nodeName() == "xml" || rootNode.isComment()) && !rootNode.isNull())
        rootNode = rootNode.nextSibling();
    QDomNodeList childNodes = rootNode.childNodes();
    QList<QDomNode> nodesToRemove;
    for (int i=0, n=childNodes.count() ; i<n ; ++i) {
        if (childNodes.at(i).nodeName() == name)
            nodesToRemove.append(childNodes.at(i));
    }
    foreach (QDomNode node, nodesToRemove)
        rootNode.removeChild(node);
    foreach (const QString &style, values)
        addTextValue(doc, name, style);
}

QDomElement XbmcXml::addTextValue(QDomDocument &doc, const QString &name, const QString &value)
{
    QDomElement elem = doc.createElement(name);
    elem.appendChild(doc.createTextNode(value));
    appendXmlNode(doc, elem);
    return elem;
}

void XbmcXml::appendXmlNode(QDomDocument &doc, QDomNode &node)
{
    QDomNode rootNode = doc.firstChild();
    while ((rootNode.nodeName() == "xml" || rootNode.isComment()) && !rootNode.isNull())
        rootNode = rootNode.nextSibling();
    rootNode.appendChild(node);
}

void XbmcXml::removeChildNodes(QDomDocument &doc, const QString &name)
{
    QDomNode rootNode = doc.firstChild();
    while ((rootNode.nodeName() == "xml" || rootNode.isComment()) && !rootNode.isNull())
        rootNode = rootNode.nextSibling();
    QDomNodeList childNodes = rootNode.childNodes();
    QList<QDomNode> nodesToRemove;
    for (int i=0, n=childNodes.count() ; i<n ; ++i) {
        if (childNodes.at(i).nodeName() == name)
            nodesToRemove.append(childNodes.at(i));
    }
    foreach (QDomNode node, nodesToRemove)
        rootNode.removeChild(node);
}

void XbmcXml::loadBooklets(Album *album)
{
    // @todo: get filename from settings
    if (!album->bookletModel()->images().isEmpty())
        return;

    QDir dir(album->path() + "/booklet");
    QStringList filters = QStringList() << "*.jpg" << "*.jpeg" << "*.JPEG" << "*.Jpeg" << "*.JPeg";
    foreach (const QString &file, dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name)) {
        Image *img = new Image;
        img->setFileName(QDir::toNativeSeparators(dir.path() + "/" + file));
        album->bookletModel()->addImage(img);
    }
    album->bookletModel()->setHasChanged(false);
}
