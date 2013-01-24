#include "XbmcSync.h"
#include "ui_XbmcSync.h"

#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include "globals/Manager.h"
#include "settings/Settings.h"

XbmcSync::XbmcSync(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::XbmcSync)
{
    ui->setupUi(this);

    m_qnam = new QNetworkAccessManager(this);
    m_renameArtworkInProgress = false;
    m_cancelRenameArtwork = false;
    m_artworkWasRenamed = false;

    connect(ui->buttonSync, SIGNAL(clicked()), this, SLOT(startSync()));
    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(onButtonClose()));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    connect(ui->radioUpdateContents, SIGNAL(clicked()), this, SLOT(onRadioContents()));
    connect(ui->radioGetWatched, SIGNAL(clicked()), this, SLOT(onRadioWatched()));
    connect(ui->radioRenameArtwork, SIGNAL(clicked()), this, SLOT(onRadioRenameArtwork()));
    ui->progressBar->setVisible(false);
    onRadioContents();
}

XbmcSync::~XbmcSync()
{
    delete ui;
}

int XbmcSync::exec()
{
    m_renameArtworkInProgress = false;
    m_cancelRenameArtwork = false;
    m_artworkWasRenamed = false;
    ui->status->clear();
    ui->progressBar->setVisible(false);
    return QDialog::exec();
}

void XbmcSync::onButtonClose()
{
    if (m_renameArtworkInProgress) {
        m_cancelRenameArtwork = true;
        return;
    }
    reject();
}

void XbmcSync::reject()
{
    QDialog::reject();
    if (m_artworkWasRenamed) {
        Settings::instance()->setDataFiles(Settings::instance()->dataFilesFrodo());
        Settings::instance()->saveSettings();
        emit sigTriggerReload();
    } else {
        emit sigFinished();
    }
}

void XbmcSync::startSync()
{
    if (m_syncType == RenameArtwork) {
        renameArtwork();
        return;
    }
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
        if (show->syncNeeded() && m_syncType == SyncContents) {
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
                if (m_syncType == SyncContents) {
                    // m_episodesToSync.append(episode);
                    m_tvShowsToSync.append(show);
                    break;
                } else if (m_syncType == SyncWatched) {
                    m_episodesToSync.append(episode);
                }
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
    url.setUserName(Settings::instance()->xbmcUsername());
    url.setPassword(Settings::instance()->xbmcPassword());
    m_request.setUrl(url);
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    m_request.setRawHeader("Accept", "application/json");

    if (!m_moviesToSync.isEmpty()) {
        m_elements.append(ElementMovies);
        m_moviesReply = m_qnam->post(m_request, "{ \"jsonrpc\": \"2.0\", \"method\": \"VideoLibrary.GetMovies\", \"id\":1, " \
                                                "\"params\": { \"limits\": { \"end\": 100000 }, \"properties\": [\"file\", \"lastplayed\", \"playcount\"] } }");
        connect(m_moviesReply, SIGNAL(finished()), this, SLOT(onMovieListFinished()));
        connect(m_moviesReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress()));
    }

    if (!m_concertsToSync.isEmpty()) {
        m_elements.append(ElementConcerts);
        m_concertsReply = m_qnam->post(m_request, "{ \"jsonrpc\": \"2.0\", \"method\": \"VideoLibrary.GetMusicVideos\", \"id\":1, " \
                                                  "\"params\": { \"limits\": { \"end\": 100000 }, \"properties\": [\"file\", \"lastplayed\", \"playcount\"] } }");
        connect(m_concertsReply, SIGNAL(finished()), this, SLOT(onConcertListFinished()));
        connect(m_concertsReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress()));
    }

    if (!m_tvShowsToSync.isEmpty()) {
        m_elements.append(ElementTvShows);
        m_showReply = m_qnam->post(m_request, "{ \"jsonrpc\": \"2.0\", \"method\": \"VideoLibrary.GetTvShows\", \"id\":1, " \
                                              "\"params\": { \"limits\": { \"end\": 100000 }, \"properties\": [\"file\", \"lastplayed\", \"playcount\"] } }");
        connect(m_showReply, SIGNAL(finished()), this, SLOT(onTvShowListFinished()));
        connect(m_showReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress()));
    }

    if (!m_episodesToSync.isEmpty()) {
        m_elements.append(ElementEpisodes);
        m_episodeReply = m_qnam->post(m_request, "{ \"jsonrpc\": \"2.0\", \"method\": \"VideoLibrary.GetEpisodes\", \"id\":1, " \
                                                 "\"params\": { \"limits\": { \"end\": 100000 }, \"properties\": [\"file\", \"lastplayed\", \"playcount\"] } }");
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
            m_xbmcMovies.insert(v.property("movieid").toInteger(), parseXbmcDataFromScriptValue(v));
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
            m_xbmcConcerts.insert(v.property("musicvideoid").toInteger(), parseXbmcDataFromScriptValue(v));
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
            m_xbmcShows.insert(v.property("tvshowid").toInteger(), parseXbmcDataFromScriptValue(v));
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
            m_xbmcEpisodes.insert(v.property("episodeid").toInteger(), parseXbmcDataFromScriptValue(v));
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

    if (m_syncType == SyncContents) {
        setupItemsToRemove();
        removeItems();
    } else if (m_syncType == SyncWatched) {
        updateWatched();
    }
}

void XbmcSync::setupItemsToRemove()
{
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

void XbmcSync::updateWatched()
{
    foreach (Movie *movie, m_moviesToSync) {
        int id = findId(movie->files(), m_xbmcMovies);
        if (id > 0) {
            movie->setWatched(m_xbmcMovies.value(id).playCount > 0);
            movie->setPlayCount(m_xbmcMovies.value(id).playCount);
            movie->setLastPlayed(m_xbmcMovies.value(id).lastPlayed);
        }
        movie->setSyncNeeded(false);
    }

    foreach (Concert *concert, m_concertsToSync) {
        int id = findId(concert->files(), m_xbmcConcerts);
        if (id > 0) {
            concert->setWatched(m_xbmcConcerts.value(id).playCount > 0);
            concert->setPlayCount(m_xbmcConcerts.value(id).playCount);
            concert->setLastPlayed(m_xbmcConcerts.value(id).lastPlayed);
        }
        concert->setSyncNeeded(false);
    }

    foreach (TvShowEpisode *episode, m_episodesToSync) {
        int id = findId(episode->files(), m_xbmcEpisodes);
        if (id > 0) {
            episode->setPlayCount(m_xbmcEpisodes.value(id).playCount);
            episode->setLastPlayed(m_xbmcEpisodes.value(id).lastPlayed);
        }
        episode->setSyncNeeded(false);
    }

    ui->status->setText(tr("Finished. Your items play count and last played date have been updated."));
    ui->buttonSync->setEnabled(true);
}

int XbmcSync::findId(QStringList files, QMap<int, XbmcData> items)
{
    if (files.isEmpty())
        return -1;

    QList<int> matches;
    int level = 0;

    do {
        matches.clear();
        QMapIterator<int, XbmcData> it(items);
        while (it.hasNext()) {
            it.next();
            QString file = it.value().file;
            QStringList xbmcFiles;
            if (file.startsWith("stack://"))
                xbmcFiles << file.mid(8).split(" , ");
            else
                xbmcFiles << file;

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

void XbmcSync::onRadioContents()
{
    ui->labelContents->setVisible(true);
    ui->labelWatched->setVisible(false);
    ui->labelRenameArtwork->setVisible(false);
    m_syncType = SyncContents;
}

void XbmcSync::onRadioWatched()
{
    ui->labelContents->setVisible(false);
    ui->labelWatched->setVisible(true);
    ui->labelRenameArtwork->setVisible(false);
    m_syncType = SyncWatched;
}

void XbmcSync::onRadioRenameArtwork()
{
    ui->labelContents->setVisible(false);
    ui->labelWatched->setVisible(false);
    ui->labelRenameArtwork->setVisible(true);
    m_syncType = RenameArtwork;
}

XbmcSync::XbmcData XbmcSync::parseXbmcDataFromScriptValue(QScriptValue value)
{
    XbmcData d;
    d.file = value.property("file").toString();
    d.lastPlayed = QDateTime::fromString(value.property("lastplayed").toString(), "yyyy-MM-dd hh:mm:ss");
    d.playCount = value.property("playcount").toInteger();
    return d;
}

void XbmcSync::renameArtwork()
{
    m_artworkWasRenamed = true;
    m_cancelRenameArtwork = false;
    m_renameArtworkInProgress = true;
    ui->status->clear();
    ui->progressBar->setMaximum(1);
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(true);
    ui->buttonClose->setText(tr("Cancel"));
    ui->buttonSync->setEnabled(false);

    int items = 0;
    items += Manager::instance()->movieModel()->movies().count();
    items += Manager::instance()->concertModel()->concerts().count();
    items += Manager::instance()->tvShowModel()->tvShows().count();
    foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows())
        items += show->episodes().count();

    ui->progressBar->setMaximum(items);

    int currentItem = 0;
    ui->status->setText(tr("Renaming Movie Artwork..."));
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        MediaCenterInterface *interface = Manager::instance()->mediaCenterInterface();
        Settings *settings = Settings::instance();
        if (m_cancelRenameArtwork)
            break;

        if (!interface->posterImageName(movie).isEmpty())
            QFile(interface->posterImageName(movie)).
                    rename(interface->posterImageName(movie, settings->dataFilesFrodo(DataFileType::MoviePoster), true));
        if (!interface->backdropImageName(movie).isEmpty())
            QFile(interface->backdropImageName(movie)).
                    rename(interface->backdropImageName(movie, settings->dataFilesFrodo(DataFileType::MovieBackdrop), true));
        if (!interface->cdArtImageName(movie).isEmpty())
            QFile(interface->cdArtImageName(movie)).
                    rename(interface->cdArtImageName(movie, settings->dataFilesFrodo(DataFileType::MovieCdArt), true));
        if (!interface->clearArtImageName(movie).isEmpty())
            QFile(interface->clearArtImageName(movie)).
                    rename(interface->clearArtImageName(movie, settings->dataFilesFrodo(DataFileType::MovieClearArt), true));
        if (!interface->logoImageName(movie).isEmpty())
            QFile(interface->logoImageName(movie)).
                    rename(interface->logoImageName(movie, settings->dataFilesFrodo(DataFileType::MovieLogo), true));

        ui->progressBar->setValue(++currentItem);
        qApp->processEvents();
    }

    ui->status->setText(tr("Renaming TV Show and Episode Artwork..."));
    foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows()) {
        MediaCenterInterface *interface = Manager::instance()->mediaCenterInterfaceTvShow();
        Settings *settings = Settings::instance();
        if (m_cancelRenameArtwork)
            break;
        if (!interface->posterImageName(show).isEmpty())
            QFile(interface->posterImageName(show)).
                    rename(interface->posterImageName(show, settings->dataFilesFrodo(DataFileType::TvShowPoster), true));
        if (!interface->backdropImageName(show).isEmpty())
            QFile(interface->backdropImageName(show)).
                    rename(interface->backdropImageName(show, settings->dataFilesFrodo(DataFileType::TvShowBackdrop), true));
        if (!interface->bannerImageName(show).isEmpty())
            QFile(interface->bannerImageName(show)).
                    rename(interface->bannerImageName(show, settings->dataFilesFrodo(DataFileType::TvShowBanner), true));
        if (!interface->logoImageName(show).isEmpty())
            QFile(interface->logoImageName(show)).
                    rename(interface->logoImageName(show, settings->dataFilesFrodo(DataFileType::TvShowLogo), true));
        if (!interface->clearArtImageName(show).isEmpty())
            QFile(interface->clearArtImageName(show)).
                    rename(interface->clearArtImageName(show, settings->dataFilesFrodo(DataFileType::TvShowClearArt), true));
        if (!interface->characterArtImageName(show).isEmpty())
            QFile(interface->characterArtImageName(show)).
                    rename(interface->characterArtImageName(show, settings->dataFilesFrodo(DataFileType::TvShowCharacterArt), true));
        foreach (const int &season, show->seasons()) {
            if (!interface->seasonPosterImageName(show, season).isEmpty())
                QFile(interface->seasonPosterImageName(show, season)).
                        rename(interface->seasonPosterImageName(show, season, settings->dataFilesFrodo(DataFileType::TvShowSeasonPoster), true));
            if (!interface->seasonBackdropImageName(show, season).isEmpty())
                QFile(interface->seasonBackdropImageName(show, season)).
                        rename(interface->seasonBackdropImageName(show, season, settings->dataFilesFrodo(DataFileType::TvShowSeasonBackdrop), true));
            if (!interface->seasonBannerImageName(show, season).isEmpty())
                QFile(interface->seasonBannerImageName(show, season)).
                        rename(interface->seasonBannerImageName(show, season, settings->dataFilesFrodo(DataFileType::TvShowSeasonBanner), true));
        }

        ui->progressBar->setValue(++currentItem);
        qApp->processEvents();
        foreach (TvShowEpisode *episode, show->episodes()) {
            if (m_cancelRenameArtwork)
                break;
            if (!interface->thumbnailImageName(episode).isEmpty())
                QFile(interface->thumbnailImageName(episode)).
                        rename(interface->thumbnailImageName(episode, settings->dataFilesFrodo(DataFileType::TvShowEpisodeThumb), true));
            ui->progressBar->setValue(++currentItem);
            qApp->processEvents();
        }
    }

    ui->status->setText(tr("Renaming Concert Artwork..."));
    foreach (Concert *concert, Manager::instance()->concertModel()->concerts()) {
        MediaCenterInterface *interface = Manager::instance()->mediaCenterInterfaceConcert();
        Settings *settings = Settings::instance();
        if (m_cancelRenameArtwork)
            break;

        if (!interface->posterImageName(concert).isEmpty())
            QFile(interface->posterImageName(concert)).
                    rename(interface->posterImageName(concert, settings->dataFilesFrodo(DataFileType::ConcertPoster), true));
        if (!interface->backdropImageName(concert).isEmpty())
            QFile(interface->backdropImageName(concert)).
                    rename(interface->backdropImageName(concert, settings->dataFilesFrodo(DataFileType::ConcertBackdrop), true));
        if (!interface->cdArtImageName(concert).isEmpty())
            QFile(interface->cdArtImageName(concert)).
                    rename(interface->cdArtImageName(concert, settings->dataFilesFrodo(DataFileType::ConcertCdArt), true));
        if (!interface->clearArtImageName(concert).isEmpty())
            QFile(interface->clearArtImageName(concert)).
                    rename(interface->clearArtImageName(concert, settings->dataFilesFrodo(DataFileType::ConcertClearArt), true));
        if (!interface->logoImageName(concert).isEmpty())
            QFile(interface->logoImageName(concert)).
                    rename(interface->logoImageName(concert, settings->dataFilesFrodo(DataFileType::ConcertLogo), true));

        ui->progressBar->setValue(++currentItem);
        qApp->processEvents();
    }

    if (m_cancelRenameArtwork)
        ui->status->setText(tr("Canceled."));
    else
        ui->status->setText(tr("Finished. All artwork has been renamed."));
    m_renameArtworkInProgress = false;
    ui->progressBar->hide();
    ui->buttonClose->setEnabled(true);
    ui->buttonClose->setText(tr("Close"));
    ui->buttonSync->setEnabled(true);
}
