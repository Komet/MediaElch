#include "TMDb.h"

#include <QDebug>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QSpacerItem>
#include <QVBoxLayout>

TMDb::TMDb(QObject *parent)
{
    setParent(parent);
    m_apiKey = "5d832bdf69dcb884922381ab01548d5b";
    m_language = "en";

    m_settingsLanguageCombo = new QComboBox;
    m_settingsLanguageCombo->addItem(tr("Chinese"), "zh");
    m_settingsLanguageCombo->addItem(tr("Croatian"), "hr");
    m_settingsLanguageCombo->addItem(tr("Czech"), "cs");
    m_settingsLanguageCombo->addItem(tr("Danish"), "da");
    m_settingsLanguageCombo->addItem(tr("Dutch"), "nl");
    m_settingsLanguageCombo->addItem(tr("English"), "en");
    m_settingsLanguageCombo->addItem(tr("Finnish"), "fi");
    m_settingsLanguageCombo->addItem(tr("French"), "fr");
    m_settingsLanguageCombo->addItem(tr("German"), "de");
    m_settingsLanguageCombo->addItem(tr("Greek"), "el");
    m_settingsLanguageCombo->addItem(tr("Hebrew"), "he");
    m_settingsLanguageCombo->addItem(tr("Hungarian"), "hu");
    m_settingsLanguageCombo->addItem(tr("Italian"), "it");
    m_settingsLanguageCombo->addItem(tr("Japanese"), "ja");
    m_settingsLanguageCombo->addItem(tr("Korean"), "ko");
    m_settingsLanguageCombo->addItem(tr("Norwegian"), "no");
    m_settingsLanguageCombo->addItem(tr("Polish"), "pl");
    m_settingsLanguageCombo->addItem(tr("Portuguese"), "pt");
    m_settingsLanguageCombo->addItem(tr("Russian"), "ru");
    m_settingsLanguageCombo->addItem(tr("Slovene"), "sl");
    m_settingsLanguageCombo->addItem(tr("Spanish"), "es");
    m_settingsLanguageCombo->addItem(tr("Swedish"), "sv");
    m_settingsLanguageCombo->addItem(tr("Turkish"), "tr");
}

TMDb::~TMDb()
{
}

QString TMDb::name()
{
    return QString("The Movie DB");
}

bool TMDb::hasSettings()
{
    return true;
}

void TMDb::loadSettings()
{
    QSettings settings;
    m_language = settings.value("Scrapers/TMDb/Language", "en").toString();
    for (int i=0, n=m_settingsLanguageCombo->count() ; i<n ; i++) {
        if (m_settingsLanguageCombo->itemData(i).toString() == m_language)
            m_settingsLanguageCombo->setCurrentIndex(i);
    }
}

void TMDb::saveSettings()
{
    QSettings settings;
    m_language = m_settingsLanguageCombo->itemData(m_settingsLanguageCombo->currentIndex()).toString();
    settings.setValue("Scrapers/TMDb/Language", m_language);
}

QWidget *TMDb::settingsWidget()
{
    QWidget *widget = new QWidget;
    m_settingsLanguageCombo->setParent(widget);
    QLabel *label = new QLabel(tr("Language"), widget);
    QHBoxLayout *hboxLayout = new QHBoxLayout(widget);
    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
    hboxLayout->addWidget(label);
    hboxLayout->addWidget(m_settingsLanguageCombo);
    hboxLayout->addSpacerItem(spacer);
    widget->setLayout(hboxLayout);

    return widget;
}

QNetworkAccessManager *TMDb::qnam()
{
    return &m_qnam;
}

void TMDb::search(QString searchStr)
{
    QUrl url(QString("http://api.themoviedb.org/2.1/Movie.search/%1/json/%2/%3").arg(m_language).arg(m_apiKey).arg(searchStr));
    m_searchReply = this->qnam()->get(QNetworkRequest(url));
    connect(m_searchReply, SIGNAL(finished()), this, SLOT(searchFinished()));
}

void TMDb::searchFinished()
{
    QList<ScraperSearchResult> results;
    if (m_searchReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_searchReply->readAll());
        results = parseSearch(msg);
    }
    m_searchReply->deleteLater();
    emit searchDone(results);
}

QList<ScraperSearchResult> TMDb::parseSearch(QString json)
{
    QList<ScraperSearchResult> results;
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate(json);
    if (sc.isArray() ) {
        QScriptValueIterator it(sc);
        while (it.hasNext() ) {
            it.next();
            if (it.value().property("id").toString().isEmpty()) {
                continue;
            }
            ScraperSearchResult result;
            result.name     = it.value().property("name").toString();
            result.id       = it.value().property("id").toString();
            result.released = QDate::fromString(it.value().property("released").toString(), "yyyy-MM-dd");
            results.append(result);
        }
    }

    return results;
}

void TMDb::loadData(QString id, Movie *movie)
{
    m_currentMovie = movie;
    QUrl url(QString("http://api.themoviedb.org/2.1/Movie.getInfo/%1/json/%2/%3").arg(m_language).arg(m_apiKey).arg(id));
    m_loadReply = this->qnam()->get(QNetworkRequest(url));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(loadFinished()));
}

void TMDb::loadFinished()
{
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        parseAndAssignInfos(msg, m_currentMovie);
    }
    m_loadReply->deleteLater();
    m_currentMovie->scraperLoadDone();
}

void TMDb::parseAndAssignInfos(QString json, Movie *movie)
{
    movie->clear();
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate(json);
    if (sc.isArray() ) {
        QScriptValueIterator it(sc);
        while (it.hasNext()) {
            it.next();
            QScriptValue v = it.value();
            if (v.property("id").toString().isEmpty()) {
                continue;
            }
            if (!v.property("name").isNull())
                movie->setName(v.property("name").toString());
            if (!v.property("original_name").isNull())
                movie->setOriginalName(v.property("original_name").toString());
            if (!v.property("overview").isNull())
                movie->setOverview(v.property("overview").toString());
            if (!v.property("rating").isNull())
                movie->setRating(v.property("rating").toNumber());
            if (!v.property("tagline").isNull())
               movie->setTagline(v.property("tagline").toString());
            if (!v.property("certification").isNull())
                movie->setCertification(v.property("certification").toString());
            if (!v.property("released").isNull())
                movie->setReleased(QDate::fromString(v.property("released").toString(), "yyyy-MM-dd"));
            if (!v.property("runtime").isNull())
                movie->setRuntime(v.property("runtime").toInteger());
            if (!v.property("trailer").isNull())
                movie->setTrailer(QUrl(v.property("trailer").toString()));
            if (v.property("genres").isArray()) {
                QScriptValueIterator itC(v.property("genres"));
                while (itC.hasNext()) {
                    itC.next();
                    QScriptValue vC = itC.value();
                    if (vC.property("id").toString().isEmpty())
                        continue;
                    movie->addGenre(vC.property("name").toString());
                }
            }
            if (v.property("studios").isArray()) {
                QScriptValueIterator itS(v.property("studios"));
                while (itS.hasNext()) {
                    itS.next();
                    QScriptValue vS = itS.value();
                    if (vS.property("id").toString().isEmpty())
                        continue;
                    movie->addStudio(vS.property("name").toString());
                }
            }
            if (v.property("countries").isArray()) {
                QScriptValueIterator itC(v.property("countries"));
                while (itC.hasNext()) {
                    itC.next();
                    QScriptValue vC = itC.value();
                    if (vC.property("name").toString().isEmpty())
                        continue;
                    movie->addCountry(vC.property("name").toString());
                }
            }
            if (v.property("cast").isArray()) {
                QScriptValueIterator itC(v.property("cast"));
                while (itC.hasNext()) {
                    itC.next();
                    QScriptValue vC = itC.value();
                    if (vC.property("name").toString().isEmpty() || vC.property("job").toString() != "Actor")
                        continue;
                    Actor a;
                    a.name = vC.property("name").toString();
                    a.role = vC.property("character").toString();
                    a.thumb = vC.property("profile").toString();
                    movie->addActor(a);
                }
            }
            if (v.property("posters").isArray()) {
                QScriptValueIterator itP(v.property("posters"));
                while (itP.hasNext()) {
                    itP.next();
                    QScriptValue vP = itP.value().property("image");
                    if (vP.property("id").toString().isEmpty() || vP.property("type").toString() != "poster")
                        continue;
                    int index = -1;
                    Poster p;
                    for (int i=0, n=movie->posters().size() ; i<n ; i++) {
                        if (movie->posters().at(i).id == vP.property("id").toString()) {
                            index = i;
                            p = movie->posters().at(i);
                        }
                    }
                    p.id = vP.property("id").toString();
                    if (vP.property("size").toString() == "thumb")
                        p.thumbUrl = vP.property("url").toString();
                    if (vP.property("size").toString() == "original")
                        p.originalUrl = vP.property("url").toString();
                    if (index == -1)
                        movie->addPoster(p);
                    else
                        movie->setPoster(index, p);
                }
            }
            if (v.property("backdrops").isArray()) {
                QScriptValueIterator itB(v.property("backdrops"));
                while (itB.hasNext()) {
                    itB.next();
                    QScriptValue vB = itB.value().property("image");
                    if (vB.property("id").toString().isEmpty() || vB.property("type").toString() != "backdrop")
                        continue;
                    int index = -1;
                    Poster b;
                    for (int i=0, n=movie->backdrops().size() ; i<n ; i++) {
                        if (movie->backdrops().at(i).id == vB.property("id").toString()) {
                            index = i;
                            b = movie->backdrops().at(i);
                        }
                    }
                    b.id = vB.property("id").toString();
                    if (vB.property("size").toString() == "thumb")
                        b.thumbUrl = vB.property("url").toString();
                    if (vB.property("size").toString() == "original")
                        b.originalUrl = vB.property("url").toString();
                    if (index == -1)
                        movie->addBackdrop(b);
                    else
                        movie->setBackdrop(index, b);
                }
            }
        }
    }
}
