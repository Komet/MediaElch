#include "OFDb.h"

#include <QDomDocument>
#include <QXmlStreamReader>

OFDb::OFDb(QObject *parent)
{
    setParent(parent);
    m_httpNotFoundCounter = 0;
}

QString OFDb::name()
{
    return QString("OFDb");
}

bool OFDb::hasSettings()
{
    return false;
}

void OFDb::loadSettings()
{
}

void OFDb::saveSettings()
{
}

QWidget *OFDb::settingsWidget()
{
    return 0;
}

QNetworkAccessManager *OFDb::qnam()
{
    return &m_qnam;
}

void OFDb::search(QString searchStr)
{
    m_httpNotFoundCounter = 0;
    m_currentSearchString = searchStr;
    QUrl url(QString("http://www.ofdbgw.org/search/%1").arg(searchStr));
    m_searchReply = this->qnam()->get(QNetworkRequest(url));
    connect(m_searchReply, SIGNAL(finished()), this, SLOT(searchFinished()));
}

void OFDb::searchFinished()
{
    if (m_searchReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302 ||
        m_searchReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 301) {
        m_searchReply->deleteLater();
        m_searchReply = this->qnam()->get(QNetworkRequest(m_searchReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl()));
        connect(m_searchReply, SIGNAL(finished()), this, SLOT(searchFinished()));
        return;
    }

    // try to get another mirror when 404 occurs
    if (m_searchReply->error() == QNetworkReply::ContentNotFoundError && m_httpNotFoundCounter < 3) {
        m_httpNotFoundCounter++;
        m_searchReply->deleteLater();
        QUrl url(QString("http://www.ofdbgw.org/search/%1").arg(m_currentSearchString));
        m_searchReply = this->qnam()->get(QNetworkRequest(url));
        connect(m_searchReply, SIGNAL(finished()), this, SLOT(searchFinished()));
        return;
    }

    QList<ScraperSearchResult> results;
    if (m_searchReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_searchReply->readAll());
        results = parseSearch(msg);
    }
    m_searchReply->deleteLater();
    emit searchDone(results);
}

QList<ScraperSearchResult> OFDb::parseSearch(QString xml)
{
    QList<ScraperSearchResult> results;
    QDomDocument domDoc;
    domDoc.setContent(xml);
    for (int i=0, n=domDoc.elementsByTagName("eintrag").size() ; i<n ; i++) {
        QDomElement entry = domDoc.elementsByTagName("eintrag").at(i).toElement();
        if (entry.elementsByTagName("id").size() == 0 || entry.elementsByTagName("id").at(0).toElement().text().isEmpty())
            continue;
        ScraperSearchResult result;
        result.id = entry.elementsByTagName("id").at(0).toElement().text();
        if (entry.elementsByTagName("titel").size() > 0)
            result.name = entry.elementsByTagName("titel").at(0).toElement().text();
        if (entry.elementsByTagName("jahr").size() > 0)
            result.released = QDate::fromString(entry.elementsByTagName("jahr").at(0).toElement().text(), "yyyy");
        results.append(result);
    }
    return results;
}

void OFDb::loadData(QString id, Movie *movie)
{
    m_httpNotFoundCounter = 0;
    m_currentLoadId = id;
    m_currentMovie = movie;
    QUrl url(QString("http://ofdbgw.org/movie/%1").arg(id));
    m_loadReply = this->qnam()->get(QNetworkRequest(url));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(loadFinished()));
}

void OFDb::loadFinished()
{
    if (m_loadReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302 ||
        m_loadReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 301) {
        m_loadReply->deleteLater();
        m_loadReply = this->qnam()->get(QNetworkRequest(m_loadReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl()));
        connect(m_loadReply, SIGNAL(finished()), this, SLOT(loadFinished()));
        return;
    }

    if (m_loadReply->error() == QNetworkReply::ContentNotFoundError && m_httpNotFoundCounter < 3) {
        m_httpNotFoundCounter++;
        m_loadReply->deleteLater();
        QUrl url(QString("http://ofdbgw.org/movie/%1").arg(m_currentLoadId));
        m_loadReply = this->qnam()->get(QNetworkRequest(url));
        connect(m_loadReply, SIGNAL(finished()), this, SLOT(loadFinished()));
        return;
    }


    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        parseAndAssignInfos(msg, m_currentMovie);
    }
    m_loadReply->deleteLater();
    m_currentMovie->scraperLoadDone();
}

void OFDb::parseAndAssignInfos(QString data, Movie *movie)
{
    movie->clear();
    QXmlStreamReader xml(data);

    xml.readNextStartElement();
    while (xml.readNextStartElement()) {
        if (xml.name() != "resultat")
            xml.skipCurrentElement();
        else
            break;
    }

    while (xml.readNextStartElement()) {
        if (xml.name() == "titel") {
            movie->setName(xml.readElementText());
        } else if (xml.name() == "jahr") {
            movie->setReleased(QDate::fromString(xml.readElementText(), "yyyy"));
        } else if (xml.name() == "bild") {
            Poster p;
            p.originalUrl = QUrl(xml.readElementText());
            p.thumbUrl = QUrl(xml.readElementText());
            movie->addPoster(p);
        } else if (xml.name() == "bewertung") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "note")
                    movie->setRating(xml.readElementText().toFloat());
                else
                    xml.skipCurrentElement();
            }
        } else if (xml.name() == "genre") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "titel")
                    movie->addGenre(xml.readElementText());
                else
                    xml.skipCurrentElement();
            }
        } else if (xml.name() == "besetzung") {
            while (xml.readNextStartElement()) {
                if (xml.name() != "person") {
                    xml.skipCurrentElement();
                } else {
                    Actor actor;
                    while (xml.readNextStartElement()) {
                        if (xml.name() == "name")
                            actor.name = xml.readElementText();
                        else if (xml.name() == "rolle")
                            actor.role = xml.readElementText();
                        else
                            xml.skipCurrentElement();
                    }
                    movie->addActor(actor);
                }
            }
        } else if (xml.name() == "produktionsland") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "name")
                    movie->addCountry(xml.readElementText());
                else
                    xml.skipCurrentElement();
            }
        } else if (xml.name() == "alternativ") {
            movie->setOriginalName(xml.readElementText());
        } else if (xml.name() == "beschreibung") {
            movie->setOverview(xml.readElementText());
        } else {
            xml.skipCurrentElement();
        }
    }
}
