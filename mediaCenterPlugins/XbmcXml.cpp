#include "XbmcXml.h"

#include <QApplication>
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
    if (feature == MediaCenterFeatures::HandleMovieSetImages)
        return false;

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
    xml.writeTextElement("lastplayed", movie->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    xml.writeTextElement("id", movie->id());
    xml.writeTextElement("tmdbid", movie->tmdbId());
    xml.writeTextElement("set", movie->set());
    xml.writeTextElement("sorttitle", movie->sortTitle());
    xml.writeTextElement("trailer", Helper::formatTrailerUrl(movie->trailer().toString()));
    xml.writeTextElement("watched", (movie->watched()) ? "true" : "false");
    foreach (const QString &studio, movie->studios())
        xml.writeTextElement("studio", studio);
    foreach (const QString &genre, movie->genres())
        xml.writeTextElement("genre", genre);
    foreach (const QString &country, movie->countries())
        xml.writeTextElement("country", country);
    foreach (const Actor &actor, movie->actors()) {
        xml.writeStartElement("actor");
        xml.writeTextElement("name", actor.name);
        xml.writeTextElement("role", actor.role);
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

    xml.writeStartElement("fileinfo");
    writeStreamDetails(xml, movie->streamDetails());
    xml.writeEndElement();

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
        QString saveFileName = dataFile.saveFileName(fi.fileName());
        QFile file(fi.absolutePath() + QDir::separator() + saveFileName);
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

    if (movie->posterImageChanged() && !movie->posterImage()->isNull()) {
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MoviePoster)) {
            QString saveFileName = dataFile.saveFileName(fi.fileName());
            qDebug() << "Saving poster to" << fi.absolutePath() + QDir::separator() + saveFileName;
            movie->posterImage()->save(fi.absolutePath() + QDir::separator() + saveFileName, "jpg", 100);
        }
    }
    if (movie->backdropImageChanged() && !movie->backdropImage()->isNull()) {
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MovieBackdrop)) {
            QString saveFileName = dataFile.saveFileName(fi.fileName());
            qDebug() << "Saving fanart to" << fi.absolutePath() + QDir::separator() + saveFileName;
            movie->backdropImage()->save(fi.absolutePath() + QDir::separator() + saveFileName, "jpg", 100);
        }
    }
    saveAdditionalImages(movie);

    foreach (const Actor &actor, movie->actors()) {
        if (!actor.image.isNull()) {
            QDir dir;
            dir.mkdir(fi.absolutePath() + QDir::separator() + ".actors");
            QString actorName = actor.name;
            actorName = actorName.replace(" ", "_");
            actor.image.save(fi.absolutePath() + QDir::separator() + ".actors" + QDir::separator() + actorName + ".tbn", "jpg", 100);
        }
    }

    return true;
}

/**
 * @brief Saves additional movie images (logo, clear art, cd art)
 * @param movie Movie object
 */
void XbmcXml::saveAdditionalImages(Movie *movie)
{
    QFileInfo fi(movie->files().at(0));
    if (movie->logoImageChanged() && !movie->logoImage()->isNull()) {
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MovieLogo)) {
            QString saveFileName = dataFile.saveFileName(fi.fileName());
            qDebug() << "Saving logo to" << fi.absolutePath() + QDir::separator() + saveFileName;
            movie->logoImage()->save(fi.absolutePath() + QDir::separator() + saveFileName, "png", 100);
        }
    }
    if (movie->clearArtImageChanged() && !movie->clearArtImage()->isNull()) {
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MovieClearArt)) {
            QString saveFileName = dataFile.saveFileName(fi.fileName());
            qDebug() << "Saving clear art to" << fi.absolutePath() + QDir::separator() + saveFileName;
            movie->clearArtImage()->save(fi.absolutePath() + QDir::separator() + saveFileName, "png", 100);
        }
    }
    if (movie->cdArtImageChanged() && !movie->cdArtImage()->isNull()) {
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MovieCdArt)) {
            QString saveFileName = dataFile.saveFileName(fi.fileName());
            qDebug() << "Saving cd art to" << fi.absolutePath() + QDir::separator() + saveFileName;
            movie->cdArtImage()->save(fi.absolutePath() + QDir::separator() + saveFileName, "png", 100);
        }
    }
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
        QString file = dataFile.saveFileName(fi.fileName());
        QFileInfo nfoFi(fi.absolutePath() + QDir::separator() + file);
        if (nfoFi.exists()) {
            nfoFile = fi.absolutePath() + QDir::separator() + file;
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
        QString file = dataFile.saveFileName(fi.fileName());
        QFileInfo nfoFi(fi.absolutePath() + QDir::separator() + file);
        if (nfoFi.exists()) {
            nfoFile = fi.absolutePath() + QDir::separator() + file;
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
        movie->setVotes(domDoc.elementsByTagName("votes").at(0).toElement().text().toInt());
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
    if (!domDoc.elementsByTagName("lastplayed").isEmpty())
        movie->setLastPlayed(QDateTime::fromString(domDoc.elementsByTagName("lastplayed").at(0).toElement().text(), "yyyy-MM-dd HH:mm:ss"));
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
    if (!domDoc.elementsByTagName("watched").isEmpty())
        movie->setWatched(domDoc.elementsByTagName("watched").at(0).toElement().text() == "true" ? true : false);

    for (int i=0, n=domDoc.elementsByTagName("studio").size() ; i<n ; i++)
        movie->addStudio(domDoc.elementsByTagName("studio").at(i).toElement().text());
    for (int i=0, n=domDoc.elementsByTagName("genre").size() ; i<n ; i++)
        movie->addGenre(domDoc.elementsByTagName("genre").at(i).toElement().text());
    for (int i=0, n=domDoc.elementsByTagName("country").size() ; i<n ; i++)
        movie->addCountry(domDoc.elementsByTagName("country").at(i).toElement().text());
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
        movie->setHasPoster(!posterImageName(movie).isEmpty());
        movie->setHasBackdrop(!backdropImageName(movie).isEmpty());
        movie->setHasLogo(!logoImageName(movie).isEmpty());
        movie->setHasClearArt(!clearArtImageName(movie).isEmpty());
        movie->setHasCdArt(!cdArtImageName(movie).isEmpty());
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
        return true;
    }
    return false;
}

/**
 * @brief Writes streamdetails to xml stream
 * @param xml XML Stream
 * @param streamDetails Stream Details object
 */
void XbmcXml::writeStreamDetails(QXmlStreamWriter &xml, StreamDetails *streamDetails)
{
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
    QString path = fi.absolutePath() + QDir::separator() + ".actors" + QDir::separator() + actorName + ".tbn";
    fi.setFile(path);
    if (fi.isFile())
        return path;
    return QString();
}

/**
 * @brief Get the path to the movie poster
 * @param movie Movie object
 * @return Path to poster image
 */
QString XbmcXml::posterImageName(Movie *movie)
{
    QString posterFileName;
    if (movie->files().size() == 0) {
        qWarning() << "Movie has no files";
        return posterFileName;
    }
    QFileInfo fi(movie->files().at(0));

    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MoviePoster)) {
        QString file = dataFile.saveFileName(fi.fileName());
        QFileInfo pFi(fi.absolutePath() + QDir::separator() + file);
        if (pFi.isFile()) {
            posterFileName = fi.absolutePath() + QDir::separator() + file;
            break;
        }
    }

    return posterFileName;
}

/**
 * @brief Get the path to the movie backdrop
 * @param movie Movie object
 * @return Path to backdrop image
 */
QString XbmcXml::backdropImageName(Movie *movie)
{
    QString fanartFileName;
    if (movie->files().size() == 0) {
        qWarning() << "Movie has no files";
        return fanartFileName;
    }
    QFileInfo fi(movie->files().at(0));

    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MovieBackdrop)) {
        QString file = dataFile.saveFileName(fi.fileName());
        QFileInfo bFi(fi.absolutePath() + QDir::separator() + file);
        if (bFi.isFile()) {
            fanartFileName = fi.absolutePath() + QDir::separator() + file;
            break;
        }
    }

    return fanartFileName;
}

/**
 * @brief Get the path to the movie logo
 * @param movie Movie object
 * @return Path to logo image
 */
QString XbmcXml::logoImageName(Movie *movie)
{
    return XbmcXml::logoImageNameStatic(movie);
}

/**
 * @brief Get the path to the movie clear art
 * @param movie Movie object
 * @return Path to clear art image
 */
QString XbmcXml::clearArtImageName(Movie *movie)
{
    return XbmcXml::clearArtImageNameStatic(movie);
}

/**
 * @brief Get the path to the movie cd art
 * @param movie Movie object
 * @return Path to cd art image
 */
QString XbmcXml::cdArtImageName(Movie *movie)
{
    return XbmcXml::cdArtImageNameStatic(movie);
}

/**
 * @brief Get the path to the movie logo
 * @param movie Movie object
 * @return Path to logo image
 */
QString XbmcXml::logoImageNameStatic(Movie *movie)
{
    QString logoFileName;
    if (movie->files().size() == 0) {
        qWarning() << "Movie has no files";
        return logoFileName;
    }
    QFileInfo fi(movie->files().at(0));

    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MovieLogo)) {
        QString file = dataFile.saveFileName(fi.fileName());
        QFileInfo bFi(fi.absolutePath() + QDir::separator() + file);
        if (bFi.isFile()) {
            logoFileName = fi.absolutePath() + QDir::separator() + file;
            break;
        }
    }

    return logoFileName;
}

/**
 * @brief Get the path to the movie clear art
 * @param movie Movie object
 * @return Path to clear art image
 */
QString XbmcXml::clearArtImageNameStatic(Movie *movie)
{
    QString clearArtFileName;
    if (movie->files().size() == 0) {
        qWarning() << "Movie has no files";
        return clearArtFileName;
    }
    QFileInfo fi(movie->files().at(0));

    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MovieClearArt)) {
        QString file = dataFile.saveFileName(fi.fileName());
        QFileInfo bFi(fi.absolutePath() + QDir::separator() + file);
        if (bFi.isFile()) {
            clearArtFileName = fi.absolutePath() + QDir::separator() + file;
            break;
        }
    }

    return clearArtFileName;
}

/**
 * @brief Get the path to the movie cd art
 * @param movie Movie object
 * @return Path to cd art image
 */
QString XbmcXml::cdArtImageNameStatic(Movie *movie)
{
    QString cdArtFileName;
    if (movie->files().size() == 0) {
        qWarning() << "Movie has no files";
        return cdArtFileName;
    }
    QFileInfo fi(movie->files().at(0));

    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::MovieCdArt)) {
        QString file = dataFile.saveFileName(fi.fileName());
        QFileInfo bFi(fi.absolutePath() + QDir::separator() + file);
        if (bFi.isFile()) {
            cdArtFileName = fi.absolutePath() + QDir::separator() + file;
            break;
        }
    }

    return cdArtFileName;
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

    xml.writeStartElement("fileinfo");
    writeStreamDetails(xml, concert->streamDetails());
    xml.writeEndElement();

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
    qDebug() << "Entered, movie=" << concert->name();
    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);
    writeConcertXml(xml, concert);
    xml.writeEndDocument();

    if (concert->files().size() == 0) {
        qWarning() << "Movie has no files";
        return false;
    }

    concert->setNfoContent(xmlContent);
    Manager::instance()->database()->update(concert);

    bool saved = false;
    QFileInfo fi(concert->files().at(0));
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::ConcertNfo)) {
        QString saveFileName = dataFile.saveFileName(fi.fileName());
        QFile file(fi.absolutePath() + QDir::separator() + saveFileName);
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

    if (concert->posterImageChanged() && !concert->posterImage()->isNull()) {
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::ConcertPoster)) {
            QString saveFileName = dataFile.saveFileName(fi.fileName());
            qDebug() << "Saving poster to" << fi.absolutePath() + QDir::separator() + saveFileName;
            concert->posterImage()->save(fi.absolutePath() + QDir::separator() + saveFileName, "jpg", 100);
        }
    }
    if (concert->backdropImageChanged() && !concert->backdropImage()->isNull()) {
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::ConcertBackdrop)) {
            QString saveFileName = dataFile.saveFileName(fi.fileName());
            qDebug() << "Saving fanart to" << fi.absolutePath() + QDir::separator() + saveFileName;
            concert->backdropImage()->save(fi.absolutePath() + QDir::separator() + saveFileName, "jpg", 100);
        }
    }
    saveAdditionalImages(concert);

    return true;
}

/**
 * @brief Saves additional concert images (logo, clear art, cd art)
 * @param concert Concert object
 */
void XbmcXml::saveAdditionalImages(Concert *concert)
{
    QFileInfo fi(concert->files().at(0));
    if (concert->logoImageChanged() && !concert->logoImage()->isNull()) {
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::ConcertLogo)) {
            QString saveFileName = dataFile.saveFileName(fi.fileName());
            qDebug() << "Saving logo to" << fi.absolutePath() + QDir::separator() + saveFileName;
            concert->logoImage()->save(fi.absolutePath() + QDir::separator() + saveFileName, "png", 100);
        }
    }
    if (concert->clearArtImageChanged() && !concert->clearArtImage()->isNull()) {
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::ConcertClearArt)) {
            QString saveFileName = dataFile.saveFileName(fi.fileName());
            qDebug() << "Saving clear art to" << fi.absolutePath() + QDir::separator() + saveFileName;
            concert->clearArtImage()->save(fi.absolutePath() + QDir::separator() + saveFileName, "png", 100);
        }
    }
    if (concert->cdArtImageChanged() && !concert->cdArtImage()->isNull()) {
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::ConcertCdArt)) {
            QString saveFileName = dataFile.saveFileName(fi.fileName());
            qDebug() << "Saving cd art to" << fi.absolutePath() + QDir::separator() + saveFileName;
            concert->cdArtImage()->save(fi.absolutePath() + QDir::separator() + saveFileName, "png", 100);
        }
    }
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
 * @brief Get the path to the concert poster
 * @param concert Concert object
 * @return Path to poster image
 */
QString XbmcXml::posterImageName(Concert *concert)
{
    if (concert->files().count() == 0)
        return QString();

    QFileInfo fi(concert->files().at(0));
    QString posterFileName;
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::ConcertPoster)) {
        QString file = dataFile.saveFileName(fi.fileName());
        QFileInfo pFi(fi.absolutePath() + QDir::separator() + file);
        if (pFi.isFile()) {
            posterFileName = fi.absolutePath() + QDir::separator() + file;
            break;
        }
    }
    fi.setFile(posterFileName);
    if (fi.isFile())
        return posterFileName;
    return QString();
}

/**
 * @brief Get the path to the concert backdrop
 * @param concert Concert object
 * @return Path to backdrop image
 */
QString XbmcXml::backdropImageName(Concert *concert)
{
    if (concert->files().count() == 0)
        return QString();

    QFileInfo fi(concert->files().at(0));
    QString fanartFileName;
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::ConcertBackdrop)) {
        QString file = dataFile.saveFileName(fi.fileName());
        QFileInfo bFi(fi.absolutePath() + QDir::separator() + file);
        if (bFi.isFile()) {
            fanartFileName = fi.absolutePath() + QDir::separator() + file;
            break;
        }
    }
    fi.setFile(fanartFileName);
    if (fi.isFile())
        return fanartFileName;
    return QString();
}

/**
 * @brief Get the path to the concert logo
 * @param concert Concert object
 * @return Path to logo image
 */
QString XbmcXml::logoImageName(Concert *concert)
{
    return XbmcXml::logoImageNameStatic(concert);
}

/**
 * @brief Get the path to the concert clear art
 * @param concert Concert object
 * @return Path to clear art image
 */
QString XbmcXml::clearArtImageName(Concert *concert)
{
    return XbmcXml::clearArtImageNameStatic(concert);
}

/**
 * @brief Get the path to the concert cd art
 * @param concert Concert object
 * @return Path to cd art image
 */
QString XbmcXml::cdArtImageName(Concert *concert)
{
    return XbmcXml::cdArtImageNameStatic(concert);
}

/**
 * @brief Get the path to the concert logo
 * @param concert Concert object
 * @return Path to logo image
 */
QString XbmcXml::logoImageNameStatic(Concert *concert)
{
    QString logoFileName;
    if (concert->files().size() == 0) {
        qWarning() << "Concert has no files";
        return logoFileName;
    }
    QFileInfo fi(concert->files().at(0));

    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::ConcertLogo)) {
        QString file = dataFile.saveFileName(fi.fileName());
        QFileInfo bFi(fi.absolutePath() + QDir::separator() + file);
        if (bFi.isFile()) {
            logoFileName = fi.absolutePath() + QDir::separator() + file;
            break;
        }
    }

    return logoFileName;
}

/**
 * @brief Get the path to the concert clear art
 * @param concert Concert object
 * @return Path to clear art image
 */
QString XbmcXml::clearArtImageNameStatic(Concert *concert)
{
    QString clearArtFileName;
    if (concert->files().size() == 0) {
        qWarning() << "Concert has no files";
        return clearArtFileName;
    }
    QFileInfo fi(concert->files().at(0));

    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::ConcertClearArt)) {
        QString file = dataFile.saveFileName(fi.fileName());
        QFileInfo bFi(fi.absolutePath() + QDir::separator() + file);
        if (bFi.isFile()) {
            clearArtFileName = fi.absolutePath() + QDir::separator() + file;
            break;
        }
    }

    return clearArtFileName;
}

/**
 * @brief Get the path to the concert cd art
 * @param concert Concert object
 * @return Path to cd art image
 */
QString XbmcXml::cdArtImageNameStatic(Concert *concert)
{
    QString cdArtFileName;
    if (concert->files().size() == 0) {
        qWarning() << "Concert has no files";
        return cdArtFileName;
    }
    QFileInfo fi(concert->files().at(0));

    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::ConcertClearArt)) {
        QString file = dataFile.saveFileName(fi.fileName());
        QFileInfo bFi(fi.absolutePath() + QDir::separator() + file);
        if (bFi.isFile()) {
            cdArtFileName = fi.absolutePath() + QDir::separator() + file;
            break;
        }
    }

    return cdArtFileName;
}

/**
 * @brief Get path to poster image
 * @param show
 * @return
 */
QString XbmcXml::posterImageName(TvShow *show)
{
    if (show->dir().isEmpty())
        return QString();

    QString posterFileName;
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowPoster)) {
        QString loadFileName = dataFile.saveFileName("");
        QFileInfo posterFi(show->dir() + QDir::separator() + loadFileName);
        if (posterFi.isFile()) {
            posterFileName = show->dir() + QDir::separator() + loadFileName;
            break;
        }
    }
    QFileInfo fi(posterFileName);
    if (fi.isFile())
        return posterFileName;
    return QString();
}

/**
 * @brief Gets path to backdrop image
 * @param show
 * @return
 */
QString XbmcXml::backdropImageName(TvShow *show)
{
    if (show->dir().isEmpty())
        return QString();

    QString posterFileName;
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowBackdrop)) {
        QString loadFileName = dataFile.saveFileName("");
        QFileInfo posterFi(show->dir() + QDir::separator() + loadFileName);
        if (posterFi.isFile()) {
            posterFileName = show->dir() + QDir::separator() + loadFileName;
            break;
        }
    }
    QFileInfo fi(posterFileName);
    if (fi.isFile())
        return posterFileName;
    return QString();
}

/**
 * @brief Get path to banner image
 * @param show
 * @return
 */
QString XbmcXml::bannerImageName(TvShow *show)
{
    if (show->dir().isEmpty())
        return QString();

    QString bannerFileName;
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowBanner)) {
        QString loadFileName = dataFile.saveFileName("");
        QFileInfo bannerFi(show->dir() + QDir::separator() + loadFileName);
        if (bannerFi.isFile()) {
            bannerFileName = show->dir() + QDir::separator() + loadFileName;
            break;
        }
    }
    QFileInfo fi(bannerFileName);
    if (fi.isFile())
        return bannerFileName;
    return QString();
}

/**
 * @brief Get the path to the tv show logo
 * @param show TV Show object
 * @return Path to logo image
 */
QString XbmcXml::logoImageNameStatic(TvShow *show)
{
    QString logoFileName;
    if (show->dir().isEmpty())
        return logoFileName;
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowLogo)) {
        QString file = dataFile.saveFileName("");
        QFileInfo fi(show->dir() + QDir::separator() + file);
        if (fi.isFile()) {
            logoFileName = show->dir() + QDir::separator() + file;
            break;
        }
    }

    return logoFileName;
}

/**
 * @brief Get the path to the tv show clear art
 * @param show TV Show object
 * @return Path to clear art image
 */
QString XbmcXml::clearArtImageNameStatic(TvShow *show)
{
    QString clearArtFileName;
    if (show->dir().isEmpty())
        return clearArtFileName;
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowClearArt)) {
        QString file = dataFile.saveFileName("");
        QFileInfo fi(show->dir() + QDir::separator() + file);
        if (fi.isFile()) {
            clearArtFileName = show->dir() + QDir::separator() + file;
            break;
        }
    }

    return clearArtFileName;
}

/**
 * @brief Get the path to the tv show character art
 * @param show TV Show object
 * @return Path to character art image
 */
QString XbmcXml::characterArtImageNameStatic(TvShow *show)
{
    QString characterArtFileName;
    if (show->dir().isEmpty())
        return characterArtFileName;
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowCharacterArt)) {
        QString file = dataFile.saveFileName("");
        QFileInfo fi(show->dir() + QDir::separator() + file);
        if (fi.isFile()) {
            characterArtFileName = show->dir() + QDir::separator() + file;
            break;
        }
    }

    return characterArtFileName;
}

/**
 * @brief Get the path to the tv show logo
 * @param show TV Show object
 * @return Path to logo image
 */
QString XbmcXml::logoImageName(TvShow *show)
{
    return logoImageNameStatic(show);
}

/**
 * @brief Get the path to the tv show clear art
 * @param show TV Show object
 * @return Path to clear art image
 */
QString XbmcXml::clearArtImageName(TvShow *show)
{
    return clearArtImageNameStatic(show);
}

/**
 * @brief Get the path to the tv show character art
 * @param show TV Show object
 * @return Path to character art image
 */
QString XbmcXml::characterArtImageName(TvShow *show)
{
    return characterArtImageNameStatic(show);
}

/**
 * @brief Get path to season poster
 * @param show
 * @param season
 * @return
 */
QString XbmcXml::seasonPosterImageName(TvShow *show, int season)
{
    QString fileName;
    if (show->dir().isEmpty())
        return fileName;
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowSeasonPoster)) {
        QString file = dataFile.saveFileName("", season);
        QFileInfo fi(show->dir() + QDir::separator() + file);
        if (fi.isFile()) {
            fileName = show->dir() + QDir::separator() + file;
            break;
        }
    }

    return fileName;
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
    QString fileName = show->dir() + QDir::separator() + ".actors" + QDir::separator() + actorName + ".tbn";
    QFileInfo fi(fileName);
    if (fi.isFile())
        return fileName;
    return QString();
}

/**
 * @brief Gets path to thumbnail image
 * @param episode
 * @return
 */
QString XbmcXml::thumbnailImageName(TvShowEpisode *episode)
{
    QString fileName;
    if (episode->files().size() == 0)
        return fileName;
    QFileInfo fi(episode->files().at(0));

    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowEpisodeThumb)) {
        QString file = dataFile.saveFileName(fi.fileName());
        QFileInfo pFi(fi.absolutePath() + QDir::separator() + file);
        if (pFi.isFile()) {
            fileName = fi.absolutePath() + QDir::separator() + file;
            break;
        }
    }

    return fileName;
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
            QFileInfo nfoFi(show->dir() + QDir::separator() + file);
            if (nfoFi.exists()) {
                nfoFile = show->dir() + QDir::separator() + file;
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
    if (!domDoc.elementsByTagName("tvdbid").isEmpty() )
        show->setTvdbId(domDoc.elementsByTagName("tvdbid").at(0).toElement().text());
    if (!domDoc.elementsByTagName("title").isEmpty() )
        show->setName(domDoc.elementsByTagName("title").at(0).toElement().text());
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

    for (int i=0, n=domDoc.elementsByTagName("genre").size() ; i<n ; i++)
        show->addGenre(domDoc.elementsByTagName("genre").at(i).toElement().text());
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
        if (episode->files().size() == 0) {
            qWarning() << "Episode has no files";
            return false;
        }
        QFileInfo fi(episode->files().at(0));
        if (!fi.isFile() ) {
            qDebug() << "Episode file 0 is no file" << episode->files();
            return false;
        }

        QString nfoFile;
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowEpisodeNfo)) {
            QString file = dataFile.saveFileName(fi.fileName());
            QFileInfo nfoFi(fi.absolutePath() + QDir::separator() + file);
            if (nfoFi.exists()) {
                nfoFile = fi.absolutePath() + QDir::separator() + file;
                break;
            }
        }

        if (nfoFile.isEmpty()) {
            qDebug() << "No usable nfo file found";
            return false;
        }

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

    QDomDocument domDoc;
    domDoc.setContent(nfoContent);
    if (!domDoc.elementsByTagName("title").isEmpty() )
        episode->setName(domDoc.elementsByTagName("title").at(0).toElement().text());
    if (!domDoc.elementsByTagName("showtitle").isEmpty() )
        episode->setShowTitle(domDoc.elementsByTagName("showtitle").at(0).toElement().text());
    if (!domDoc.elementsByTagName("season").isEmpty())
        episode->setSeason(domDoc.elementsByTagName("season").at(0).toElement().text().toInt());
    if (!domDoc.elementsByTagName("episode").isEmpty())
        episode->setEpisode(domDoc.elementsByTagName("episode").at(0).toElement().text().toInt());
    if (!domDoc.elementsByTagName("rating").isEmpty())
        episode->setRating(domDoc.elementsByTagName("rating").at(0).toElement().text().toFloat());
    if (!domDoc.elementsByTagName("plot").isEmpty())
        episode->setOverview(domDoc.elementsByTagName("plot").at(0).toElement().text());
    if (!domDoc.elementsByTagName("mpaa").isEmpty())
        episode->setCertification(domDoc.elementsByTagName("mpaa").at(0).toElement().text());
    if (!domDoc.elementsByTagName("aired").isEmpty())
        episode->setFirstAired(QDate::fromString(domDoc.elementsByTagName("aired").at(0).toElement().text(), "yyyy-MM-dd"));
    if (!domDoc.elementsByTagName("playcount").isEmpty())
        episode->setPlayCount(domDoc.elementsByTagName("playcount").at(0).toElement().text().toInt());
    if (!domDoc.elementsByTagName("lastplayed").isEmpty())
        episode->setLastPlayed(QDateTime::fromString(domDoc.elementsByTagName("lastplayed").at(0).toElement().text(), "yyyy-MM-dd HH:mm:ss"));
    if (!domDoc.elementsByTagName("studio").isEmpty())
        episode->setNetwork(domDoc.elementsByTagName("studio").at(0).toElement().text());
    if (!domDoc.elementsByTagName("thumb").isEmpty())
        episode->setThumbnail(QUrl(domDoc.elementsByTagName("thumb").at(0).toElement().text()));
    for (int i=0, n=domDoc.elementsByTagName("credits").size() ; i<n ; i++)
        episode->addWriter(domDoc.elementsByTagName("credits").at(i).toElement().text());
    for (int i=0, n=domDoc.elementsByTagName("director").size() ; i<n ; i++)
        episode->addDirector(domDoc.elementsByTagName("director").at(i).toElement().text());

    episode->setStreamDetailsLoaded(loadStreamDetails(episode->streamDetails(), domDoc));

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
        QFile file(show->dir() + QDir::separator() + dataFile.saveFileName(""));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Nfo file could not be openend for writing" << file.fileName();
            return false;
        }
        file.write(xmlContent);
        file.close();
    }

    if (show->posterImageChanged() && !show->posterImage()->isNull()) {
        qDebug() << "Poster image has changed";
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowPoster)) {
            QString saveFileName = dataFile.saveFileName("");
            qDebug() << "Saving poster to" << show->dir() + QDir::separator() + saveFileName;
            show->posterImage()->save(show->dir() + QDir::separator() + saveFileName, "jpg", 100);
        }
    }

    if (show->backdropImageChanged() && !show->backdropImage()->isNull()) {
        qDebug() << "Backdrop image has changed";
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowBackdrop)) {
            QString saveFileName = dataFile.saveFileName("");
            qDebug() << "Saving poster to" << show->dir() + QDir::separator() + saveFileName;
            show->backdropImage()->save(show->dir() + QDir::separator() + saveFileName, "jpg", 100);
        }
    }

    if (show->bannerImageChanged() && !show->bannerImage()->isNull()) {
        qDebug() << "Banner image has changed";
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowBanner)) {
            QString saveFileName = dataFile.saveFileName("");
            qDebug() << "Saving banner to" << show->dir() + QDir::separator() + saveFileName;
            show->bannerImage()->save(show->dir() + QDir::separator() + saveFileName, "jpg", 100);
        }
    }

    foreach (const Actor &actor, show->actors()) {
        if (!actor.image.isNull()) {
            QDir dir;
            dir.mkdir(show->dir() + QDir::separator() + ".actors");
            QString actorName = actor.name;
            actorName = actorName.replace(" ", "_");
            actor.image.save(show->dir() + QDir::separator() + ".actors" + QDir::separator() + actorName + ".tbn", "jpg", 100);
        }
    }

    foreach (int season, show->seasons()) {
        if (show->seasonPosterImageChanged(season) && !show->seasonPosterImage(season)->isNull()) {
            foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowSeasonPoster)) {
                QString saveFileName = dataFile.saveFileName("", season);
                qDebug() << "Saving season poster for season" << season << "to" << show->dir() + QDir::separator() + saveFileName;
                show->seasonPosterImage(season)->save(show->dir() + QDir::separator() + saveFileName, "jpg", 100);
            }
        }
    }
    saveAdditionalImages(show);

    return true;
}

/**
 * @brief Saves additional tv show images (logo, clear art,)
 * @param show TV Show object
 */
void XbmcXml::saveAdditionalImages(TvShow *show)
{
    if (show->logoImageChanged() && !show->logoImage()->isNull()) {
        qDebug() << "Logo image has changed";
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowLogo)) {
            QString saveFileName = dataFile.saveFileName("");
            qDebug() << "Saving logo to" << show->dir() + QDir::separator() + saveFileName;
            show->logoImage()->save(show->dir() + QDir::separator() + saveFileName, "png", 100);
        }
    }
    if (show->clearArtImageChanged() && !show->clearArtImage()->isNull()) {
        qDebug() << "Clear art image has changed";
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowClearArt)) {
            QString saveFileName = dataFile.saveFileName("");
            qDebug() << "Saving clear art to" << show->dir() + QDir::separator() + saveFileName;
            show->clearArtImage()->save(show->dir() + QDir::separator() + saveFileName, "png", 100);
        }
    }
    if (show->characterArtImageChanged() && !show->characterArtImage()->isNull()) {
        qDebug() << "Character art image has changed";
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowCharacterArt)) {
            QString saveFileName = dataFile.saveFileName("");
            qDebug() << "Saving character art to" << show->dir() + QDir::separator() + saveFileName;
            show->characterArtImage()->save(show->dir() + QDir::separator() + saveFileName, "png", 100);
        }
    }
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
    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);
    writeTvShowEpisodeXml(xml, episode);
    xml.writeEndDocument();

    if (episode->files().isEmpty()) {
        qWarning() << "Episode has no files";
        return false;
    }

    episode->setNfoContent(xmlContent);
    Manager::instance()->database()->update(episode);

    QFileInfo fi(episode->files().at(0));
    foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowEpisodeNfo)) {
        QString saveFileName = dataFile.saveFileName(fi.fileName());
        QFile file(fi.absolutePath() + QDir::separator() + saveFileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Nfo file could not be opened for writing" << saveFileName;
            return false;
        }
        file.write(xmlContent);
        file.close();
    }

    if (episode->thumbnailImageChanged() && !episode->thumbnailImage()->isNull()) {
        foreach (DataFile dataFile, Settings::instance()->dataFiles(DataFileType::TvShowEpisodeThumb)) {
            QString saveFileName = dataFile.saveFileName(fi.fileName());
            qDebug() << "Thumbnail image has changed, saving to" << fi.absolutePath() + QDir::separator() + saveFileName;
            episode->thumbnailImage()->save(fi.absolutePath() + QDir::separator() + saveFileName, "jpg", 100);
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
    xml.writeTextElement("rating", QString("%1").arg(show->rating()));
    xml.writeTextElement("episode", QString("%1").arg(show->episodes().count()));
    xml.writeTextElement("plot", show->overview());
    xml.writeTextElement("outline", show->overview());
    xml.writeTextElement("mpaa", QString("%1").arg(show->certification()));
    xml.writeTextElement("premiered", show->firstAired().toString("yyyy-MM-dd"));
    xml.writeTextElement("studio", show->network());
    xml.writeTextElement("tvdbid", show->tvdbId());
    xml.writeTextElement("id", show->tvdbId());

    if (!show->episodeGuideUrl().isEmpty()) {
        xml.writeStartElement("episodeguide");
        xml.writeTextElement("url", show->episodeGuideUrl());
        xml.writeEndElement();
    }

    foreach (const QString &genre, show->genres())
        xml.writeTextElement("genre", genre);

    foreach (const Actor &actor, show->actors()) {
        xml.writeStartElement("actor");
        xml.writeTextElement("name", actor.name);
        xml.writeTextElement("role", actor.role);
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
    xml.writeTextElement("plot", episode->overview());
    xml.writeTextElement("outline", episode->overview());
    xml.writeTextElement("mpaa", episode->certification());
    xml.writeTextElement("playcount", QString("%1").arg(episode->playCount()));
    xml.writeTextElement("lastplayed", episode->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    xml.writeTextElement("aired", episode->firstAired().toString("yyyy-MM-dd"));
    xml.writeTextElement("studio", episode->network());
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

    xml.writeStartElement("fileinfo");
    writeStreamDetails(xml, episode->streamDetails());
    xml.writeEndElement();

    xml.writeEndElement();
}

/**
 * @brief Loading of movie set posters is not possible with nfos
 * @param setName
 * @return
 */
QImage XbmcXml::movieSetPoster(QString setName)
{
    Q_UNUSED(setName);
    return QImage();
}

/**
 * @brief Loading of movie set backdrops is not possible with nfos
 * @param setName
 * @return
 */
QImage XbmcXml::movieSetBackdrop(QString setName)
{
    Q_UNUSED(setName);
    return QImage();
}

/**
 * @brief Saving of movie set posters is not possible with nfos
 * @param setName
 * @param poster
 */
void XbmcXml::saveMovieSetPoster(QString setName, QImage poster)
{
    Q_UNUSED(setName);
    Q_UNUSED(poster);
}

/**
 * @brief Saving of movie set backdrops is not possible with nfos
 * @param setName
 * @param backdrop
 */
void XbmcXml::saveMovieSetBackdrop(QString setName, QImage backdrop)
{
    Q_UNUSED(setName);
    Q_UNUSED(backdrop);
}
