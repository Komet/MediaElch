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

    if (m_searchReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromLatin1(m_searchReply->readAll());
        m_searchReply->deleteLater();

        int pos = 0;
        QRegExp rx("<a href=\"/media/trailer/([^\"]*)\">([^<]*)</a>");
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
    m_loadReply = m_qnam->get(QNetworkRequest(QUrl("http://www.moviemaze.de/media/trailer/" + id)));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

void MovieMaze::onLoadFinished()
{
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromLatin1(m_loadReply->readAll());
        m_currentTrailers.append(parseTrailers(msg));

        int pos=0;
        QRegExp rx("<h5><a href=\"([^\"]*)\">Trailer (.*)</a></h5>");
        rx.setMinimal(true);
        while ((pos = rx.indexIn(msg, pos)) != -1) {
            m_trailerSites.append("http://www.moviemaze.de" + rx.cap(1));
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
    if (m_loadReply->error() == QNetworkReply::NoError ) {
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

    rx.setPattern("<div id=\"HeaderContent\">.*<h2>([^<]*)</h2>");
    if (rx.indexIn(html, 0) != -1)
        name = rx.cap(1);

    rx.setPattern("<div class=\"trailer-choose\" .* data-image=\"([^\"]*)\".*<span class=\"trailer-language ([^\"]*)\">.*<a class=\"trailer-ico download\" href=\"([^\"]*)\"><span class=\"ico\">&nbsp;</span><span class=\"text\">Download</span></a>");
    while ((pos = rx.indexIn(html, pos)) != -1) {
        TrailerResult result;
        result.name = name;
        result.preview = rx.cap(1);
        result.language = rx.cap(2);
        if (result.language == "de")
            result.language = tr("German");
        else if (result.language == "en")
            result.language = tr("English");
        result.trailerUrl = rx.cap(3);
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
    if (m_previewLoadReply->error() == QNetworkReply::NoError ) {
        QImage img;
        img.loadFromData(m_previewLoadReply->readAll());
        m_currentTrailers[m_currentPreviewLoad].previewImage = img;
    }
    m_currentTrailers[m_currentPreviewLoad].previewImageLoaded = true;
    m_previewLoadReply->deleteLater();
    loadPreviewImages();
}
