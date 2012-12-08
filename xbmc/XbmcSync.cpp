#include "XbmcSync.h"
#include "ui_XbmcSync.h"

#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include "data/Movie.h"
#include "globals/Manager.h"
#include "settings/Settings.h"

XbmcSync::XbmcSync(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::XbmcSync)
{
    ui->setupUi(this);

    m_qnam = new QNetworkAccessManager(this);

    connect(ui->buttonSync, SIGNAL(clicked()), this, SLOT(startSync()));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

XbmcSync::~XbmcSync()
{
    delete ui;
}

int XbmcSync::exec()
{
    ui->status->clear();
    return QDialog::exec();
}

void XbmcSync::startSync()
{
    m_allReady = false;
    m_elements.clear();
    m_aborted = false;

    m_moviesToSync.clear();
    m_concertsToSync.clear();
    m_tvShowsToSync.clear();
    m_episodesToSync.clear();

    m_xbmcMovies.clear();
    m_xbmcConcerts.clear();
    m_xbmcShows.clear();
    m_xbmcEpisodes.clear();

    m_moviesToRemove.clear();
    m_concertsToRemove.clear();
    m_tvShowsToRemove.clear();
    m_episodesToRemove.clear();

    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (movie->syncNeeded())
            m_moviesToSync.append(movie);
    }

    foreach (Concert *concert, Manager::instance()->concertModel()->concerts()) {
        if (concert->syncNeeded())
            m_concertsToSync.append(concert);
    }

    foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows()) {
        if (show->syncNeeded()) {
            m_tvShowsToSync.append(show);
            continue;
        }
        /* @todo: Enable updating single episodes
         * Syncing single episodes is currently disabled:
         * XBMC doesn't pickup new episodes when VideoLibrary.Scan is called
         * so removing of the whole show is needed.
         */
        foreach (TvShowEpisode *episode, show->episodes()) {
            if (episode->syncNeeded()) {
                // m_episodesToSync.append(episode);
                m_tvShowsToSync.append(show);
                break;
            }
        }
    }

    QString host = Settings::instance()->xbmcHost();
    int port = Settings::instance()->xbmcPort();

    if (host.isEmpty() || port == 0) {
        ui->status->setText(tr("Please fill in your XBMC host and port."));
        return;
    }

    QUrl url(QString("%1:%2/jsonrpc").arg(host).arg(port));
    url.setUserName(Settings.instance()->xbmcUsername());
    url.setPassword(Settings::instance()->xbmcPassword());
    m_request.setUrl(url);
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    m_request.setRawHeader("Accept", "application/json");

    if (!m_moviesToSync.isEmpty()) {
        m_elements.append(ElementMovies);
        m_moviesReply = m_qnam->post(m_request, "{ \"jsonrpc\": \"2.0\", \"method\": \"VideoLibrary.GetMovies\", \"id\":1, \"params\": { \"limits\": { \"end\": 100000 }, \"properties\": [\"file\"] } }");
        connect(m_moviesReply, SIGNAL(finished()), this, SLOT(onMovieListFinished()));
        connect(m_moviesReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress()));
    }

    if (!m_concertsToSync.isEmpty()) {
        m_elements.append(ElementConcerts);
        m_concertsReply = m_qnam->post(m_request, "{ \"jsonrpc\": \"2.0\", \"method\": \"VideoLibrary.GetMusicVideos\", \"id\":1, \"params\": { \"limits\": { \"end\": 100000 }, \"properties\": [\"file\"] } }");
        connect(m_concertsReply, SIGNAL(finished()), this, SLOT(onConcertListFinished()));
        connect(m_concertsReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress()));
    }

    if (!m_tvShowsToSync.isEmpty()) {
        m_elements.append(ElementTvShows);
        m_showReply = m_qnam->post(m_request, "{ \"jsonrpc\": \"2.0\", \"method\": \"VideoLibrary.GetTvShows\", \"id\":1, \"params\": { \"limits\": { \"end\": 100000 }, \"properties\": [\"file\"] } }");
        connect(m_showReply, SIGNAL(finished()), this, SLOT(onTvShowListFinished()));
        connect(m_showReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress()));
    }

    if (!m_episodesToSync.isEmpty()) {
        m_elements.append(ElementEpisodes);
        m_episodeReply = m_qnam->post(m_request, "{ \"jsonrpc\": \"2.0\", \"method\": \"VideoLibrary.GetEpisodes\", \"id\":1, \"params\": { \"limits\": { \"end\": 100000 }, \"properties\": [\"file\"] } }");
        connect(m_episodeReply, SIGNAL(finished()), this, SLOT(onEpisodeListFinished()));
        connect(m_episodeReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress()));
    }

    if (m_moviesToSync.isEmpty() && m_concertsToSync.isEmpty() && m_tvShowsToSync.isEmpty() && m_episodesToSync.isEmpty()) {
        ui->status->setText(tr("Nothing to synchronize"));
    } else {
        m_timer.start(5000);
        ui->status->setText(tr("Getting contents from XBMC"));
        ui->buttonSync->setEnabled(false);
    }
}

void XbmcSync::onTimeout()
{
    ui->status->setText(tr("XBMC is not reachable. Please check your settings."));
    ui->buttonSync->setEnabled(true);
    m_aborted = true;
    if (!m_moviesToSync.isEmpty() && m_moviesReply)
        m_moviesReply->abort();
    if (!m_concertsToSync.isEmpty() && m_concertsReply)
        m_concertsReply->abort();
    if (!m_tvShowsToSync.isEmpty() && m_showReply)
        m_showReply->abort();
    if (!m_episodesToSync.isEmpty() && m_episodeReply)
        m_episodeReply->abort();
}

void XbmcSync::onDownloadProgress()
{
    m_timer.stop();
}

void XbmcSync::onMovieListFinished()
{
    m_timer.stop();
    if (m_moviesReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_moviesReply->readAll());
        QScriptValue sc;
        QScriptEngine engine;
        sc = engine.evaluate("(" + QString(msg) + ")");
        QScriptValueIterator it(sc.property("result").property("movies"));
        while (it.hasNext()) {
            it.next();
            QScriptValue v = it.value();
            if (v.property("movieid").isNull() || v.property("movieid").toInteger() == 0)
                continue;
            m_xbmcMovies.insert(v.property("movieid").toInteger(), v.property("file").toString());
        }
    } else {
        qWarning() << m_moviesReply->errorString();
        onTimeout();
    }
    m_moviesReply->deleteLater();
    checkIfListsReady(ElementMovies);
}

void XbmcSync::onConcertListFinished()
{
    m_timer.stop();
    if (m_concertsReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_concertsReply->readAll());
        QScriptValue sc;
        QScriptEngine engine;
        sc = engine.evaluate("(" + QString(msg) + ")");
        QScriptValueIterator it(sc.property("result").property("musicvideos"));
        while (it.hasNext()) {
            it.next();
            QScriptValue v = it.value();
            if (v.property("musicvideoid").isNull() || v.property("musicvideoid").toInteger() == 0)
                continue;
            m_xbmcConcerts.insert(v.property("musicvideoid").toInteger(), v.property("file").toString());
        }
    } else {
        qWarning() << m_concertsReply->errorString();
        onTimeout();
    }
    m_concertsReply->deleteLater();
    checkIfListsReady(ElementConcerts);
}

void XbmcSync::onTvShowListFinished()
{
    m_timer.stop();
    if (m_showReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_showReply->readAll());
        QScriptValue sc;
        QScriptEngine engine;
        sc = engine.evaluate("(" + QString(msg) + ")");
        QScriptValueIterator it(sc.property("result").property("tvshows"));
        while (it.hasNext()) {
            it.next();
            QScriptValue v = it.value();
            if (v.property("tvshowid").isNull() || v.property("tvshowid").toInteger() == 0)
                continue;
            m_xbmcShows.insert(v.property("tvshowid").toInteger(), v.property("file").toString());
        }
    } else {
        qWarning() << m_showReply->errorString();
        onTimeout();
    }
    m_showReply->deleteLater();
    checkIfListsReady(ElementTvShows);
}

void XbmcSync::onEpisodeListFinished()
{
    m_timer.stop();
    if (m_episodeReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_episodeReply->readAll());
        QScriptValue sc;
        QScriptEngine engine;
        sc = engine.evaluate("(" + QString(msg) + ")");
        QScriptValueIterator it(sc.property("result").property("episodes"));
        while (it.hasNext()) {
            it.next();
            QScriptValue v = it.value();
            if (v.property("episodeid").isNull() || v.property("episodeid").toInteger() == 0)
                continue;
            m_xbmcEpisodes.insert(v.property("episodeid").toInteger(), v.property("file").toString());
        }
    } else {
        qWarning() << m_episodeReply->errorString();
        onTimeout();
    }
    m_episodeReply->deleteLater();
    checkIfListsReady(ElementEpisodes);
}

void XbmcSync::checkIfListsReady(Elements element)
{
    QMutexLocker locker(&m_mutex);

    m_elements.removeOne(element);
    if (m_allReady || !m_elements.isEmpty() || m_aborted)
        return;

    m_allReady = true;

    foreach (Movie *movie, m_moviesToSync) {
        movie->setSyncNeeded(false);
        int id = findId(movie->files(), m_xbmcMovies);
        if (id > 0)
            m_moviesToRemove.append(id);
    }

    foreach (Concert *concert, m_concertsToSync) {
        concert->setSyncNeeded(false);
        int id = findId(concert->files(), m_xbmcConcerts);
        if (id > 0)
            m_concertsToRemove.append(id);
    }

    foreach (TvShow *show, m_tvShowsToSync) {
        show->setSyncNeeded(false);
        foreach (TvShowEpisode *episode, show->episodes())
            episode->setSyncNeeded(false);
        QString showDir = show->dir();
        if (showDir.contains("/") && !showDir.endsWith("/"))
            showDir.append("/");
        else if (!showDir.contains("/") && !showDir.endsWith("\\"))
            showDir.append("\\");
        int id = findId(QStringList() << showDir, m_xbmcShows);
        if (id > 0)
            m_tvShowsToRemove.append(id);
    }

    foreach (TvShowEpisode *episode, m_episodesToSync) {
        episode->setSyncNeeded(false);
        int id = findId(episode->files(), m_xbmcEpisodes);
        if (id > 0)
            m_episodesToRemove.append(id);
    }

    removeItems();
}

void XbmcSync::removeItems()
{
    if (!m_moviesToRemove.isEmpty()) {
        ui->status->setText(tr("Removing movies from database"));
        int id = m_moviesToRemove.takeFirst();
        m_reply = m_qnam->post(m_request, QString("{ \"jsonrpc\": \"2.0\", \"method\": \"VideoLibrary.RemoveMovie\", \"id\":1, \"params\": { \"movieid\": %1 } }").arg(id).toUtf8());
        connect(m_reply, SIGNAL(finished()), this, SLOT(onRemoveFinished()));
        return;
    }

    if (!m_concertsToRemove.isEmpty()) {
        ui->status->setText(tr("Removing concerts from database"));
        int id = m_concertsToRemove.takeFirst();
        m_reply = m_qnam->post(m_request, QString("{ \"jsonrpc\": \"2.0\", \"method\": \"VideoLibrary.RemoveMusicVideo\", \"id\":1, \"params\": { \"musicvideoid\": %1 } }").arg(id).toUtf8());
        connect(m_reply, SIGNAL(finished()), this, SLOT(onRemoveFinished()));
        return;
    }

    if (!m_tvShowsToRemove.isEmpty()) {
        ui->status->setText(tr("Removing TV shows from database"));
        int id = m_tvShowsToRemove.takeFirst();
        m_reply = m_qnam->post(m_request, QString("{ \"jsonrpc\": \"2.0\", \"method\": \"VideoLibrary.RemoveTVShow\", \"id\":1, \"params\": { \"tvshowid\": %1 } }").arg(id).toUtf8());
        connect(m_reply, SIGNAL(finished()), this, SLOT(onRemoveFinished()));
        return;
    }

    if (!m_episodesToRemove.isEmpty()) {
        ui->status->setText(tr("Removing episodes from database"));
        int id = m_episodesToRemove.takeFirst();
        m_reply = m_qnam->post(m_request, QString("{ \"jsonrpc\": \"2.0\", \"method\": \"VideoLibrary.RemoveEpisode\", \"id\":1, \"params\": { \"episodeid\": %1 } }").arg(id).toUtf8());
        connect(m_reply, SIGNAL(finished()), this, SLOT(onRemoveFinished()));
        return;
    }

    triggerReload();
}

void XbmcSync::onRemoveFinished()
{
    m_reply->deleteLater();
    if (!m_moviesToRemove.isEmpty() || !m_concertsToRemove.isEmpty() || !m_tvShowsToRemove.isEmpty() || !m_episodesToRemove.isEmpty())
        removeItems();
    else
        triggerReload();
}

void XbmcSync::triggerReload()
{
    ui->status->setText(tr("Trigger scan for new items"));
    m_reply = m_qnam->post(m_request, "{ \"jsonrpc\": \"2.0\", \"method\": \"VideoLibrary.Scan\", \"id\":1}");
    connect(m_reply, SIGNAL(finished()), this, SLOT(onScanFinished()));
}

void XbmcSync::onScanFinished()
{
    ui->status->setText(tr("Finished. XBMC is now loading your updated items."));
    ui->buttonSync->setEnabled(true);
}

int XbmcSync::findId(QStringList files, QMap<int, QString> items)
{
    if (files.isEmpty())
        return -1;

    QList<int> matches;
    int level = 0;

    do {
        matches.clear();
        QMapIterator<int, QString> it(items);
        while (it.hasNext()) {
            it.next();
            QStringList xbmcFiles;
            if (it.value().startsWith("stack://"))
                xbmcFiles << it.value().mid(8).split(" , ");
            else
                xbmcFiles << it.value();

            if (compareFiles(files, xbmcFiles, level))
                matches.append(it.key());
        }
    } while (matches.count() > 1 && level++ < 4);

    if (matches.count() == 1)
        return matches.at(0);
    else if (matches.count() == 0)
        return 0;
    else
        return -1;
}

bool XbmcSync::compareFiles(QStringList files, QStringList xbmcFiles, int level)
{
    if (files.count() == 1 && xbmcFiles.count() == 1) {
        QStringList file = splitFile(files.at(0));
        QStringList xbmcFile = splitFile(xbmcFiles.at(0));
        for (int i=0 ; i<=level ; ++i) {
            if (file.isEmpty() || xbmcFile.isEmpty())
                return false;

            if (QString::compare(file.takeLast(), xbmcFile.takeLast(), Qt::CaseInsensitive) != 0)
                return false;
        }
        return true;
    } else if (files.count() == xbmcFiles.count()) {
        // construct a new stack
        QStringList stack;
        QStringList xbmcStack;
        foreach (QString file, xbmcFiles) {
            QStringList parts = splitFile(file);
            if (parts.count() < level)
                return false;
            QStringList partsNew;
            for (int i=0 ; i<=level ; ++i)
                partsNew << parts.takeLast();
            xbmcStack << partsNew.join("/");
        }

        foreach (QString file, files) {
            QStringList parts = splitFile(file);
            if (parts.count() < level)
                return false;
            QStringList partsNew;
            for (int i=0 ; i<=level ; ++i)
                partsNew << parts.takeLast();
            stack << partsNew.join("/");
        }

        qSort(stack);
        qSort(xbmcStack);

        return (stack == xbmcStack);

    }
    return false;
}

QStringList XbmcSync::splitFile(QString file)
{
    // Windows file names must not contain /
    if (file.contains("/"))
        return file.split("/");
    else
        return file.split("\\");
}
