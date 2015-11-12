#include "AEBN.h"

#include <QDebug>
#include <QGridLayout>
#include <QRegExp>
#include "data/Storage.h"
#include "globals/NetworkReplyWatcher.h"
#include "main/MainWindow.h"

AEBN::AEBN(QObject *parent)
{
    setParent(parent);
    m_language = "en";

    m_widget = new QWidget(MainWindow::instance());
    m_box = new QComboBox(m_widget);
    m_box->addItem(tr("Bulgarian"), "bg");
    m_box->addItem(tr("Chinese"), "zh");
    m_box->addItem(tr("Croatian"), "hr");
    m_box->addItem(tr("Czech"), "cs");
    m_box->addItem(tr("Danish"), "da");
    m_box->addItem(tr("Dutch"), "nl");
    m_box->addItem(tr("English"), "en");
    m_box->addItem(tr("Finnish"), "fi");
    m_box->addItem(tr("French"), "fr");
    m_box->addItem(tr("German"), "de");
    m_box->addItem(tr("Greek"), "el");
    m_box->addItem(tr("Hebrew"), "he");
    m_box->addItem(tr("Hungarian"), "hu");
    m_box->addItem(tr("Italian"), "it");
    m_box->addItem(tr("Japanese"), "ja");
    m_box->addItem(tr("Korean"), "ko");
    m_box->addItem(tr("Norwegian"), "no");
    m_box->addItem(tr("Polish"), "pl");
    m_box->addItem(tr("Portuguese"), "pt");
    m_box->addItem(tr("Russian"), "ru");
    m_box->addItem(tr("Slovene"), "sl");
    m_box->addItem(tr("Spanish"), "es");
    m_box->addItem(tr("Swedish"), "sv");
    m_box->addItem(tr("Turkish"), "tr");
    QGridLayout *layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);

    m_scraperSupports << MovieScraperInfos::Title
                      << MovieScraperInfos::Released
                      << MovieScraperInfos::Runtime
                      << MovieScraperInfos::Overview
                      << MovieScraperInfos::Poster
                      << MovieScraperInfos::Actors
                      << MovieScraperInfos::Genres
                      << MovieScraperInfos::Studios
                      << MovieScraperInfos::Director
                      << MovieScraperInfos::Set
                      << MovieScraperInfos::Tags;
}

QString AEBN::name()
{
    return QString("AEBN");
}

QString AEBN::identifier()
{
    return QString("aebn");
}

bool AEBN::isAdult()
{
    return true;
}

QList<int> AEBN::scraperSupports()
{
    return m_scraperSupports;
}

QList<int> AEBN::scraperNativelySupports()
{
    return m_scraperSupports;
}

QNetworkAccessManager *AEBN::qnam()
{
    return &m_qnam;
}

void AEBN::search(QString searchStr)
{
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);
    QUrl url(QString("http://straight.theater.aebn.net/dispatcher/fts?userQuery=%2&targetSearchMode=basic&locale=%1&searchType=movie&sortType=Relevance&imageType=Large&theaterId=822&genreId=101")
             .arg(m_language)
             .arg(encodedSearch));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
}

void AEBN::onSearchFinished()
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

QList<ScraperSearchResult> AEBN::parseSearch(QString html)
{
    QList<ScraperSearchResult> results;
    int offset = 0;
    QRegExp rx("<a id=\"FTSMovieSearch_link_image_detail_[0-9]+\" href=\"/dispatcher/movieDetail\\?movieId=([0-9]+)([^\"]*)\" title=\"([^\"]*)\"><img src=\"([^\"]*)\" alt=\"([^\"]*)\" /></a>");
    rx.setMinimal(true);
    while ((offset = rx.indexIn(html, offset)) != -1) {
        ScraperSearchResult result;
        result.id = rx.cap(1);
        result.name = rx.cap(3);
        results << result;
        offset += rx.matchedLength();
    }

    return results;
}

void AEBN::loadData(QMap<ScraperInterface*, QString> ids, Movie *movie, QList<int> infos)
{
    movie->clear(infos);

    QUrl url(QString("http://straight.theater.aebn.net/dispatcher/movieDetail?movieId=%1&locale=%2&theaterId=822&genreId=101").arg(ids.values().first()).arg(m_language));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

void AEBN::onLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        QStringList actorIds;
        parseAndAssignInfos(msg, movie, reply->property("infosToLoad").value<Storage*>()->infosToLoad(), actorIds);
        if (!actorIds.isEmpty()) {
            downloadActors(movie, actorIds);
            return;
        }
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }
    movie->controller()->scraperLoadDone(this);
}

void AEBN::parseAndAssignInfos(QString html, Movie *movie, QList<int> infos, QStringList &actorIds)
{
    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern("<h1 itemprop=\"name\"  class=\"md-movieTitle\"  >(.*)</h1>");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1)
        movie->setName(rx.cap(1));

    rx.setPattern("<span class=\"runTime\"><span itemprop=\"duration\" content=\"([^\"]*)\">([0-9]+)</span>");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1)
        movie->setRuntime(rx.cap(2).toInt());

    rx.setPattern("<span class=\"detailsLink\" itemprop=\"datePublished\" content=\"([0-9]{4})(.*)\">");
    if (infos.contains(MovieScraperInfos::Released) && rx.indexIn(html) != -1)
        movie->setReleased(QDate::fromString(rx.cap(1), "yyyy"));

    rx.setPattern("<span itemprop=\"about\">(.*)</span>");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        movie->setOverview(rx.cap(1));
        if (Settings::instance()->usePlotForOutline())
            movie->setOutline(rx.cap(1));
    }

    rx.setPattern("<div id=\"md-boxCover\"><a href=\"([^\"]*)\" target=\"_blank\" onclick=\"([^\"]*)\"><img itemprop=\"thumbnailUrl\" src=\"([^\"]*)\" alt=\"([^\"]*)\" name=\"boxImage\" id=\"boxImage\" /></a>");
    if (infos.contains(MovieScraperInfos::Poster) && rx.indexIn(html) != -1) {
        Poster p;
        p.thumbUrl = rx.cap(3);
        p.originalUrl = rx.cap(1);
        movie->addPoster(p);
    }

    rx.setPattern("<span class=\"detailsLink\"><a href=\"([^\"]*)\" class=\"series\">(.*)</a>");
    if (infos.contains(MovieScraperInfos::Set) && rx.indexIn(html) != -1)
        movie->setSet(rx.cap(2));

    rx.setPattern("<span class=\"detailsLink\" itemprop=\"director\" itemscope itemtype=\"http://schema.org/Person\">(.*)<a href=\"(.*)\" itemprop=\"name\">(.*)</a>");
    if (infos.contains(MovieScraperInfos::Director) && rx.indexIn(html) != -1)
        movie->setDirector(rx.cap(3));

    rx.setPattern("<a href=\"(.*)\" itemprop=\"productionCompany\">(.*)</a>");
    if (infos.contains(MovieScraperInfos::Studios) && rx.indexIn(html) != -1)
        movie->addStudio(rx.cap(2));

    if (infos.contains(MovieScraperInfos::Genres)) {
        int offset = 0;
        rx.setPattern("<a href=\"(.*)\"(.*) itemprop=\"genre\">(.*)</a>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            movie->addGenre(rx.cap(3));
            offset += rx.matchedLength();
        }
    }

    if (infos.contains(MovieScraperInfos::Tags)) {
        int offset = 0;
        rx.setPattern("<a href=\"(.*)sexActs=[0-9]*(.*)\" (.*)>(.*)</a>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            movie->addTag(rx.cap(4));
            offset += rx.matchedLength();
        }
        offset = 0;
        rx.setPattern("<a href=\"(.*)positions=[0-9]*(.*)\" (.*)>(.*)</a>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            movie->addTag(rx.cap(4));
            offset += rx.matchedLength();
        }
    }

    if (infos.contains(MovieScraperInfos::Actors)) {
        int offset = 0;
        rx.setPattern("<a href=\"/dispatcher/starDetail\\?(.*)starId=([0-9]*)&amp;(.*)\"  class=\"linkWithPopup\" onmouseover=\"(.*)\" onmouseout=\"killPopUp\\(\\)\"   itemprop=\"actor\" itemscope itemtype=\"http://schema.org/Person\"><span itemprop=\"name\">(.*)</span></a>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            bool skip = false;
            foreach (Actor a, movie->actors()) {
                if (a.name == rx.cap(5)) {
                    skip = true;
                    break;
                }
            }
            if (skip)
                continue;
            Actor a;
            a.name = rx.cap(5);
            a.id = rx.cap(2);
            movie->addActor(a);
            if (Settings::instance()->downloadActorImages() && !actorIds.contains(rx.cap(2)))
                actorIds.append(rx.cap(2));
        }

        offset = 0;
        rx.setPattern("<a href=\"([^\"]*)\"   itemprop=\"actor\" itemscope itemtype=\"http://schema.org/Person\"><span itemprop=\"name\">(.*)</span></a>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            bool skip = false;
            foreach (Actor a, movie->actors()) {
                if (a.name == rx.cap(2)) {
                    skip = true;
                    break;
                }
            }
            if (skip)
                continue;

            Actor a;
            a.name = rx.cap(2);
            movie->addActor(a);
        }
    }
}

void AEBN::downloadActors(Movie *movie, QStringList actorIds)
{
    if (actorIds.isEmpty()) {
        movie->controller()->scraperLoadDone(this);
        return;
    }

    QString id = actorIds.takeFirst();
    QUrl url(QString("http://straight.theater.aebn.net/dispatcher/starDetail?locale=%2&starId=%1&theaterId=822&genreId=101").arg(id).arg(m_language));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("actorIds", actorIds);
    reply->setProperty("actorId", id);
    connect(reply, SIGNAL(finished()), this, SLOT(onActorLoadFinished()));
}

void AEBN::onActorLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QStringList actorIds = reply->property("actorIds").toStringList();
    QString actorId = reply->property("actorId").toString();
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignActor(msg, movie, actorId);
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }
    downloadActors(movie, actorIds);
}

void AEBN::parseAndAssignActor(QString html, Movie *movie, QString id)
{
    QRegExp rx("<img itemprop=\"image\" src=\"([^\"]*)\" alt=\"([^\"]*)\" class=\"star\" />");
    rx.setMinimal(true);
    if (rx.indexIn(html) != -1) {
        foreach (Actor *a, movie->actorsPointer()) {
            if (a->id == id)
                a->thumb = rx.cap(1);
        }
    }
}

bool AEBN::hasSettings()
{
    return true;
}

void AEBN::loadSettings(QSettings &settings)
{
    m_language = settings.value("Scrapers/AEBN/Language", "en").toString();
    for (int i=0, n=m_box->count() ; i<n ; ++i) {
        if (m_box->itemData(i).toString() == m_language)
            m_box->setCurrentIndex(i);
    }
}

void AEBN::saveSettings(QSettings &settings)
{
    m_language = m_box->itemData(m_box->currentIndex()).toString();
    settings.setValue("Scrapers/AEBN/Language", m_language);
}

QWidget *AEBN::settingsWidget()
{
    return m_widget;
}
