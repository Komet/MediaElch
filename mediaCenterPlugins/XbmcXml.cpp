#include "XbmcXml.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QFileInfo>
#include <QXmlStreamWriter>

#include "globals/Globals.h"
#include "globals/Helper.h"

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
 * @brief Gets called when MediaElch shuts down
 */
void XbmcXml::shutdown()
{
}

/**
 * @brief Writes movie elements to an xml stream
 * @param xml XML stream
 * @param movie Movie to save
 * @param writePath If true the full path of the files will be written (currently unused, just for export)
 * @param pathSearch (currently unused, just for export)
 * @param pathReplace (currently unused, just for export)
 * @todo: Remove last three parameters or reimplement (Export)
 */
void XbmcXml::writeMovieXml(QXmlStreamWriter &xml, Movie *movie, bool writePath, QString pathSearch, QString pathReplace)
{
    qDebug() << "Entered, movie=" << movie->name();
    xml.writeStartElement("movie");
    xml.writeTextElement("title", movie->name());
    xml.writeTextElement("originaltitle", movie->originalName());
    xml.writeTextElement("rating", QString("%1").arg(movie->rating()));
    xml.writeTextElement("year", movie->released().toString("yyyy"));
    xml.writeTextElement("plot", movie->overview());
    xml.writeTextElement("tagline", movie->tagline());
    if (movie->runtime() > 0)
        xml.writeTextElement("runtime", QString("%1").arg(movie->runtime()));
    xml.writeTextElement("mpaa", movie->certification());
    xml.writeTextElement("playcount", QString("%1").arg(movie->playcount()));
    xml.writeTextElement("lastplayed", movie->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    if (writePath && movie->files().size() > 0) {
        QFileInfo fi(movie->files().at(0));
        xml.writeTextElement("path", fi.absolutePath());
        if (movie->files().size() == 1) {
            fi.setFile(movie->files().at(0));
            xml.writeTextElement("filenameandpath", fi.absoluteFilePath().replace(pathSearch, pathReplace));
            xml.writeTextElement("basepath", fi.absoluteFilePath().replace(pathSearch, pathReplace));
        } else {
            QStringList files;
            foreach (const QString &file, movie->files()) {
                fi.setFile(file);
                files.append(fi.absoluteFilePath().replace(pathSearch, pathReplace));
            }
            xml.writeTextElement("filenameandpath", QString("stack://%1").arg(files.join(" , ")));
            xml.writeTextElement("basepath", QString("stack://%1").arg(files.join(" , ")));
        }
    }
    xml.writeTextElement("id", movie->id());
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
    QFileInfo fi(movie->files().at(0));
    QFile file;
    if (m_movieNfoFileNames.contains(movie))
        file.setFileName(fi.absolutePath() + QDir::separator() + m_movieNfoFileNames[movie]);
    else
        file.setFileName(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".nfo");
    qDebug() << "Saving to" << file.fileName();
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "File could not be openend";
        return false;
    }
    file.write(xmlContent);
    file.close();

    if (movie->posterImageChanged() && !movie->posterImage()->isNull()) {
        if (m_moviePosterFileNames.contains(movie)) {
            qDebug() << "Saving poster to" << fi.absolutePath() + QDir::separator() + m_moviePosterFileNames[movie];
            movie->posterImage()->save(fi.absolutePath() + QDir::separator() + m_moviePosterFileNames[movie], "jpg", 100);
        } else {
            qDebug() << "Saving poster to" << fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".tbn";
            movie->posterImage()->save(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".tbn", "jpg", 100);
        }
    }
    if (movie->backdropImageChanged() && !movie->backdropImage()->isNull()) {
        if (m_movieBackdropFileNames.contains(movie)) {
            qDebug() << "Saving backdrop to" << fi.absolutePath() + QDir::separator() + m_movieBackdropFileNames[movie];
            movie->backdropImage()->save(fi.absolutePath() + QDir::separator() + m_movieBackdropFileNames[movie], "jpg", 100);
        } else {
            qDebug() << "Saving backdrop to" << fi.absolutePath() + QDir::separator() + fi.completeBaseName() + "-fanart.jpg";
            movie->backdropImage()->save(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + "-fanart.jpg", "jpg", 100);
        }
    }

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
 * @brief Loads movie infos (except images)
 * @param movie Movie to load
 * @return Loading success
 */
bool XbmcXml::loadMovie(Movie *movie)
{
    qDebug() << "Entered, files=" << movie->files();
    if (movie->files().size() == 0) {
        qWarning() << "Movie has no files";
        return false;
    }
    QFileInfo fi(movie->files().at(0));
    if (!fi.isFile() ) {
        qWarning() << "First file of the movie is not readable" << movie->files().at(0);
        return false;
    }
    QString nfoFile = fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".nfo";
    qDebug() << "Trying to load nfoFile" << nfoFile;
    fi.setFile(nfoFile);
    if (!fi.exists()) {
        // try movie.nfo instead
        nfoFile = fi.absolutePath() + QDir::separator() + "movie.nfo";
        qDebug() << "Trying to load nfoFile" << nfoFile;
        fi.setFile(nfoFile);
        if (!fi.exists()) {
            qDebug() << "No usable nfo file found";
            return false;
        }
    }

    if (m_movieNfoFileNames.contains(movie))
        m_movieNfoFileNames[movie] = fi.fileName();
    else
        m_movieNfoFileNames.insert(movie, fi.fileName());

    QFile file(nfoFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "File" << nfoFile << "could not be opened for reading";
        return false;
    }
    movie->clear();
    movie->setChanged(false);
    QDomDocument domDoc;
    domDoc.setContent(file.readAll());
    if (!domDoc.elementsByTagName("title").isEmpty() )
        movie->setName(domDoc.elementsByTagName("title").at(0).toElement().text());
    if (!domDoc.elementsByTagName("originaltitle").isEmpty())
        movie->setOriginalName(domDoc.elementsByTagName("originaltitle").at(0).toElement().text());
    if (!domDoc.elementsByTagName("rating").isEmpty())
        movie->setRating(domDoc.elementsByTagName("rating").at(0).toElement().text().toFloat());
    if (!domDoc.elementsByTagName("year").isEmpty())
        movie->setReleased(QDate::fromString(domDoc.elementsByTagName("year").at(0).toElement().text(), "yyyy"));
    if (!domDoc.elementsByTagName("plot").isEmpty())
        movie->setOverview(domDoc.elementsByTagName("plot").at(0).toElement().text());
    if (!domDoc.elementsByTagName("tagline").isEmpty())
        movie->setTagline(domDoc.elementsByTagName("tagline").at(0).toElement().text());
    if (!domDoc.elementsByTagName("runtime").isEmpty())
        movie->setRuntime(domDoc.elementsByTagName("runtime").at(0).toElement().text().toInt());
    if (!domDoc.elementsByTagName("mpaa").isEmpty())
        movie->setCertification(domDoc.elementsByTagName("mpaa").at(0).toElement().text());
    if (!domDoc.elementsByTagName("playcount").isEmpty())
        movie->setPlayCount(domDoc.elementsByTagName("playcount").at(0).toElement().text().toInt());
    if (!domDoc.elementsByTagName("lastplayed").isEmpty())
        movie->setLastPlayed(QDateTime::fromString(domDoc.elementsByTagName("lastplayed").at(0).toElement().text(), "yyyy-MM-dd HH:mm:ss"));
    if (!domDoc.elementsByTagName("id").isEmpty())
        movie->setId(domDoc.elementsByTagName("id").at(0).toElement().text());
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

    file.close();

    return true;
}

/**
 * @brief Loads images of a movie
 * @param movie Movie to load
 */
void XbmcXml::loadMovieImages(Movie *movie)
{
    qDebug() << "Entered, movie=" << movie->name();
    if (movie->files().size() == 0) {
        qWarning() << "Movie has no files";
        return;
    }
    QFileInfo fi(movie->files().at(0));
    QFileInfo posterFi(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".tbn");
    QFileInfo backdropFi(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + "-fanart.jpg");

    if (!posterFi.isFile())
        posterFi.setFile(fi.absolutePath() + QDir::separator() + "movie.tbn");
    if (!posterFi.isFile())
        posterFi.setFile(fi.absolutePath() + QDir::separator() + "movie.jpg");
    if (!posterFi.isFile())
        posterFi.setFile(fi.absolutePath() + QDir::separator() + "folder.jpg");

    if (posterFi.isFile()) {
        qDebug() << "Trying to load poster file" << posterFi.absoluteFilePath();
        movie->posterImage()->load(posterFi.absoluteFilePath());
        if (m_moviePosterFileNames.contains(movie))
            m_moviePosterFileNames[movie] = posterFi.fileName();
        else
            m_moviePosterFileNames.insert(movie, posterFi.fileName());
    } else {
        qDebug() << "Poster file is invalid" << posterFi.absoluteFilePath();
    }

    if (!backdropFi.isFile())
        backdropFi.setFile(fi.absolutePath() + QDir::separator() + "fanart.jpg");

    if (backdropFi.isFile()) {
        qDebug() << "Trying to load backdrop file" << backdropFi.absoluteFilePath();
        movie->backdropImage()->load(backdropFi.absoluteFilePath());
        if (m_movieBackdropFileNames.contains(movie))
            m_movieBackdropFileNames[movie] = backdropFi.fileName();
        else
            m_movieBackdropFileNames.insert(movie, backdropFi.fileName());
    } else {
        qDebug() << "Backdrop file is invalid" << backdropFi.absoluteFilePath();
    }

    foreach (Actor *actor, movie->actorsPointer()) {
        if (actor->imageHasChanged)
            continue;
        QString actorName = actor->name;
        actorName = actorName.replace(" ", "_");
        actor->image.load(fi.absolutePath() + QDir::separator() + ".actors" + QDir::separator() + actorName + ".tbn");
    }
}

/**
 * @brief Loads images for a tv show
 * @param show Show to load images for
 */
void XbmcXml::loadTvShowImages(TvShow *show)
{
    qDebug() << "Entered, show=" << show->name();
    if (show->dir().isEmpty()) {
        qWarning() << "TvShow has no dir";
        return;
    }
    QFileInfo posterFi(show->dir() + QDir::separator() + "season-all.tbn");
    QFileInfo backdropFi(show->dir() + QDir::separator() + "fanart.jpg");
    QFileInfo bannerFi(show->dir() + QDir::separator() + "folder.jpg");
    if (posterFi.isFile()) {
        qDebug() << "Trying to load poster file" << posterFi.absoluteFilePath();
        show->posterImage()->load(posterFi.absoluteFilePath());
    } else {
        posterFi.setFile(show->dir() + QDir::separator() + "poster.jpg");
        if (posterFi.isFile()) {
            qDebug() << "Trying to load poster file" << posterFi.absoluteFilePath();
            show->posterImage()->load(posterFi.absoluteFilePath());
        }
    }
    if (backdropFi.isFile()) {
        qDebug() << "Trying to load backdrop file" << backdropFi.absoluteFilePath();
        show->backdropImage()->load(backdropFi.absoluteFilePath());
    }
    if (bannerFi.isFile()) {
        qDebug() << "Trying to load banner file" << bannerFi.absoluteFilePath();
        show->bannerImage()->load(bannerFi.absoluteFilePath());
    } else {
        bannerFi.setFile(show->dir() + QDir::separator() + "banner.jpg");
        if (bannerFi.isFile()) {
            qDebug() << "Trying to load banner file" << bannerFi.absoluteFilePath();
            show->bannerImage()->load(bannerFi.absoluteFilePath());
        }
    }

    foreach (int season, show->seasons()) {
        QString s = QString("%1").arg(season);
        if (season < 10)
            s.prepend("0");
        QFileInfo seasonFi(show->dir() + QDir::separator() + "season" + s + ".tbn");
        if (seasonFi.isFile()) {
            qDebug() << "Trying to load season poster" << seasonFi.absoluteFilePath() << "for season" << season;
            show->seasonPosterImage(season)->load(seasonFi.absoluteFilePath());
        }
    }

    foreach (Actor *actor, show->actorsPointer()) {
        if (actor->imageHasChanged)
            continue;
        QString actorName = actor->name;
        actorName = actorName.replace(" ", "_");
        actor->image.load(show->dir() + QDir::separator() + ".actors" + QDir::separator() + actorName + ".tbn");
    }
}

/**
 * @brief Loads images for a tv show episode
 * @param episode Episode to load images for
 */
void XbmcXml::loadTvShowEpisodeImages(TvShowEpisode *episode)
{
    qDebug() << "Entered, episode=" << episode->name();
    if (episode->files().isEmpty()) {
        qWarning() << "Episode has no files";
        return;
    }
    QFileInfo fi(episode->files().at(0));
    QFileInfo thumbnailFi(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".tbn");
    if (thumbnailFi.isFile()) {
        qDebug() << "Trying to load thumbnail file" << thumbnailFi.absoluteFilePath();
        episode->thumbnailImage()->load(thumbnailFi.absoluteFilePath());
    }
}

/**
 * @brief Exports a list of movies and tv shows into a single xml file
 * This is currently unused due to disabled export
 * @param movies List of movies to export
 * @param shows List of shows to export
 * @param exportPath
 * @param pathSearch
 * @param pathReplace
 * @todo: Remove or reimplement (Export)
 */
void XbmcXml::exportDatabase(QList<Movie*> movies, QList<TvShow*> shows, QString exportPath, QString pathSearch, QString pathReplace)
{
    emit sigExportStarted();

    QDir dir(exportPath);
    if (!dir.mkdir("actors")) {
        emit sigExportRaiseError(tr("Could not create actors directory"));
        return;
    }
    if (!dir.mkdir("movies")) {
        emit sigExportRaiseError(tr("Could not create movies directory"));
        return;
    }
    if (!dir.mkdir("tvshows")) {
        emit sigExportRaiseError(tr("Could not create tv shows directory"));
        return;
    }

    int numOfMovies = movies.count();
    int numOfElements = numOfMovies + shows.size();
    for (int i=0, n=shows.count() ; i<n ; ++i)
        numOfElements += shows[i]->episodeCount();

    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);
    xml.writeStartElement("videodb");
    xml.writeTextElement("version", "1");
    for (int i=0, n=movies.size() ; i<n ; ++i) {
        emit sigExportProgress(i, numOfElements);
        Movie *movie = movies[i];
        if (!movie->infoLoaded())
            continue;

        writeMovieXml(xml, movie, true, pathSearch, pathReplace);

        if (movie->files().size() == 0)
            continue;

        QFileInfo fi(movie->files().at(0));
        QString actorPath = fi.absolutePath() + QDir::separator() + ".actors";
        QDir movieDir(actorPath);
        QStringList actorFilters;
        actorFilters << "*.tbn";
        foreach (QString actorFile, movieDir.entryList(actorFilters, QDir::NoDotAndDotDot | QDir::Files))
            QFile::copy(actorPath + QDir::separator() + actorFile, exportPath + QDir::separator() + "actors" + QDir::separator() + actorFile.replace(" ", "_"));

        QFileInfo posterFi(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".tbn");
        QFileInfo backdropFi(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + "-fanart.jpg");
        QString newPosterName = QString("%1_%2.tbn").arg(movie->name().replace(" ", "_")).arg(movie->released().year());
        QString newBackdropName = QString("%1_%2-fanart.jpg").arg(movie->name().replace(" ", "_")).arg(movie->released().year());
        if (posterFi.isFile())
            QFile::copy(posterFi.absoluteFilePath(), exportPath + QDir::separator() + "movies" + QDir::separator() + newPosterName);
        if (backdropFi.isFile())
            QFile::copy(backdropFi.absoluteFilePath(), exportPath + QDir::separator() + "movies" + QDir::separator() + newBackdropName);
    }

    int progress = numOfMovies;
    for (int i=0, x=shows.count() ; i<x ; ++i) {
        emit sigExportProgress(progress++, numOfElements);
        if (!shows[i]->infoLoaded())
            continue;

        // create tv show directory
        QDir tvShowDir(exportPath + QDir::separator() + "tvshows");
        if (!tvShowDir.mkdir(shows[i]->name().replace(" ", "_")))
            continue;
        tvShowDir.setPath(exportPath + QDir::separator() + "tvshows" + QDir::separator() + shows[i]->name().replace(" ", "_"));

        // copy actors
        QDir actorsDir(shows[i]->dir() + QDir::separator() + ".actors");
        QStringList actorFilters;
        actorFilters << "*.tbn";
        foreach (QString actorFile, actorsDir.entryList(actorFilters, QDir::NoDotAndDotDot | QDir::Files))
            QFile::copy(actorsDir.absolutePath() + QDir::separator() + actorFile, exportPath + QDir::separator() + "actors" + QDir::separator() + actorFile.replace(" ", "_"));

        // copy poster and backdrop
        QFileInfo posterFi(shows[i]->dir() + QDir::separator() + "season-all.tbn");
        QFileInfo backdropFi(shows[i]->dir() + QDir::separator() + "fanart.jpg");
        if (posterFi.isFile())
            QFile::copy(posterFi.absoluteFilePath(), tvShowDir.absolutePath() + QDir::separator() + "season-all.tbn");
        if (backdropFi.isFile())
            QFile::copy(backdropFi.absoluteFilePath(), tvShowDir.absolutePath() + QDir::separator() + "fanart.jpg");

        // copy season backdrops
        foreach (int s, shows[i]->seasons()) {
            QString season = QString::number(s);
            if (s < 10)
                season.prepend("0");
            QFileInfo seasonFi(shows[i]->dir() + QDir::separator() + "season" + season + ".tbn");
            if (seasonFi.isFile())
                QFile::copy(seasonFi.absoluteFilePath(), tvShowDir.absolutePath() + QDir::separator() + "season" + season + ".tbn");
        }

        xml.writeStartElement("tvshow");
        writeTvShowXml(xml, shows[i], true, pathSearch, pathReplace, false);

        for (int n=0, y=shows[i]->episodes().count() ; n<y ; ++n) {
            emit sigExportProgress(progress++, numOfElements);
            TvShowEpisode *episode = shows[i]->episodes().at(n);
            if (!episode->infoLoaded())
                continue;
            if (episode->files().isEmpty())
                continue;

            QFileInfo fi(episode->files().at(0));
            QFileInfo thumbFi(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".tbn");
            if (thumbFi.isFile())
                QFile::copy(thumbFi.absoluteFilePath(), tvShowDir.absolutePath() + QDir::separator() + "s" + episode->seasonString() + "e" + episode->episodeString() + ".tbn");

            writeTvShowEpisodeXml(xml, shows[i]->episodes().at(n), true, pathSearch, pathReplace);
        }
        xml.writeEndElement();
    }

    xml.writeEndElement();
    xml.writeEndDocument();

    QFile file(exportPath + QDir::separator() + "videodb.xml");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(xmlContent);
        file.close();
    }

    emit sigExportDone();
}

/**
 * @brief Loads tv show information
 * @param show Show to load
 * @return Loading success
 */
bool XbmcXml::loadTvShow(TvShow *show)
{
    qDebug() << "Entered, show=" << show->name();
    if (show->dir().isEmpty()) {
        qWarning() << "Show dir is empty";
        return false;
    }
    QFileInfo fi(show->dir().append(QDir::separator()).append("tvshow.nfo"));
    if (!fi.isFile() ) {
        qDebug() << "Nfo doesn't exist" << fi.absoluteFilePath();
        return false;
    }

    QFile file(fi.absoluteFilePath());
    qDebug() << "Trying to load" << fi.absoluteFilePath();
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Nfo file could not be opened for reading" << fi.absoluteFilePath();
        return false;
    }

    show->clear();
    QDomDocument domDoc;
    domDoc.setContent(file.readAll());
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

    file.close();

    return true;
}

/**
 * @brief Loads tv show episode information
 * @param episode Episode to load infos for
 * @return Loading success
 */
bool XbmcXml::loadTvShowEpisode(TvShowEpisode *episode)
{
    qDebug() << "Entered, episode=" << episode->name();
    if (episode->files().size() == 0) {
        qWarning() << "Episode has no files";
        return false;
    }
    QFileInfo fi(episode->files().at(0));
    if (!fi.isFile() ) {
        qDebug() << "Episode file 0 is no file" << episode->files();
        return false;
    }
    QString nfoFile = fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".nfo";
    fi.setFile(nfoFile);
    qDebug() << "Trying to load" << nfoFile;
    if (!fi.exists()) {
        qDebug() << "Nfo file doesn't exist" << nfoFile;
        return false;
    }

    QFile file(nfoFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Nfo file could not be opened for reading" << nfoFile;
        return false;
    }
    episode->clear();

    QDomDocument domDoc;
    domDoc.setContent(file.readAll());
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

    file.close();

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
    qDebug() << "Entered, show=" << show->name();
    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);
    writeTvShowXml(xml, show);
    xml.writeEndDocument();

    if (show->dir().isEmpty()) {
        qWarning() << "Show dir is not set";
        return false;
    }
    QFile file(show->dir() + QDir::separator() + "tvshow.nfo");
    qDebug() << "Trying to load" << file.fileName();
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Nfo file could not be openend for writing";
        return false;
    }
    file.write(xmlContent);
    file.close();

    if (show->posterImageChanged() && !show->posterImage()->isNull()) {
        qDebug() << "Poster image has changed";
        qDebug() << "Saving to" << show->dir() + QDir::separator() + "season-all.tbn";
        qDebug() << "Saving to" << show->dir() + QDir::separator() + "poster.jpg";
        show->posterImage()->save(show->dir() + QDir::separator() + "season-all.tbn", "jpg", 100);
        show->posterImage()->save(show->dir() + QDir::separator() + "poster.jpg", "jpg", 100);
    }
    if (show->backdropImageChanged() && !show->backdropImage()->isNull()) {
        qDebug() << "Backdrop image has changed";
        qDebug() << "Saving to " << show->dir() + QDir::separator() + "fanart.jpg";
        show->backdropImage()->save(show->dir() + QDir::separator() + "fanart.jpg", "jpg", 100);
    }
    if (show->bannerImageChanged() && !show->bannerImage()->isNull()) {
        qDebug() << "Banner image has changed";
        qDebug() << "Saving to" << show->dir() + QDir::separator() + "folder.jpg";
        qDebug() << "Saving to" << show->dir() + QDir::separator() + "banner.jpg";
        show->bannerImage()->save(show->dir() + QDir::separator() + "folder.jpg", "jpg", 100);
        show->bannerImage()->save(show->dir() + QDir::separator() + "banner.jpg", "jpg", 100);
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
            QString s = QString("%1").arg(season);
            if (season < 10)
                s.prepend("0");
            qDebug() << "Saving season poster image for season" << season << "to" << show->dir() + QDir::separator() + "season" + s + ".tbn";
            show->seasonPosterImage(season)->save(show->dir() + QDir::separator() + "season" + s + ".tbn", "jpg", 100);
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
    QFileInfo fi(episode->files().at(0));
    QFile file(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".nfo");
    qDebug() << "Trying to open" << file.fileName();
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Nfo file could not be opened for writing";
        return false;
    }
    file.write(xmlContent);
    file.close();

    if (episode->thumbnailImageChanged() && !episode->thumbnailImage()->isNull()) {
        qDebug() << "Thumbnail image has changed, saving to" << fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".tbn";
        episode->thumbnailImage()->save(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".tbn", "jpg", 100);
    }

    return true;
}

/**
 * @brief Writes tv show elements to an xml stream
 * @param xml XML stream
 * @param show Tv show to save
 * @param writePath If true the full path of the files will be written (currently unused, just for export)
 * @param pathSearch (currently unused, just for export)
 * @param pathReplace (currently unused, just for export)
 * @param writeStartAndEndElement
 * @todo: Remove last four parameters or reimplement (Export)
 */
void XbmcXml::writeTvShowXml(QXmlStreamWriter &xml, TvShow *show, bool writePath, QString pathSearch, QString pathReplace, bool writeStartAndEndElement)
{
    qDebug() << "Entered, show=" << show->name();
    if (writeStartAndEndElement)
        xml.writeStartElement("tvshow");
    xml.writeTextElement("title", show->name());
    xml.writeTextElement("showtitle", show->showTitle());
    xml.writeTextElement("rating", QString("%1").arg(show->rating()));
    xml.writeTextElement("episode", QString("%1").arg(show->episodes().count()));
    xml.writeTextElement("plot", show->overview());
    xml.writeTextElement("mpaa", QString("%1").arg(show->certification()));
    xml.writeTextElement("premiered", show->firstAired().toString("yyyy-MM-dd"));
    xml.writeTextElement("studio", show->network());

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

    if (writePath && !show->dir().isEmpty()) {
        QString dir = show->dir();
        xml.writeTextElement("path", dir.replace(pathSearch, pathReplace));
        xml.writeTextElement("filenameandpath", "");
        xml.writeTextElement("file", "");
        xml.writeTextElement("basepath", dir.replace(pathSearch, pathReplace));
    }

    if (writeStartAndEndElement)
        xml.writeEndElement();
}

/**
 * @brief Writes tv show episode elements to an xml stream
 * @param xml XML stream
 * @param episode Episode to save
 * @param writePath If true the full path of the files will be written (currently unused, just for export)
 * @param pathSearch (currently unused, just for export)
 * @param pathReplace (currently unused, just for export)
 * @todo: Remove last three parameters or reimplement (Export)
 */
void XbmcXml::writeTvShowEpisodeXml(QXmlStreamWriter &xml, TvShowEpisode *episode, bool writePath, QString pathSearch, QString pathReplace)
{
    qDebug() << "Entered, episode=" << episode->name();
    xml.writeStartElement("episodedetails");
    xml.writeTextElement("title", episode->name());
    xml.writeTextElement("showtitle", episode->showTitle());
    xml.writeTextElement("rating", QString("%1").arg(episode->rating()));
    xml.writeTextElement("season", QString("%1").arg(episode->season()));
    xml.writeTextElement("episode", QString("%1").arg(episode->episode()));
    xml.writeTextElement("plot", episode->overview());
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

    if (writePath && episode->files().size() > 0) {
        QFileInfo fi(episode->files().at(0));
        xml.writeTextElement("path", fi.absolutePath());
        if (episode->files().size() == 1) {
            fi.setFile(episode->files().at(0));
            xml.writeTextElement("filenameandpath", fi.absoluteFilePath().replace(pathSearch, pathReplace));
            xml.writeTextElement("basepath", fi.absoluteFilePath().replace(pathSearch, pathReplace));
        } else {
            QStringList files;
            foreach (const QString &file, episode->files()) {
                fi.setFile(file);
                files.append(fi.absoluteFilePath().replace(pathSearch, pathReplace));
            }
            xml.writeTextElement("filenameandpath", QString("stack://%1").arg(files.join(" , ")));
            xml.writeTextElement("basepath", QString("stack://%1").arg(files.join(" , ")));
        }
    }

    if (episode->tvShow() != 0) {
        foreach (const Actor &actor, episode->tvShow()->actors()) {
            xml.writeStartElement("actor");
            xml.writeTextElement("name", actor.name);
            xml.writeTextElement("role", actor.role);
            xml.writeTextElement("thumb", actor.thumb);
            xml.writeEndElement();
        }
    }
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
