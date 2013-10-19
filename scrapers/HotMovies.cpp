#include "HotMovies.h"

#include <QDebug>
#include <QGridLayout>
#include <QRegExp>
#include "data/Storage.h"
#include "globals/Helper.h"
#include "main/MainWindow.h"

HotMovies::HotMovies(QObject *parent)
{
    setParent(parent);
    m_language = "en";

    m_widget = new QWidget(MainWindow::instance());
    m_box = new QComboBox(m_widget);
    m_box->addItem(tr("Bulgarian"), "bg");
    m_box->addItem(tr("Chinese"), "zh");
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
    m_box->addItem(tr("Norwegian"), "no");
    m_box->addItem(tr("Polish"), "pl");
    m_box->addItem(tr("Portuguese"), "pt");
    m_box->addItem(tr("Russian"), "ru");
    m_box->addItem(tr("Spanish"), "es");
    m_box->addItem(tr("Swedish"), "sv");

    QGridLayout *layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);

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
    QUrl url;
    if (m_language != "en")
        url.setUrl(QString("http://www.hotmovies.com/%1/search.php?words=%2&search_in=video_title&num_per_page=30")
                   .arg(m_language)
                   .arg(encodedSearch));
    else
        url.setUrl(QString("http://www.hotmovies.com/search.php?words=%2&search_in=video_title&num_per_page=30")
                   .arg(encodedSearch));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
}

void HotMovies::onSearchFinished()
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

QList<ScraperSearchResult> HotMovies::parseSearch(QString html)
{
    QList<ScraperSearchResult> results;
    int offset = 0;

    QRegExp rx("<tr>.*<td colspan=\"2\">.*<h3 class=\"title\">.*<a href=\"(.*)\" title=\".*\" >(.*)</a>");
    rx.setMinimal(true);
    while ((offset = rx.indexIn(html, offset)) != -1) {
        ScraperSearchResult result;
        result.id = rx.cap(1);
        result.name = rx.cap(2);
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
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

void HotMovies::onLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError ) {
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

    rx.setPattern("<span itemprop=\"name\">.*</span>.*</h1><hr/>.*<div class=\"rating\"><a href=\".*\" rel=\"nofollow\" title=\".*\"><img src=\"http://imgcover-[0-9].hotmovies.com/vodimages/images/stars-([0-9]+-[0-9]+).png\" border=\"0\" /></a><br/><span class=\"rating_number \">([0-9]+) .*</span></div>.*</div>.*<div class=\"video_info\">");
    if (infos.contains(MovieScraperInfos::Rating) && rx.indexIn(html) != -1) {
        movie->setRating(rx.cap(1).replace("-", ".").toFloat()*2);
        movie->setVotes(rx.cap(2).toInt());
    }

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

    rx.setPattern("var descfullcontent = \"([^\"]*)\"");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1)
        movie->setOverview(QString::fromUtf8(QByteArray::fromPercentEncoding(rx.cap(1).toUtf8())));

    rx.setPattern("<img alt=\"[^\"]*\" id=\"cover\" src=\"([^\"]*)\"");
    if (infos.contains(MovieScraperInfos::Poster) && rx.indexIn(html) != -1) {
        Poster p;
        p.thumbUrl = rx.cap(1);
        p.originalUrl = rx.cap(1);
        movie->addPoster(p);
    }

    if (infos.contains(MovieScraperInfos::Actors)) {
        rx.setPattern("key=\"([^\"]*)\"/> <img src=\"http://imgcover-[0-9]+.hotmovies.com/vodimages/images/spacer.gif\" class=\"lg_star_image\" /> </span><a href=\"[^\"]*\".*"
                      "title=\"[^\"]*\" rel=\"tag\" itemprop=\"actor\" itemscope itemtype=\"http://schema.org/Person\"><span itemprop=\"name\">(.*)</span></a>");
        int offset = 0;
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            Actor a;
            a.name = rx.cap(2);
            a.thumb = rx.cap(1);
            movie->addActor(a);
        }
    }

    if (infos.contains(MovieScraperInfos::Genres)) {
        rx.setPattern("\">[^>]*[\\s]+->[\\s]([^>]*)</a>");
        int offset = 0;
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            movie->addGenre(rx.cap(1));
        }
    }

    rx.setPattern("<a href=\".*\" title=\".*\" itemprop=\"productionCompany\" itemscope itemtype=\"http://schema.org/Organization\"><span itemprop=\"name\">(.*)</span></a>");
    if (infos.contains(MovieScraperInfos::Studios) && rx.indexIn(html) != -1)
        movie->addStudio(rx.cap(1));

    rx.setPattern("itemprop=\"director\" itemscope itemtype=\"http://schema.org/Person\"><span itemprop=\"name\">(.*)</span>");
    if (infos.contains(MovieScraperInfos::Director) && rx.indexIn(html) != -1)
        movie->setDirector(rx.cap(1));

    rx.setPattern("<a href=\"http://www.hotmovies.com/.*[/?]series/[^\"]*\" title=\"[^\"]*\" rel=\"tag\">(.*)</a>");
    if (infos.contains(MovieScraperInfos::Set) && rx.indexIn(html) != -1)
        movie->setSet(rx.cap(1));
}

bool HotMovies::hasSettings()
{
    return true;
}

void HotMovies::loadSettings(QSettings &settings)
{
    m_language = settings.value("Scrapers/HotMovies/Language", "en").toString();
    for (int i=0, n=m_box->count() ; i<n ; ++i) {
        if (m_box->itemData(i).toString() == m_language)
            m_box->setCurrentIndex(i);
    }
}

void HotMovies::saveSettings(QSettings &settings)
{
    m_language = m_box->itemData(m_box->currentIndex()).toString();
    settings.setValue("Scrapers/HotMovies/Language", m_language);
}

QWidget *HotMovies::settingsWidget()
{
    return m_widget;
}
