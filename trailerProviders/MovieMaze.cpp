#include "MovieMaze.h"

#include <QRegExp>

MovieMaze::MovieMaze(QObject *parent)
{
    setParent(parent);
    m_qnam = new QNetworkAccessManager(this);
}

QString MovieMaze::name()
{
    return QString("MovieMaze");
}

void MovieMaze::searchMovie(QString searchStr)
{
    m_currentSearch = searchStr;
    QUrl url("http://www.moviemaze.de/media/trailer/archiv.phtml");
    QNetworkRequest request(url);
    m_searchReply = m_qnam->get(request);
    connect(m_searchReply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
}

void MovieMaze::onSearchFinished()
{
    QList<ScraperSearchResult> results;

    if (m_searchReply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromLatin1(m_searchReply->readAll());
        m_searchReply->deleteLater();

        int pos = 0;
        QRegExp rx("<a itemprop=\"url\" href=\"/media/trailer/([^\"]*)\" title=\"[^\"]*\"><span itemprop=\"name\">([^<]*)</span></a>");
        rx.setMinimal(true);
        while ((pos = rx.indexIn(msg, pos)) != -1) {
            if (rx.cap(2).contains(m_currentSearch, Qt::CaseInsensitive)) {
                ScraperSearchResult r;
                r.id = rx.cap(1);
                r.name = rx.cap(2);
                results.append(r);
            }
            pos += rx.matchedLength();
        }
    }
    m_searchReply->deleteLater();
    emit sigSearchDone(results);
}

void MovieMaze::loadMovieTrailers(QString id)
{
    m_trailerSites.clear();
    m_currentTrailers.clear();
    if (m_loadReply)
        m_loadReply->abort();
    m_loadReply = m_qnam->get(QNetworkRequest(QUrl("http://www.moviemaze.de/media/trailer/" + id)));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

void MovieMaze::onLoadFinished()
{
    if (m_loadReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302 ||
        m_loadReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 301) {
        qDebug() << "Got redirect" << m_loadReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        QString url = m_loadReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
        if (!url.startsWith("http://"))
            url.prepend("http://www.moviemaze.de");
        m_loadReply->deleteLater();
        m_loadReply = m_qnam->get(QNetworkRequest(url));
        connect(m_loadReply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
        return;
    }

    if (m_loadReply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        m_currentTrailers.append(parseTrailers(msg));
        int pos=0;
        QRegExp rx("<meta itemprop=\"url\" content=\"([^\"]*)\">[^<]*<meta itemprop=\"name\" content=\"([^\"]*)\">");
        rx.setMinimal(true);

        while ((pos = rx.indexIn(msg, pos)) != -1) {
            if (rx.cap(2).contains("Trailer"))
                m_trailerSites.append(rx.cap(1));
            pos += rx.matchedLength();
        }
    }
    m_loadReply->deleteLater();

    if (m_trailerSites.count() > 0) {
        m_loadReply = m_qnam->get(QNetworkRequest(m_trailerSites.at(0)));
        connect(m_loadReply, SIGNAL(finished()), this, SLOT(onSubLoadFinished()));
    } else {
        loadPreviewImages();
    }
}

void MovieMaze::onSubLoadFinished()
{
    if (m_loadReply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromLatin1(m_loadReply->readAll());
        m_currentTrailers.append(parseTrailers(msg));
    }
    m_loadReply->deleteLater();
    m_trailerSites.takeFirst();

    if (m_trailerSites.count() > 0) {
        m_loadReply = m_qnam->get(QNetworkRequest(m_trailerSites.at(0)));
        connect(m_loadReply, SIGNAL(finished()), this, SLOT(onSubLoadFinished()));
    } else {
        loadPreviewImages();
    }
}

QList<TrailerResult> MovieMaze::parseTrailers(QString html)
{
    QList<TrailerResult> results;
    QString name;
    int pos=0;
    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern("<h1 itemprop=\"name\" class=\"headline\">([^<]*)</h1>");
    if (rx.indexIn(html, 0) != -1)
        name = rx.cap(1);

    rx.setPattern("<a onclick=\"[^\"]*\" class=\"[^\"]*\" data-title=\"[^\"]*\" data-playlist=\"[0-9]*\" data-clip=\"[0-9]*\" "
                  "data-clip-id=\"[0-9]*\" data-width=\"[0-9]*\" data-height=\"[0-9]*\" data-image=\"([^\"]*)\" data-length=\"[^\"]*\" data-embedurl=\"[^\"]*\" data-downloadurl=\"([^\"]*)\">[^<]*"
                  "<img src=\"/assets/images/icon-flag-(.*).png\" alt=\"\">[^<]*</a>");
    while ((pos = rx.indexIn(html, pos)) != -1) {
        TrailerResult result;
        result.name = name;
        result.preview = "http://www.moviemaze.de" + rx.cap(1);
        result.language = rx.cap(3);
        if (result.language == "de")
            result.language = tr("German");
        else if (result.language == "us")
            result.language = tr("English");
        result.trailerUrl = rx.cap(2);
        result.previewImageLoaded = false;
        results.append(result);
        pos += rx.matchedLength();
    }
    return results;
}

void MovieMaze::loadPreviewImages()
{
    for (int i=0, n=m_currentTrailers.size() ; i<n ; ++i) {
        if (!m_currentTrailers[i].previewImageLoaded) {
            m_currentPreviewLoad = i;
            m_previewLoadReply = m_qnam->get(QNetworkRequest(m_currentTrailers[i].preview));
            connect(m_previewLoadReply, SIGNAL(finished()), this, SLOT(onLoadPreviewImageFinished()));
            return;
        }
    }

    emit sigLoadDone(m_currentTrailers);
}

void MovieMaze::onLoadPreviewImageFinished()
{
    if (m_previewLoadReply->error() == QNetworkReply::NoError) {
        QImage img;
        img.loadFromData(m_previewLoadReply->readAll());
        m_currentTrailers[m_currentPreviewLoad].previewImage = img;
    }
    m_currentTrailers[m_currentPreviewLoad].previewImageLoaded = true;
    m_previewLoadReply->deleteLater();
    loadPreviewImages();
}
