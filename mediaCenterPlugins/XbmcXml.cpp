#include "XbmcXml.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QFileInfo>
#include <QXmlStreamWriter>

XbmcXml::XbmcXml(QObject *parent)
{
    setParent(parent);
}

XbmcXml::~XbmcXml()
{
}

void XbmcXml::writeXml(QXmlStreamWriter &xml, Movie *movie, bool writePath, QString pathSearch, QString pathReplace)
{
    xml.writeStartElement("movie");
    xml.writeTextElement("title", movie->name());
    xml.writeTextElement("originaltitle", movie->originalName());
    xml.writeTextElement("rating", QString("%1").arg(movie->rating()));
    xml.writeTextElement("year", movie->released().toString("yyyy"));
    xml.writeTextElement("plot", movie->overview());
    xml.writeTextElement("tagline", movie->tagline());
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
    xml.writeTextElement("trailer", movie->trailer().toString());
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

bool XbmcXml::saveData(Movie *movie)
{
    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);
    writeXml(xml, movie);
    xml.writeEndDocument();

    if (movie->files().size() == 0)
        return false;
    QFileInfo fi(movie->files().at(0));
    QFile file(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".nfo");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    file.write(xmlContent);
    file.close();

    if (movie->posterImageChanged() && !movie->posterImage()->isNull())
        movie->posterImage()->save(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".tbn", "jpg", 100);
    if (movie->backdropImageChanged() && !movie->backdropImage()->isNull())
        movie->backdropImage()->save(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + "-fanart.jpg", "jpg", 100);

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

bool XbmcXml::loadData(Movie *movie)
{
    if (movie->files().size() == 0)
        return false;
    QFileInfo fi(movie->files().at(0));
    if (!fi.isFile() ) {
        return false;
    }
    QString nfoFile = fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".nfo";
    fi.setFile(nfoFile);
    if (!fi.exists()) {
        return false;
    }

    QFile file(nfoFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    movie->clear();
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

void XbmcXml::loadImages(Movie *movie)
{
    if (movie->files().size() == 0)
        return;
    QFileInfo fi(movie->files().at(0));
    QFileInfo posterFi(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".tbn");
    QFileInfo backdropFi(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + "-fanart.jpg");
    if (posterFi.isFile()) {
        movie->posterImage()->load(posterFi.absoluteFilePath());
    }
    if (backdropFi.isFile()) {
        movie->backdropImage()->load(backdropFi.absoluteFilePath());
    }
}

void XbmcXml::exportDatabase(QList<Movie *> movies, QString exportPath, QString pathSearch, QString pathReplace)
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

    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);
    xml.writeStartElement("videodb");
    xml.writeTextElement("version", "1");
    for (int i=0, n=movies.size() ; i<n ; ++i) {
        emit sigExportProgress(i, n);
        Movie *movie = movies[i];
        writeXml(xml, movie, true, pathSearch, pathReplace);

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
    xml.writeEndElement();
    xml.writeEndDocument();

    QFile file(exportPath + QDir::separator() + "videodb.xml");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(xmlContent);
        file.close();
    }

    emit sigExportDone();
}
