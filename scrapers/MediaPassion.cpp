#include "MediaPassion.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSpacerItem>
#include "data/Storage.h"
#include "globals/Manager.h"
#include "globals/Helper.h"
#include "globals/NetworkReplyWatcher.h"
#include "main/MainWindow.h"

QMap<QUrl, QString> MediaPassion::m_contentCache;

MediaPassion::MediaPassion(QObject *parent)
{
    setParent(parent);

    m_baseUrl = "http://scraper.media-passion.fr/API/1";

    m_widget = new QWidget(MainWindow::instance());

    m_usernameEdit = new QLineEdit(m_widget);
    m_passwordEdit = new QLineEdit(m_widget);
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    m_languageCombo = new QComboBox(m_widget);
    m_languageCombo->addItem(tr("English"), "en");
    m_languageCombo->addItem(tr("French"), "fr");

    m_ratingCombo = new QComboBox(m_widget);
    m_ratingCombo->addItem(tr("Allocine"), "allocine");
    m_ratingCombo->addItem(tr("IMDB"), "imdb");
    m_ratingCombo->addItem(tr("Cine Passion"), "cinepassion");

    m_certificationCombo = new QComboBox(m_widget);
    m_certificationCombo->addItem(tr("Argentina"), "Argentina");
    m_certificationCombo->addItem(tr("Australia"), "Australia");
    m_certificationCombo->addItem(tr("Belgium"), "Belgium");
    m_certificationCombo->addItem(tr("Brazil"), "Brazil");
    m_certificationCombo->addItem(tr("Canada"), "Canada");
    m_certificationCombo->addItem(tr("Chile"), "Chile");
    m_certificationCombo->addItem(tr("Finland"), "Finland");
    m_certificationCombo->addItem(tr("France"), "France");
    m_certificationCombo->addItem(tr("Germany"), "Germany");
    m_certificationCombo->addItem(tr("Hong Kong"), "Hong Kong");
    m_certificationCombo->addItem(tr("Iceland"), "Iceland");
    m_certificationCombo->addItem(tr("India"), "India");
    m_certificationCombo->addItem(tr("Ireland"), "Ireland");
    m_certificationCombo->addItem(tr("Israel"), "Israel");
    m_certificationCombo->addItem(tr("Italy"), "Italy");
    m_certificationCombo->addItem(tr("Japan"), "Japan");
    m_certificationCombo->addItem(tr("Malaysia"), "Malaysia");
    m_certificationCombo->addItem(tr("Netherlands"), "Netherlands");
    m_certificationCombo->addItem(tr("New Zealand"), "New Zealand");
    m_certificationCombo->addItem(tr("Norway"), "Norway");
    m_certificationCombo->addItem(tr("Peru"), "Peru");
    m_certificationCombo->addItem(tr("Philippines"), "Philippines");
    m_certificationCombo->addItem(tr("Portugal"), "Portugal");
    m_certificationCombo->addItem(tr("Singapore"), "Singapore");
    m_certificationCombo->addItem(tr("South Africa"), "South Africa");
    m_certificationCombo->addItem(tr("South Korea"), "South Korea");
    m_certificationCombo->addItem(tr("Spain"), "Spain");
    m_certificationCombo->addItem(tr("Sweden"), "Sweden");
    m_certificationCombo->addItem(tr("Switzerland"), "Switzerland");
    m_certificationCombo->addItem(tr("UK"), "UK");
    m_certificationCombo->addItem(tr("USA"), "USA");

    QGridLayout *layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Username")), 0, 0);
    layout->addWidget(m_usernameEdit, 0, 1);
    layout->addWidget(new QLabel(tr("Password")), 1, 0);
    layout->addWidget(m_passwordEdit, 1, 1);
    layout->addWidget(new QLabel(tr("Language")), 2, 0);
    layout->addWidget(m_languageCombo, 2, 1);
    layout->addWidget(new QLabel(tr("Rating")), 3, 0);
    layout->addWidget(m_ratingCombo, 3, 1);
    layout->addWidget(new QLabel(tr("Certification")), 4, 0);
    layout->addWidget(m_certificationCombo, 4, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
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
                              << MovieScraperInfos::Set
                              << MovieScraperInfos::CdArt
                              << MovieScraperInfos::ClearArt
                              << MovieScraperInfos::Logo;
}

QString MediaPassion::apiKey()
{
    return "9657c1fffd38824e5ab0472e022e577e";
}

QString MediaPassion::name()
{
    return QString("Media Passion");
}

QString MediaPassion::identifier()
{
    return QString("media-passion");
}

bool MediaPassion::isAdult()
{
    return false;
}

QNetworkAccessManager *MediaPassion::qnam()
{
    return &m_qnam;
}

void MediaPassion::search(QString searchStr)
{
    if (!checkUserAndPass()) {
        QMessageBox::warning(MainWindow::instance(), tr("No username and password"),
                             tr("In order to use this scraper you have to set your username and password in MediaElchs settings."));
        emit searchDone(QList<ScraperSearchResult>());
        return;
    }

    searchStr = QUrl::toPercentEncoding(searchStr);
    QRegExp rx("^tt\\d+$");
    QString query = (rx.exactMatch(searchStr)) ? "IMDB" : "Title";
    QUrl url(QString("%1/Movie.Search/%2/%3/%4/%5/XML/%6/%7").arg(m_baseUrl)
                                                             .arg(m_usernameEnc)
                                                             .arg(m_passwordEnc)
                                                             .arg(query)
                                                             .arg(m_language)
                                                             .arg(MediaPassion::apiKey())
                                                             .arg(searchStr));
    QNetworkRequest request(url);
    QNetworkReply *reply = qnam()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
}

void MediaPassion::onSearchFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << reply->errorString();
        emit searchDone(QList<ScraperSearchResult>());
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    QString errorMsg;
    if (hasError(msg, errorMsg)) {
        QMessageBox::warning(MainWindow::instance(), tr("Scraper returned an error"), tr("The scraper returned the following error: %1").arg(errorMsg));
        emit searchDone(QList<ScraperSearchResult>());
        return;
    }
    emit searchDone(parseSearch(msg));
}

void MediaPassion::loadData(QMap<ScraperInterface*, QString> ids, Movie *movie, QList<int> infos)
{
    if (!checkUserAndPass()) {
        QMessageBox::warning(MainWindow::instance(), tr("No username and password"),
                             tr("In order to use this scraper you have to set your username and password in MediaElchs settings."));
        movie->controller()->scraperLoadDone(this);
        return;
    }

    movie->clear(infos);

    QUrl url(QString("%1/Movie.GetInfo/%2/%3/ID/%4/XML/%5/%6").arg(m_baseUrl)
                                                              .arg(m_usernameEnc)
                                                              .arg(m_passwordEnc)
                                                              .arg(m_language)
                                                              .arg(MediaPassion::apiKey())
                                                              .arg(ids.values().first()));

    if (MediaPassion::m_contentCache.contains(url)) {
        parseAndAssignInfos(m_contentCache.value(url), movie, infos);
        movie->controller()->scraperLoadDone(this);
        return;
    }
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("movie", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

void MediaPassion::onLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    Movie *movie = reply->property("movie").value<Storage*>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    if (!movie)
        return;

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << reply->errorString();
        movie->controller()->scraperLoadDone(this);
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    QString errorMsg;
    if (hasError(msg, errorMsg)) {
        QMessageBox::warning(MainWindow::instance(), tr("Scraper returned an error"), tr("The scraper returned the following error: %1").arg(errorMsg));
        movie->controller()->scraperLoadDone(this);
        return;
    }
    MediaPassion::m_contentCache.insert(reply->url(), msg);

    parseAndAssignInfos(msg, movie, infos);
    movie->controller()->scraperLoadDone(this);
}

bool MediaPassion::hasSettings()
{
    return true;
}

void MediaPassion::loadSettings(QSettings &settings)
{
    m_username = settings.value("Scrapers/MediaPassion/Username", "").toString();
    m_password = settings.value("Scrapers/MediaPassion/Password", "").toString();
    m_usernameEnc = QString(m_username.toUtf8().toBase64());
    m_passwordEnc = QCryptographicHash::hash(QString("%1%2").arg(m_username.toLower()).arg(m_password).toUtf8(), QCryptographicHash::Sha1).toHex();
    m_ratingType = settings.value("Scrapers/MediaPassion/Rating", "cinepassion").toString();
    m_certificationNation = settings.value("Scrapers/MediaPassion/Certification", "France").toString();
    m_language = settings.value("Scrapers/MediaPassion/Language", "fr").toString();

    m_usernameEdit->setText(m_username);
    m_passwordEdit->setText(m_password);
    for (int i=0, n=m_ratingCombo->count() ; i<n ; ++i) {
        if (m_ratingCombo->itemData(i).toString() == m_ratingType)
            m_ratingCombo->setCurrentIndex(i);
    }
    for (int i=0, n=m_certificationCombo->count() ; i<n ; ++i) {
        if (m_certificationCombo->itemData(i).toString() == m_certificationNation)
            m_certificationCombo->setCurrentIndex(i);
    }
    for (int i=0, n=m_languageCombo->count() ; i<n ; ++i) {
        if (m_languageCombo->itemData(i).toString() == m_language)
            m_languageCombo->setCurrentIndex(i);
    }
}

void MediaPassion::saveSettings(QSettings &settings)
{
    m_username = m_usernameEdit->text();
    m_password = m_passwordEdit->text();
    m_ratingType = m_ratingCombo->itemData(m_ratingCombo->currentIndex()).toString();
    m_certificationNation = m_certificationCombo->itemData(m_certificationCombo->currentIndex()).toString();
    m_language = m_languageCombo->itemData(m_languageCombo->currentIndex()).toString();
    settings.setValue("Scrapers/MediaPassion/Username", m_username);
    settings.setValue("Scrapers/MediaPassion/Password", m_password);
    settings.setValue("Scrapers/MediaPassion/Rating", m_ratingType);
    settings.setValue("Scrapers/MediaPassion/Certification", m_certificationNation);
    settings.setValue("Scrapers/MediaPassion/Language", m_language);

    loadSettings(settings);
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

bool MediaPassion::checkUserAndPass()
{
    return (!m_username.isEmpty() && !m_password.isEmpty());
}

QList<ScraperSearchResult> MediaPassion::parseSearch(QString xml)
{
    QList<ScraperSearchResult> results;
    QDomDocument domDoc;
    domDoc.setContent(xml);

    for (int i=0, n=domDoc.elementsByTagName("movie").count() ; i<n ; ++i) {
        QDomElement entry = domDoc.elementsByTagName("movie").at(i).toElement();
        if (entry.elementsByTagName("id").size() == 0 || entry.elementsByTagName("id").at(0).toElement().text().isEmpty())
            continue;
        ScraperSearchResult result;
        result.id = entry.elementsByTagName("id").at(0).toElement().text();
        if (entry.elementsByTagName("title").size() > 0)
            result.name = entry.elementsByTagName("title").at(0).toElement().text();
        if (entry.elementsByTagName("year").size() > 0)
            result.released = QDate::fromString(entry.elementsByTagName("year").at(0).toElement().text(), "yyyy");
        results.append(result);
    }
    return results;
}

void MediaPassion::parseAndAssignInfos(QString data, Movie *movie, QList<int> infos)
{
    QXmlStreamReader xml(data);

    while (xml.readNextStartElement()) {
        if (xml.name() != "movie")
            xml.skipCurrentElement();
        else
            break;
    }

    QMap<int, Poster> posters;
    QMap<int, Poster> fanarts;
    QMap<int, Poster> discArts;
    QMap<int, Poster> clearArts;
    QMap<int, Poster> logos;

    while (xml.readNextStartElement()) {
        if (xml.name() == "id") {
            movie->setMediaPassionId(xml.readElementText());
        } else if (xml.name() == "id_imdb") {
            movie->setId(xml.readElementText());
        } else if (infos.contains(MovieScraperInfos::Title) && xml.name() == "title") {
            movie->setName(xml.readElementText());
        } else if (infos.contains(MovieScraperInfos::Title) && xml.name() == "originaltitle") {
            movie->setOriginalName(xml.readElementText());
        } else if (infos.contains(MovieScraperInfos::Released) && xml.name() == "year") {
            movie->setReleased(QDate::fromString(xml.readElementText(), "yyyy"));
        } else if (infos.contains(MovieScraperInfos::Runtime) && xml.name() == "runtime") {
            movie->setRuntime(xml.readElementText().toInt());
        } else if (infos.contains(MovieScraperInfos::Overview) && xml.name() == "plot") {
            QString plot = xml.readElementText();
            movie->setOverview(plot);
            if (Settings::instance()->usePlotForOutline())
                movie->setOutline(plot);
        } else if (infos.contains(MovieScraperInfos::Tagline) && xml.name() == "tagline") {
            movie->setTagline(xml.readElementText());
        } else if (infos.contains(MovieScraperInfos::Title) && xml.name() == "sorttitle") {
            movie->setSortTitle(xml.readElementText());
        } else if (infos.contains(MovieScraperInfos::Set) && xml.name() == "saga") {
            movie->setSet(xml.readElementText());
        } else if (infos.contains(MovieScraperInfos::Trailer) && xml.name() == "trailers") {
            bool trailerSet = false;
            while (xml.readNextStartElement()) {
                if (xml.name() == "trailer" && !trailerSet) {
                    QString trailer = xml.readElementText();
                    if (!trailer.isEmpty()) {
                        movie->setTrailer(QUrl(Helper::instance()->formatTrailerUrl(trailer)));
                        trailerSet = true;
                    }
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else if (infos.contains(MovieScraperInfos::Countries) && xml.name() == "countries") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "country") {
                    QString country = xml.readElementText();
                    if (!country.isEmpty())
                        movie->addCountry(Helper::instance()->mapCountry(country));
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else if (infos.contains(MovieScraperInfos::Genres) && xml.name() == "genres") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "genre") {
                    QString genre = xml.readElementText();
                    if (!genre.isEmpty())
                        movie->addGenre(Helper::instance()->mapGenre(genre));
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else if (infos.contains(MovieScraperInfos::Studios) && xml.name() == "studios") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "studio") {
                    QString studio = xml.readElementText();
                    if (!studio.isEmpty())
                        movie->addStudio(Helper::instance()->mapStudio(studio));
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else if (infos.contains(MovieScraperInfos::Director) && xml.name() == "directors") {
            QStringList directors;
            while (xml.readNextStartElement()) {
                if (xml.name() == "director") {
                    QString director = xml.readElementText();
                    if (!director.isEmpty())
                        directors.append(director);
                } else {
                    xml.skipCurrentElement();
                }
            }
            movie->setDirector(directors.join(", "));
        } else if (infos.contains(MovieScraperInfos::Writer) && xml.name() == "credits") {
            QStringList writers;
            while (xml.readNextStartElement()) {
                if (xml.name() == "credit") {
                    QString writer = xml.readElementText();
                    if (!writer.isEmpty())
                        writers.append(writer);
                } else {
                    xml.skipCurrentElement();
                }
            }
            movie->setWriter(writers.join(", "));
        } else if (infos.contains(MovieScraperInfos::Actors) && xml.name() == "casting") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "person") {
                    Actor actor;
                    actor.name  = xml.attributes().value("name").toString();
                    actor.role  = xml.attributes().value("character").toString();
                    actor.thumb = xml.attributes().value("thumb").toString();
                    movie->addActor(actor);
                    xml.readElementText();
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else if (infos.contains(MovieScraperInfos::Certification) && xml.name() == "certifications") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "certification" && xml.attributes().value("nation").toString() == m_certificationNation)
                    movie->setCertification(xml.readElementText());
                else
                    xml.skipCurrentElement();
            }
        } else if (infos.contains(MovieScraperInfos::Rating) && xml.name() == "ratings") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "rating" && xml.attributes().value("type").toString() == m_ratingType) {
                    movie->setVotes(xml.attributes().value("votes").toString().toInt());
                    movie->setRating(xml.readElementText().toFloat());
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else if (xml.name() == "images") {
            while (xml.readNextStartElement()) {
                if (xml.name() != "image") {
                    xml.skipCurrentElement();
                    continue;
                }

                if (infos.contains(MovieScraperInfos::Poster) && xml.attributes().value("type") == "Poster") {
                    int id = xml.attributes().value("id").toString().toInt();
                    Poster p;
                    if (posters.contains(id))
                        p = posters.value(id);
                    if (xml.attributes().value("size") == "original") {
                        p.originalUrl = xml.attributes().value("url").toString();
                        p.originalSize.setWidth(xml.attributes().value("width").toString().toInt());
                        p.originalSize.setHeight(xml.attributes().value("height").toString().toInt());
                    } else if (xml.attributes().value("size") == "preview") {
                        p.thumbUrl = xml.attributes().value("url").toString();
                    }
                    posters.insert(id, p);
                } else if (infos.contains(MovieScraperInfos::Backdrop) && xml.attributes().value("type") == "Fanart") {
                    int id = xml.attributes().value("id").toString().toInt();
                    Poster p;
                    if (fanarts.contains(id))
                        p = fanarts.value(id);
                    if (xml.attributes().value("size") == "original") {
                        p.originalUrl = xml.attributes().value("url").toString();
                        p.originalSize.setWidth(xml.attributes().value("width").toString().toInt());
                        p.originalSize.setHeight(xml.attributes().value("height").toString().toInt());
                    } else if (xml.attributes().value("size") == "preview") {
                        p.thumbUrl = xml.attributes().value("url").toString();
                    }
                    fanarts.insert(id, p);
                } else if (infos.contains(MovieScraperInfos::CdArt) && QString::compare("cdart", xml.attributes().value("type"), Qt::CaseInsensitive) == 0) {
                    int id = xml.attributes().value("id").toString().toInt();
                    Poster p;
                    if (discArts.contains(id))
                        p = discArts.value(id);
                    if (xml.attributes().value("size") == "original") {
                        p.originalUrl = xml.attributes().value("url").toString();
                        p.originalSize.setWidth(xml.attributes().value("width").toString().toInt());
                        p.originalSize.setHeight(xml.attributes().value("height").toString().toInt());
                    } else if (xml.attributes().value("size") == "preview") {
                        p.thumbUrl = xml.attributes().value("url").toString();
                    }
                    discArts.insert(id, p);
                } else if (infos.contains(MovieScraperInfos::ClearArt) && QString::compare("hdclearart", xml.attributes().value("type"), Qt::CaseInsensitive) == 0) {
                    int id = xml.attributes().value("id").toString().toInt();
                    Poster p;
                    if (clearArts.contains(id))
                        p = clearArts.value(id);
                    if (xml.attributes().value("size") == "original") {
                        p.originalUrl = xml.attributes().value("url").toString();
                        p.originalSize.setWidth(xml.attributes().value("width").toString().toInt());
                        p.originalSize.setHeight(xml.attributes().value("height").toString().toInt());
                    } else if (xml.attributes().value("size") == "preview") {
                        p.thumbUrl = xml.attributes().value("url").toString();
                    }
                    clearArts.insert(id, p);
                } else if (infos.contains(MovieScraperInfos::Logo) && QString::compare("hdlogo", xml.attributes().value("type"), Qt::CaseInsensitive) == 0) {
                    int id = xml.attributes().value("id").toString().toInt();
                    Poster p;
                    if (logos.contains(id))
                        p = logos.value(id);
                    if (xml.attributes().value("size") == "original") {
                        p.originalUrl = xml.attributes().value("url").toString();
                        p.originalSize.setWidth(xml.attributes().value("width").toString().toInt());
                        p.originalSize.setHeight(xml.attributes().value("height").toString().toInt());
                    } else if (xml.attributes().value("size") == "preview") {
                        p.thumbUrl = xml.attributes().value("url").toString();
                    }
                    logos.insert(id, p);
                }
                xml.readElementText();
            }
        } else {
            xml.skipCurrentElement();
        }
    }

    if (infos.contains(MovieScraperInfos::Poster)) {
        QMapIterator<int, Poster> it(posters);
        while (it.hasNext()) {
            it.next();
            movie->addPoster(it.value());
        }
    }
    if (infos.contains(MovieScraperInfos::Backdrop)) {
        QMapIterator<int, Poster> it(fanarts);
        while (it.hasNext()) {
            it.next();
            movie->addBackdrop(it.value());
        }
    }
    if (infos.contains(MovieScraperInfos::CdArt)) {
        QMapIterator<int, Poster> it(discArts);
        while (it.hasNext()) {
            it.next();
            movie->addDiscArt(it.value());
        }
    }
    if (infos.contains(MovieScraperInfos::ClearArt)) {
        QMapIterator<int, Poster> it(clearArts);
        while (it.hasNext()) {
            it.next();
            movie->addClearArt(it.value());
        }
    }
    if (infos.contains(MovieScraperInfos::Logo)) {
        QMapIterator<int, Poster> it(logos);
        while (it.hasNext()) {
            it.next();
            movie->addLogo(it.value());
        }
    }
}

bool MediaPassion::hasError(QString xml, QString &errorMsg)
{
    QDomDocument domDoc;
    domDoc.setContent(xml);
    int numOfErrors = domDoc.elementsByTagName("error").count();
    for (int i=0,n=numOfErrors ; i<n ; ++i) {
        if (domDoc.elementsByTagName("error").at(i).toElement().text() == "No Results") {
            numOfErrors--;
            continue;
        }
        errorMsg.append(domDoc.elementsByTagName("error").at(i).toElement().text()).append("\n");
    }
    return (numOfErrors > 0);
}
