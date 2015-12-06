#include "AdultDvdEmpire.h"

#include <QDebug>
#include <QRegExp>
#include <QTextDocument>
#include "data/Storage.h"
#include "globals/NetworkReplyWatcher.h"
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
                      << MovieScraperInfos::Set
                      << MovieScraperInfos::Rating
                      << MovieScraperInfos::Director;
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
    new NetworkReplyWatcher(this, reply);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
}

void AdultDvdEmpire::onSearchFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
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
    QRegExp rx("<a href=\"([^\"]*)\"[\\s\\n]*title=\"([^\"]*)\" Category \"List Page\" Label=\"Title\">");
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
    QUrl url(QString("http://www.adultdvdempire.com%1").arg(ids.values().first()));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    reply->setProperty("id", ids.values().first());
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

void AdultDvdEmpire::onLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, reply->property("infosToLoad").value<Storage*>()->infosToLoad());
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }

    movie->controller()->scraperLoadDone(this);
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

    rx.setPattern("<small>Length: </small> ([0-9]*) hrs. ([0-9]*) mins.</li>");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1)
        movie->setRuntime(rx.cap(1).toInt()*60 + rx.cap(2).toInt());

    rx.setPattern("<li><small>Production Year:</small> ([0-9]{4})</li>");
    if (infos.contains(MovieScraperInfos::Released) && rx.indexIn(html) != -1)
        movie->setReleased(QDate::fromString(rx.cap(1), "yyyy"));

    rx.setPattern("<li><small>Studio: </small><a href=\"[^\"]*\"[\\s\\n]*Category=\"Item Page\"[\\s\\n]*Label=\"Studio - Details\">(.*)</a>");
    if (infos.contains(MovieScraperInfos::Studios) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1));
        movie->addStudio(doc.toPlainText().trimmed());
    }

    if (infos.contains(MovieScraperInfos::Actors)) {
        int offset = 0;
        rx.setPattern("<li><a href=\"[^\"]*\"[\\s\\n]*Category=\"Item Page\"[\\s\\n]*Label=\"Performer\"><img src=\"[^\"]*\"[\\s\\n]*alt=\"[^\"]*\" title=\"[^\"]*\"[\\s\\n]*class=\"img-responsive headshot\"[\\s\\n]*style=\"background-image:url\\(([^\\)]*)\\);\" /><span>(.*)</span></a></li>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            Actor a;
            a.name = rx.cap(2);
            a.thumb = rx.cap(1);
            movie->addActor(a);
        }
    }

    rx.setPattern("<a href=\"[^\"]*\"[\\s\\n]*Category=\"Item Page\"[\\s\\n]*Label=\"Director\"><img src=\"[^\"]*\"[\\s\\n]*alt=\"[^\"]*\" title=\"[^\"]*\"[\\s\\n]*class=\"img-responsive headshot\"[\\s\\n]*style=\"[^\"]*\" />[\\s\\n]*(.*)<br /><small>Director</small></a>");
    if (infos.contains(MovieScraperInfos::Director) && rx.indexIn(html) != -1)
        movie->setDirector(rx.cap(1).trimmed());

    if (infos.contains(MovieScraperInfos::Genres)) {
        rx.setPattern("<li><a href=\"[^\"]*\"[\\s\\n]*Category=\"Item Page\" Label=\"Category\" />[\\s\\n]*(.*)[\\s\\n]*</a></li>");
        int offset = 0;
        while ((offset = rx.indexIn(html, offset)) != -1) {
            movie->addGenre(rx.cap(1).trimmed());
            offset += rx.matchedLength();
        }
    }

    rx.setPattern("<h4 class=\"spacing-bottom text-dark synopsis\">(.*)</h4>");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1).trimmed());
        movie->setOverview(doc.toPlainText());
        if (Settings::instance()->usePlotForOutline())
            movie->setOutline(doc.toPlainText());
    }

    rx.setPattern("<a href=\"([^\"]*)\" id=\"front-cover\"");
    if (infos.contains(MovieScraperInfos::Poster) && rx.indexIn(html) != -1) {
        Poster p;
        p.thumbUrl = rx.cap(1);
        p.originalUrl = rx.cap(1);
        movie->addPoster(p);
    }

    rx.setPattern("<a href=\"[^\"]*\"[\\s\\n]*Category=\"Item Page\" Label=\"Series\"[\\s\\n]*class=\"\">[\\s\\n]*(.*)<");
    if (infos.contains(MovieScraperInfos::Set) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1));
        QString set = doc.toPlainText().trimmed();
        if (set.endsWith("Series", Qt::CaseInsensitive))
            set.chop(6);
        set = set.trimmed();
        if (set.startsWith("\""))
            set.remove(0, 1);
        if (set.endsWith("\""))
            set.chop(1);
        movie->setSet(set.trimmed());
    }

    rx.setPattern("Average Rating (.*) <small>out of (\\d+)</small>");
    if (infos.contains(MovieScraperInfos::Rating) && rx.indexIn(html) != -1) {
        movie->setRating(rx.cap(1).toFloat());
        movie->setVotes(rx.cap(2).toInt());
    }

    if (infos.contains(MovieScraperInfos::Backdrop)) {
        rx.setPattern("<a rel=\"screenshots\"[\\s\\n]*href=\"([^\"]*)\"");
        int offset = 0;
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            Poster p;
            p.thumbUrl = rx.cap(1);
            p.originalUrl = rx.cap(1);
            movie->addBackdrop(p);
        }
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
