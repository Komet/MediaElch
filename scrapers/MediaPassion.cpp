#include "MediaPassion.h"

#include <QGridLayout>
#include <QLabel>

MediaPassion::MediaPassion(QObject *parent)
{
    setParent(parent);

    m_baseUrl = "http://passion-xbmc.org/scraper/API/1";

    m_usernameEdit = new QLineEdit();
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    m_widget = new QWidget();
    QGridLayout *layout = new QGridLayout();
    layout->addWidget(new QLabel(tr("Username")), 0, 0);
    layout->addWidget(m_usernameEdit, 0, 1);
    layout->addWidget(new QLabel(tr("Password")), 1, 0);
    layout->addWidget(m_passwordEdit, 1, 1);
    m_widget->setLayout(layout);

    m_scraperSupports << MovieScraperInfos::Title
                      << MovieScraperInfos::Tagline
                      << MovieScraperInfos::Rating
                      << MovieScraperInfos::Released
                      << MovieScraperInfos::Runtime
                      << MovieScraperInfos::Certification
                      << MovieScraperInfos::Trailer
                      << MovieScraperInfos::Overview
                      << MovieScraperInfos::Poster
                      << MovieScraperInfos::Backdrop
                      << MovieScraperInfos::Actors
                      << MovieScraperInfos::Genres
                      << MovieScraperInfos::Studios
                      << MovieScraperInfos::Countries
                      << MovieScraperInfos::Director
                      << MovieScraperInfos::Writer
                      << MovieScraperInfos::Logo
                      << MovieScraperInfos::Banner
                      << MovieScraperInfos::Thumb
                      << MovieScraperInfos::CdArt
                      << MovieScraperInfos::ClearArt
                      << MovieScraperInfos::Set;

    m_scraperNativelySupports << MovieScraperInfos::Title
                              << MovieScraperInfos::Tagline
                              << MovieScraperInfos::Rating
                              << MovieScraperInfos::Released
                              << MovieScraperInfos::Runtime
                              << MovieScraperInfos::Certification
                              << MovieScraperInfos::Trailer
                              << MovieScraperInfos::Overview
                              << MovieScraperInfos::Poster
                              << MovieScraperInfos::Backdrop
                              << MovieScraperInfos::Actors
                              << MovieScraperInfos::Genres
                              << MovieScraperInfos::Studios
                              << MovieScraperInfos::Countries
                              << MovieScraperInfos::Director
                              << MovieScraperInfos::Writer
                              << MovieScraperInfos::Set;
}

QString MediaPassion::apiKey()
{
    return "3d35bad89eb07d05351f09cc4789b951";
}

QString MediaPassion::name()
{
    return QString("Media Passion");
}

QString MediaPassion::identifier()
{
    return QString("media-passion");
}

QNetworkAccessManager *MediaPassion::qnam()
{
    return &m_qnam;
}

void MediaPassion::search(QString searchStr)
{
    searchStr = QUrl::toPercentEncoding(searchStr);
    QRegExp rx("^tt\\d+$");
    QString query = (rx.exactMatch(searchStr)) ? "IMDB" : "Title";
    QUrl url(QString("%1/Movie.Search/%2/%3/%4/fr/XML/%5/%6").arg(m_baseUrl)
                                                             .arg(m_username)
                                                             .arg(m_password)
                                                             .arg(query)
                                                             .arg(MediaPassion::apiKey())
                                                             .arg(searchStr));
    QNetworkRequest request(url);
    QNetworkReply *reply = qnam()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
}

void MediaPassion::onSearchFinished()
{
    qDebug() << "SEARCH FINISHED";

    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    qDebug() << reply->readAll();

    if (reply->error() != QNetworkReply::NoError ) {
        qWarning() << "Network Error" << reply->errorString();
        reply->deleteLater();
        emit searchDone(QList<ScraperSearchResult>());
        return;
    }

    QList<ScraperSearchResult> results;
    emit searchDone(results);
}

void MediaPassion::loadData(QMap<ScraperInterface*, QString> ids, Movie *movie, QList<int> infos)
{

}

bool MediaPassion::hasSettings()
{
    return true;
}

void MediaPassion::loadSettings(QSettings &settings)
{
    m_username = settings.value("Scrapers/MediaPassion/Username", "").toString();
    m_password = settings.value("Scrapers/MediaPassion/Password", "").toString();
    m_usernameEdit->setText(m_username);
    m_passwordEdit->setText(m_password);
}

void MediaPassion::saveSettings(QSettings &settings)
{
    m_username = m_usernameEdit->text();
    m_password = m_passwordEdit->text();
    settings.setValue("Scrapers/MediaPassion/Username", m_username);
    settings.setValue("Scrapers/MediaPassion/Password", m_password);
}

QList<int> MediaPassion::scraperSupports()
{
    return m_scraperSupports;
}

QList<int> MediaPassion::scraperNativelySupports()
{
    return m_scraperNativelySupports;
}

QWidget *MediaPassion::settingsWidget()
{
    return m_widget;
}
