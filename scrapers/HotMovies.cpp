#include "HotMovies.h"

#include <QDebug>
#include <QGridLayout>
#include <QRegExp>
#include "data/Storage.h"
#include "globals/Helper.h"
#include "globals/NetworkReplyWatcher.h"
#include "main/MainWindow.h"

HotMovies::HotMovies(QObject *parent)
{
    setParent(parent);
    m_scraperSupports << MovieScraperInfos::Title
                      << MovieScraperInfos::Rating
                      << MovieScraperInfos::Released
                      << MovieScraperInfos::Runtime
                      << MovieScraperInfos::Overview
                      << MovieScraperInfos::Poster
                      << MovieScraperInfos::Actors
                      << MovieScraperInfos::Genres
                      << MovieScraperInfos::Studios
                      << MovieScraperInfos::Director
                      << MovieScraperInfos::Set;
}

QString HotMovies::name()
{
    return QString("HotMovies");
}

QString HotMovies::identifier()
{
    return QString("hotmovies");
}

bool HotMovies::isAdult()
{
    return true;
}

QList<int> HotMovies::scraperSupports()
{
    return m_scraperSupports;
}

QList<int> HotMovies::scraperNativelySupports()
{
    return m_scraperSupports;
}

QNetworkAccessManager *HotMovies::qnam()
{
    return &m_qnam;
}

void HotMovies::search(QString searchStr)
{
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);
    QUrl url(QString("http://www.hotmovies.com/search.php?words=%1&search_in=video_title&num_per_page=30").arg(encodedSearch));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
}

void HotMovies::onSearchFinished()
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

QList<ScraperSearchResult> HotMovies::parseSearch(QString html)
{
    QList<ScraperSearchResult> results;
    int offset = 0;

    QRegExp rx("<tr>.*<td colspan=\"2\" class=\"td_title\">.*<h3 class=\"title\">.*<a href=\"(.*)\" title=\".*\">(.*)</a>");
    rx.setMinimal(true);
    while ((offset = rx.indexIn(html, offset)) != -1) {
        ScraperSearchResult result;
        result.id = rx.cap(1);
        result.name = rx.cap(2).trimmed();
        results << result;
        offset += rx.matchedLength();
    }

    return results;
}

void HotMovies::loadData(QMap<ScraperInterface*, QString> ids, Movie *movie, QList<int> infos)
{
    movie->clear(infos);

    QUrl url(ids.values().first());
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

void HotMovies::onLoadFinished()
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

void HotMovies::parseAndAssignInfos(QString html, Movie *movie, QList<int> infos)
{
    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern("<span itemprop=\"name\">(.*)</span>");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1)
        movie->setName(rx.cap(1));

    rx.setPattern("<meta itemprop=\"ratingValue\" content=\"(.*)\">");
    if (infos.contains(MovieScraperInfos::Rating) && rx.indexIn(html) != -1)
        movie->setRating(rx.cap(1).toFloat());

    rx.setPattern("<span class=\"rating_number \" itemprop=\"ratingCount\">(\\d+) Rating");
    if (infos.contains(MovieScraperInfos::Rating) && rx.indexIn(html) != -1)
        movie->setVotes(rx.cap(1).toInt());

    rx.setPattern("<span itemprop=\"copyrightYear\">([0-9]{4})</span>");
    if (infos.contains(MovieScraperInfos::Released) && rx.indexIn(html) != -1)
        movie->setReleased(QDate::fromString(rx.cap(1), "yyyy"));

    rx.setPattern("<span itemprop=\"duration\" datetime=\"PT[^\"]*\">(.*)</span>");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1) {
        QStringList runtime = rx.cap(1).split(":");
        if (runtime.count() == 3)
            movie->setRuntime(runtime.at(0).toInt()*60 + runtime.at(1).toInt());
        else if (runtime.count() == 2)
            movie->setRuntime(runtime.at(0).toInt());
    }

    rx.setPattern("<div class=\"video_description\" itemprop=\"description\">(.*)</div>");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        movie->setOverview(rx.cap(1).trimmed());
        if (Settings::instance()->usePlotForOutline())
            movie->setOutline(rx.cap(1).trimmed());
    }

    rx.setPattern("<img itemprop=\"image\" alt=\"[^\"]*\" id=\"cover\" src=\"([^\"]*)\"");
    if (infos.contains(MovieScraperInfos::Poster) && rx.indexIn(html) != -1) {
        Poster p;
        p.thumbUrl = rx.cap(1);
        p.originalUrl = rx.cap(1);
        movie->addPoster(p);
    }

    if (infos.contains(MovieScraperInfos::Actors)) {
        rx.setPattern("key=\"([^\"]*)\"/> <img src=\"http://imgcover-[0-9]+.hotmovies.com/vodimages/images/spacer.gif\" class=\"lg_star_image\" /> </span><a href=\"[^\"]*\".*"
                      "title=\"[^\"]*\" rel=\"tag\" itemprop=\"actor\" itemscope itemtype=\"http://schema.org/Person\"><span itemprop=\"name\">(.*)</span></a>");

        rx.setPattern("<div class=\"star_wrapper\" key=\"(.*)\"><a href=\".*\" .* title=\".*\" rel=\"tag\" itemprop=\"url\"><span itemprop=\"name\">(.*)</span></a></div>");
        int offset = 0;
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            Actor a;
            a.name = rx.cap(2);
            if (!rx.cap(1).endsWith("missing_f.gif") && !!rx.cap(1).endsWith("missing_m.gif"))
                a.thumb = rx.cap(1);
            movie->addActor(a);
        }
    }

    if (infos.contains(MovieScraperInfos::Genres)) {
        rx.setPattern("<span itemprop=\"genre\">.* -> (.*)</span>");
        int offset = 0;
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            movie->addGenre(rx.cap(1));
        }
    }

    rx.setPattern("<strong>Studio:</strong> <a itemprop=\"url\" href=\"[^\"]*\"[\\s\\n]*title=\"[^\"]*\"><span itemprop=\"name\">(.*)</span></a>");
    if (infos.contains(MovieScraperInfos::Studios) && rx.indexIn(html) != -1)
        movie->addStudio(rx.cap(1));

    rx.setPattern("<span itemprop=\"director\" itemscope itemtype=\"http://schema.org/Person\"><a itemprop=\"url\" href=\"[^\"]*\"[\\s\\n]*title=\"[^\"]*\" rel=\"tag\"><span itemprop=\"name\">(.*)</span></a>");
    if (infos.contains(MovieScraperInfos::Director) && rx.indexIn(html) != -1)
        movie->setDirector(rx.cap(1));

    rx.setPattern("<a href=\"http://www.hotmovies.com/.*[/?]series/[^\"]*\" title=\"[^\"]*\" rel=\"tag\">(.*)</a>");
    if (infos.contains(MovieScraperInfos::Set) && rx.indexIn(html) != -1)
        movie->setSet(rx.cap(1));
}

bool HotMovies::hasSettings()
{
    return false;
}

void HotMovies::loadSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

void HotMovies::saveSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

QWidget *HotMovies::settingsWidget()
{
    return 0;
}
