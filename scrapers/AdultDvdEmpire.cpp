#include "AdultDvdEmpire.h"

#include <QDebug>
#include <QRegExp>
#include <QTextDocument>
#include "data/Storage.h"
#include "settings/Settings.h"

AdultDvdEmpire::AdultDvdEmpire(QObject *parent)
{
    setParent(parent);

    m_scraperSupports << MovieScraperInfos::Title
                      << MovieScraperInfos::Released
                      << MovieScraperInfos::Runtime
                      << MovieScraperInfos::Overview
                      << MovieScraperInfos::Poster
                      << MovieScraperInfos::Actors
                      << MovieScraperInfos::Genres
                      << MovieScraperInfos::Studios
                      << MovieScraperInfos::Backdrop
                      << MovieScraperInfos::Set;
}

QString AdultDvdEmpire::name()
{
    return QString("Adult DVD Empire");
}

QString AdultDvdEmpire::identifier()
{
    return QString("adult-dvd-empire");
}

bool AdultDvdEmpire::isAdult()
{
    return true;
}

QList<int> AdultDvdEmpire::scraperSupports()
{
    return m_scraperSupports;
}

QList<int> AdultDvdEmpire::scraperNativelySupports()
{
    return m_scraperSupports;
}

QNetworkAccessManager *AdultDvdEmpire::qnam()
{
    return &m_qnam;
}

void AdultDvdEmpire::search(QString searchStr)
{
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);
    QUrl url(QString("http://www.adultdvdempire.com/dvd/search?q=%1").arg(encodedSearch));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
}

void AdultDvdEmpire::onSearchFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError ) {
        qWarning() << "Network Error" << reply->errorString();
        emit searchDone(QList<ScraperSearchResult>());
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    emit searchDone(parseSearch(msg));
}

QList<ScraperSearchResult> AdultDvdEmpire::parseSearch(QString html)
{
    QTextDocument doc;
    QList<ScraperSearchResult> results;
    int offset = 0;
    QRegExp rx("<p class=\"title\"><a href=\"/([^\"]*)\" title=\"([^\"]*)\"");
    rx.setMinimal(true);
    while ((offset = rx.indexIn(html, offset)) != -1) {
        doc.setHtml(rx.cap(2).trimmed());
        ScraperSearchResult result;
        result.id = rx.cap(1);
        result.name = doc.toPlainText();
        results << result;
        offset += rx.matchedLength();
    }

    return results;
}

void AdultDvdEmpire::loadData(QMap<ScraperInterface*, QString> ids, Movie *movie, QList<int> infos)
{
    movie->clear(infos);
    QUrl url(QString("http://www.adultdvdempire.com/%1").arg(ids.values().first()));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    reply->setProperty("id", ids.values().first());
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

void AdultDvdEmpire::onLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QString id = reply->property("id").toString();
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, reply->property("infosToLoad").value<Storage*>()->infosToLoad());
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }

    if (!reply->property("infosToLoad").value<Storage*>()->infosToLoad().contains(MovieScraperInfos::Backdrop)) {
        movie->controller()->scraperLoadDone(this);
        return;
    }

    QUrl url(QString("http://www.adultdvdempire.com/scenes/%1").arg(id));
    QNetworkReply *scenesReply = qnam()->get(QNetworkRequest(url));
    scenesReply->setProperty("storage", Storage::toVariant(scenesReply, movie));
    connect(scenesReply, SIGNAL(finished()), this, SLOT(onLoadScenesFinished()));
}

void AdultDvdEmpire::parseAndAssignInfos(QString html, Movie *movie, QList<int> infos)
{
    QTextDocument doc;
    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern("<h1>(.*)</h1>");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1).trimmed());
        movie->setName(doc.toPlainText());
    }

    rx.setPattern("<strong>Length</strong> ([0-9]*) hrs. ([0-9]*) mins.<br/>");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1)
        movie->setRuntime(rx.cap(1).toInt()*60 + rx.cap(2).toInt());

    rx.setPattern("<strong>Production Year</strong> ([0-9]{4})<br/>");
    if (infos.contains(MovieScraperInfos::Released) && rx.indexIn(html) != -1)
        movie->setReleased(QDate::fromString(rx.cap(1), "yyyy"));

    rx.setPattern("</p><p>(.*)</p></div");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1).trimmed());
        movie->setOverview(doc.toPlainText());
        if (Settings::instance()->usePlotForOutline())
            movie->setOutline(doc.toPlainText());
    }

    rx.setPattern("<div id=\"Boxcover\"><a href=\"([^\"]*)\"");
    if (infos.contains(MovieScraperInfos::Poster) && rx.indexIn(html) != -1) {
        Poster p;
        p.thumbUrl = rx.cap(1);
        p.originalUrl = rx.cap(1);
        movie->addPoster(p);
    }

    rx.setPattern("Studio</strong>[^>]*>([^<]*)</a>");
    if (infos.contains(MovieScraperInfos::Studios) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1));
        movie->addStudio(doc.toPlainText());
    }

    rx.setPattern("<p><a href=\"[^\"]*/series/[^\"]*\">([^<]*)</a></p>");
    if (infos.contains(MovieScraperInfos::Set) && rx.indexIn(html) != -1) {
        QString set = rx.cap(1);
        if (set.endsWith("Series", Qt::CaseInsensitive))
            set.chop(6);
        doc.setHtml(set.trimmed());
        movie->setSet(doc.toPlainText());
    }

    if (infos.contains(MovieScraperInfos::Genres)) {
        rx.setPattern("Categories</h2><p>(.*)</p>");
        if (rx.indexIn(html) != -1) {
            QRegExp rx2("<a href=\".*\">(.*)</a>");
            rx2.setMinimal(true);
            QString categories = rx.cap(1);
            int offset = 0;
            while ((offset = rx2.indexIn(categories, offset)) != -1) {
                doc.setHtml(rx2.cap(1));
                movie->addGenre(doc.toPlainText());
                offset += rx2.matchedLength();
            }
        }
    }

    if (infos.contains(MovieScraperInfos::Actors)) {
        int offset = 0;
        rx.setPattern("Cast</h2><ul class=\"cast listgrid listgrid3\">(.*)</ul>");
        if (rx.indexIn(html) != -1) {
            QString cast = rx.cap(1);
            QRegExp rx2("<a href=\".*\"><img src='(.*)' alt='.*' title='.*' /><br /><span>(.*)</span></a>");
            rx2.setMinimal(true);
            while ((offset = rx2.indexIn(cast, offset)) != -1) {
                offset += rx2.matchedLength();
                doc.setHtml(rx2.cap(2));
                Actor a;
                a.name = doc.toPlainText();
                a.thumb = rx2.cap(1);
                movie->addActor(a);
            }
        }
    }
}

void AdultDvdEmpire::onLoadScenesFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignScenes(msg, movie);
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }

    movie->controller()->scraperLoadDone(this);
}

void AdultDvdEmpire::parseAndAssignScenes(QString html, Movie *movie)
{
    QRegExp rx("<a rel=\"screenshots\" href=\"([^\"]*)\" class=\"fancy\" id=\"fancy\" [^>]*><img src=\"([^\"]*)\" alt");
    rx.setMinimal(true);
    int offset = 0;
    while ((offset = rx.indexIn(html, offset)) != -1) {
        offset += rx.matchedLength();
        Poster p;
        p.thumbUrl = rx.cap(2);
        p.originalUrl = rx.cap(1);
        movie->addBackdrop(p);
    }
}

bool AdultDvdEmpire::hasSettings()
{
    return false;
}

void AdultDvdEmpire::loadSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

void AdultDvdEmpire::saveSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

QWidget *AdultDvdEmpire::settingsWidget()
{
    return 0;
}
