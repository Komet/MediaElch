#include "SettingsWindow.h"
#include "ui_SettingsWindow.h"

#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include "data/MovieFilesOrganizer.h"
#include "data/Storage.h"
#include "export/ExportTemplateLoader.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "notifications/NotificationBox.h"
#include "plugins/PluginsWidget.h"
#include "settings/DataFile.h"
#include "settings/ExportTemplateWidget.h"

SettingsWindow::SettingsWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);

    m_pluginDialog = new PluginManagerDialog(this);
    m_buttonActiveColor = QColor(70, 155, 198);
    m_buttonColor = QColor(128, 129, 132);

#ifdef Q_OS_MAC
    QFont smallFont = ui->labelGlobal->font();
    smallFont.setPointSize(smallFont.pointSize()-1);
    ui->labelGlobal->setFont(smallFont);
    ui->label_44->setFont(smallFont);
    ui->label_45->setFont(smallFont);
    ui->label_46->setFont(smallFont);
    ui->label_47->setFont(smallFont);
    ui->label_48->setFont(smallFont);
    ui->label_49->setFont(smallFont);
    ui->label_7->setFont(smallFont);
    ui->label_18->setFont(smallFont);
#endif

    ui->customScraperTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->customScraperTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->customScraperTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->tvScraperTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tvScraperTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tvScraperTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->stackedWidget->setCurrentIndex(0);
    //ui->stackedWidget->setAnimation(QEasingCurve::Linear);
    //ui->stackedWidget->setSpeed(200);

    m_settings = Settings::instance(this);

    ui->xbmcPort->setValidator(new QIntValidator(0, 99999, ui->xbmcPort));
    ui->dirs->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->dirs->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->exportTemplates->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    int scraperCounter = 0;
    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->hasSettings()) {
            QLabel *name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            m_scraperRows.insert(scraper, scraperCounter);
            scraperCounter++;
        }
    }
    foreach (TvScraperInterface *scraper, Manager::instance()->tvScrapers()) {
        if (scraper->hasSettings()) {
            QLabel *name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            scraperCounter++;
        }
    }
    foreach (ConcertScraperInterface *scraper, Manager::instance()->concertScrapers()) {
        if (scraper->hasSettings()) {
            QLabel *name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            scraperCounter++;
        }
    }
    foreach (MusicScraperInterface *scraper, Manager::instance()->musicScrapers()) {
        if (scraper->hasSettings()) {
            QLabel *name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            scraperCounter++;
        }
    }

    foreach (ImageProviderInterface *scraper, Manager::instance()->imageProviders()) {
        if (scraper->hasSettings()) {
            QLabel *name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            scraperCounter++;
        }
    }

    ui->comboMovieSetArtwork->setItemData(0, MovieSetArtworkSingleSetFolder);
    ui->comboMovieSetArtwork->setItemData(1, MovieSetArtworkSingleArtworkFolder);

    Helper::instance()->removeFocusRect(ui->stackedWidget->widget(9));

    connect(ui->buttonAddDir, SIGNAL(clicked()), this, SLOT(chooseDirToAdd()));
    connect(ui->buttonRemoveDir, SIGNAL(clicked()), this, SLOT(removeDir()));
    connect(ui->buttonMovieFilesToDirs, SIGNAL(clicked()), this, SLOT(organize()));
    connect(ui->dirs, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(dirListRowChanged(int)));
    connect(ui->comboMovieSetArtwork, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboMovieSetArtworkChanged()));
    connect(ui->btnMovieSetArtworkDir, SIGNAL(clicked()), this, SLOT(onChooseMovieSetArtworkDir()));
    connect(ui->chkUseProxy, SIGNAL(clicked()), this, SLOT(onUseProxy()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(onCancel()));
    connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onSave()));
    connect(ExportTemplateLoader::instance(this), SIGNAL(sigTemplatesLoaded(QList<ExportTemplate*>)), this, SLOT(onTemplatesLoaded(QList<ExportTemplate*>)));
    connect(ExportTemplateLoader::instance(this), SIGNAL(sigTemplateInstalled(ExportTemplate*,bool)), this, SLOT(onTemplateInstalled(ExportTemplate*,bool)));
    connect(ExportTemplateLoader::instance(this), SIGNAL(sigTemplateUninstalled(ExportTemplate*,bool)), this, SLOT(onTemplateUninstalled(ExportTemplate*,bool)));
    connect(ui->btnChooseUnrar, SIGNAL(clicked()), this, SLOT(onChooseUnrar()));
    connect(ui->btnChooseMakemkvcon, SIGNAL(clicked()), this, SLOT(onChooseMakeMkvCon()));
    connect(ui->chkEnableAdultScrapers, SIGNAL(clicked()), this, SLOT(onShowAdultScrapers()));

    ui->movieNfo->setProperty("dataFileType", DataFileType::MovieNfo);
    ui->moviePoster->setProperty("dataFileType", DataFileType::MoviePoster);
    ui->movieBackdrop->setProperty("dataFileType", DataFileType::MovieBackdrop);
    ui->movieCdArt->setProperty("dataFileType", DataFileType::MovieCdArt);
    ui->movieClearArt->setProperty("dataFileType", DataFileType::MovieClearArt);
    ui->movieLogo->setProperty("dataFileType", DataFileType::MovieLogo);
    ui->movieBanner->setProperty("dataFileType", DataFileType::MovieBanner);
    ui->movieThumb->setProperty("dataFileType", DataFileType::MovieThumb);
    ui->movieSetPosterFileName->setProperty("dataFileType", DataFileType::MovieSetPoster);
    ui->movieSetFanartFileName->setProperty("dataFileType", DataFileType::MovieSetBackdrop);
    ui->showBackdrop->setProperty("dataFileType", DataFileType::TvShowBackdrop);
    ui->showBanner->setProperty("dataFileType", DataFileType::TvShowBanner);
    ui->showCharacterArt->setProperty("dataFileType", DataFileType::TvShowCharacterArt);
    ui->showClearArt->setProperty("dataFileType", DataFileType::TvShowClearArt);
    ui->showEpisodeNfo->setProperty("dataFileType", DataFileType::TvShowEpisodeNfo);
    ui->showEpisodeThumbnail->setProperty("dataFileType", DataFileType::TvShowEpisodeThumb);
    ui->showLogo->setProperty("dataFileType", DataFileType::TvShowLogo);
    ui->showThumb->setProperty("dataFileType", DataFileType::TvShowThumb);
    ui->showNfo->setProperty("dataFileType", DataFileType::TvShowNfo);
    ui->showPoster->setProperty("dataFileType", DataFileType::TvShowPoster);
    ui->showSeasonBackdrop->setProperty("dataFileType", DataFileType::TvShowSeasonBackdrop);
    ui->showSeasonBanner->setProperty("dataFileType", DataFileType::TvShowSeasonBanner);
    ui->showSeasonPoster->setProperty("dataFileType", DataFileType::TvShowSeasonPoster);
    ui->showSeasonThumb->setProperty("dataFileType", DataFileType::TvShowSeasonThumb);
    ui->concertNfo->setProperty("dataFileType", DataFileType::ConcertNfo);
    ui->concertPoster->setProperty("dataFileType", DataFileType::ConcertPoster);
    ui->concertBackdrop->setProperty("dataFileType", DataFileType::ConcertBackdrop);
    ui->concertLogo->setProperty("dataFileType", DataFileType::ConcertLogo);
    ui->concertClearArt->setProperty("dataFileType", DataFileType::ConcertClearArt);
    ui->concertDiscArt->setProperty("dataFileType", DataFileType::ConcertCdArt);
    ui->artistFanart->setProperty("dataFileType", DataFileType::ArtistFanart);
    ui->artistLogo->setProperty("dataFileType", DataFileType::ArtistLogo);
    ui->artistThumb->setProperty("dataFileType", DataFileType::ArtistThumb);
    ui->albumThumb->setProperty("dataFileType", DataFileType::AlbumThumb);
    ui->albumDiscArt->setProperty("dataFileType", DataFileType::AlbumCdArt);

#ifdef Q_OS_MAC
    ui->btnCancel->setVisible(false);
    ui->btnSave->setVisible(false);
    ui->horizontalSpacerButtons->setGeometry(QRect(0, 0, 1, 1));
#endif

    ui->comboStartupSection->addItem(tr("Movies"), "movies");
    ui->comboStartupSection->addItem(tr("TV Shows"), "tvshows");
    ui->comboStartupSection->addItem(tr("Concerts"), "concerts");
    ui->comboStartupSection->addItem(tr("Music"), "music");
    ui->comboStartupSection->addItem(tr("Import"), "import");

    QPainter p;
    foreach (QAction *action, findChildren<QAction*>()) {
        if (!action->property("page").isValid())
            continue;
        action->setIcon(Manager::instance()->iconFont()->icon(action->property("iconName").toString(), m_buttonColor));
    }
    ui->actionGlobal->setIcon(Manager::instance()->iconFont()->icon(ui->actionGlobal->property("iconName").toString(), m_buttonActiveColor));

#ifndef PLUGINS
    ui->actionPlugins->setVisible(false);
#endif

    connect(PluginManager::instance(), SIGNAL(sigPluginListUpdated(QList<PluginManager::Plugin>)), this, SLOT(onPluginListUpdated(QList<PluginManager::Plugin>)));
    connect(ui->pluginList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(onPluginActivated(QListWidgetItem*)));
    connect(ui->btnInstallPlugin, SIGNAL(clicked()), this, SLOT(onInstallPlugin()));
    connect(ui->btnUninstallPlugin, SIGNAL(clicked()), this, SLOT(onUninstallPlugin()));
    connect(ui->btnUpdatePlugin, SIGNAL(clicked()), this, SLOT(onUpdatePlugin()));
    ui->btnUninstallPlugin->setVisible(false);
    ui->btnUpdatePlugin->setVisible(false);
    Helper::instance()->setButtonStyle(ui->btnInstallPlugin, Helper::ButtonSuccess);
    Helper::instance()->setButtonStyle(ui->btnUninstallPlugin, Helper::ButtonDanger);
    Helper::instance()->setButtonStyle(ui->btnUpdatePlugin, Helper::ButtonInfo);

    m_pluginsInstallable = false;
#if defined(Q_OS_MAC)
    m_pluginsInstallable = true;
    ui->pluginsLinux->setVisible(false);
#elif defined(Q_OS_WIN)
    m_pluginsInstallable = true;
    ui->pluginsLinux->setVisible(false);
#endif

#if !defined(PLUGINS)
    ui->actionPlugins->setVisible(false);
#endif

    loadSettings();
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::show()
{
    ui->themesErrorMessage->setText("");
    loadRemoteTemplates();
    loadSettings();
    if (Settings::instance()->settingsWindowSize().isValid() && !Settings::instance()->settingsWindowPosition().isNull()) {
        move(Settings::instance()->settingsWindowPosition());
        resize(Settings::instance()->settingsWindowSize());
    }
    QMainWindow::show();
}

void SettingsWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
#ifdef Q_OS_MAC
    saveSettings();
    emit sigSaved();
#endif

    Settings::instance()->setSettingsWindowSize(size());
    Settings::instance()->setSettingsWindowPosition(pos());
}

void SettingsWindow::onSave()
{
    saveSettings();
    close();
    emit sigSaved();
}

void SettingsWindow::onCancel()
{
    m_settings->loadSettings();
    close();
}

void SettingsWindow::onAction()
{
    QAction *triggeredAction = static_cast<QAction*>(sender());
    foreach (QAction *action, ui->toolBar->actions())
        action->setIcon(Manager::instance()->iconFont()->icon(action->property("iconName").toString(), m_buttonColor));
    triggeredAction->setIcon(Manager::instance()->iconFont()->icon(triggeredAction->property("iconName").toString(), m_buttonActiveColor));
    //ui->stackedWidget->slideInIdx(triggeredAction->property("page").toInt());
    ui->stackedWidget->setCurrentIndex(triggeredAction->property("page").toInt());
}

void SettingsWindow::loadSettings()
{
    m_settings->loadSettings();

    // Stream Details
    ui->chkAutoLoadStreamDetails->setChecked(m_settings->autoLoadStreamDetails());

    // Proxy
    ui->chkUseProxy->setChecked(m_settings->useProxy());
    ui->proxyType->setCurrentIndex(m_settings->proxyType());
    ui->proxyHost->setText(m_settings->proxyHost());
    ui->proxyPort->setValue(m_settings->proxyPort());
    ui->proxyUsername->setText(m_settings->proxyUsername());
    ui->proxyPassword->setText(m_settings->proxyPassword());
    onUseProxy();

    ui->usePlotForOutline->setChecked(m_settings->usePlotForOutline());
    ui->chkDownloadActorImages->setChecked(m_settings->downloadActorImages());
    ui->chkIgnoreArticlesWhenSorting->setChecked(m_settings->ignoreArticlesWhenSorting());
    ui->chkCheckForUpdates->setChecked(m_settings->checkForUpdates());
    ui->chkEnableAdultScrapers->setChecked(m_settings->showAdultScrapers());
    onShowAdultScrapers();

    for (int i=0, n=ui->comboStartupSection->count() ; i<n ; ++i) {
        if (ui->comboStartupSection->itemData(i, Qt::UserRole) == m_settings->startupSection()) {
            ui->comboStartupSection->setCurrentIndex(i);
            break;
        }
    }

    // Directories
    ui->dirs->setRowCount(0);
    ui->dirs->clearContents();
    QList<SettingsDir> movieDirectories = m_settings->movieDirectories();
    for (int i=0, n=movieDirectories.count() ; i<n ; ++i)
        addDir(movieDirectories.at(i).path, movieDirectories.at(i).separateFolders, movieDirectories.at(i).autoReload, DirTypeMovies);
    QList<SettingsDir> tvShowDirectories = m_settings->tvShowDirectories();
    for (int i=0, n=tvShowDirectories.count() ; i<n ; ++i)
        addDir(tvShowDirectories.at(i).path, tvShowDirectories.at(i).separateFolders, tvShowDirectories.at(i).autoReload, DirTypeTvShows);
    QList<SettingsDir> concertDirectories = m_settings->concertDirectories();
    for (int i=0, n=concertDirectories.count() ; i<n ; ++i)
        addDir(concertDirectories.at(i).path, concertDirectories.at(i).separateFolders, concertDirectories.at(i).autoReload, DirTypeConcerts);
    QList<SettingsDir> downloadDirectories = m_settings->downloadDirectories();
    for (int i=0, n=downloadDirectories.count() ; i<n ; ++i)
        addDir(downloadDirectories.at(i).path, false, false, DirTypeDownloads);
    QList<SettingsDir> musicDirectories = m_settings->musicDirectories();
    for (int i=0, n=musicDirectories.count() ; i<n ; ++i)
        addDir(musicDirectories.at(i).path, musicDirectories.at(i).separateFolders, musicDirectories.at(i).autoReload, DirTypeMusic);

    dirListRowChanged(ui->dirs->currentRow());

    // Exclude words
    ui->excludeWordsText->setPlainText(m_settings->excludeWords());

    ui->useYoutubePluginUrls->setChecked(m_settings->useYoutubePluginUrls());

    // XBMC
    ui->xbmcHost->setText(m_settings->xbmcHost());
    if (m_settings->xbmcPort() != 0)
        ui->xbmcPort->setText(QString::number(m_settings->xbmcPort()));
    else
        ui->xbmcPort->clear();
    ui->xbmcUser->setText(m_settings->xbmcUser());
    ui->xbmcPassword->setText(m_settings->xbmcPassword());

    // Movie set artwork
    for (int i=0, n=ui->comboMovieSetArtwork->count() ; i<n ; ++i) {
        if (ui->comboMovieSetArtwork->itemData(i).toInt() == m_settings->movieSetArtworkType()) {
            ui->comboMovieSetArtwork->setCurrentIndex(i);
            break;
        }
    }
    ui->movieSetArtworkDir->setText(m_settings->movieSetArtworkDirectory());
    onComboMovieSetArtworkChanged();

    foreach (QLineEdit *lineEdit, findChildren<QLineEdit*>()) {
        if (lineEdit->property("dataFileType").isNull())
            continue;
        int dataFileType = lineEdit->property("dataFileType").toInt();
        QList<DataFile> dataFiles = m_settings->dataFiles(dataFileType);
        QStringList filenames;
        foreach (DataFile dataFile, dataFiles)
            filenames << dataFile.fileName();
        lineEdit->setText(filenames.join(","));
    }

    QList<int> infos = QList<int>() << MovieScraperInfos::Title << MovieScraperInfos::Set << MovieScraperInfos::Tagline
                                    << MovieScraperInfos::Rating << MovieScraperInfos::Released << MovieScraperInfos::Runtime
                                    << MovieScraperInfos::Director << MovieScraperInfos::Writer << MovieScraperInfos::Certification
                                    << MovieScraperInfos::Trailer << MovieScraperInfos::Overview << MovieScraperInfos::Poster
                                    << MovieScraperInfos::Backdrop << MovieScraperInfos::Actors << MovieScraperInfos::Genres
                                    << MovieScraperInfos::Studios << MovieScraperInfos::Countries << MovieScraperInfos::Logo
                                    << MovieScraperInfos::ClearArt << MovieScraperInfos::CdArt << MovieScraperInfos::Banner
                                    << MovieScraperInfos::Thumb;

    ui->customScraperTable->clearContents();
    ui->customScraperTable->setRowCount(0);

    foreach (const int &info, infos) {
        int row = ui->customScraperTable->rowCount();
        ui->customScraperTable->insertRow(row);
        ui->customScraperTable->setItem(row, 0, new QTableWidgetItem(titleForMovieScraperInfo(info)));
        ui->customScraperTable->setCellWidget(row, 1, comboForMovieScraperInfo(info));
    }


    QList<int> tvInfos = QList<int>() << TvShowScraperInfos::Title << TvShowScraperInfos::Rating << TvShowScraperInfos::FirstAired
                                      << TvShowScraperInfos::Runtime << TvShowScraperInfos::Director << TvShowScraperInfos::Writer
                                      << TvShowScraperInfos::Certification << TvShowScraperInfos::Overview << TvShowScraperInfos::Genres
                                      << TvShowScraperInfos::Actors;

    ui->tvScraperTable->clearContents();
    ui->tvScraperTable->setRowCount(0);

    foreach (const int &info, tvInfos) {
        int row = ui->tvScraperTable->rowCount();
        ui->tvScraperTable->insertRow(row);
        ui->tvScraperTable->setItem(row, 0, new QTableWidgetItem(titleForTvScraperInfo(info)));
        ui->tvScraperTable->setCellWidget(row, 1, comboForTvScraperInfo(info));
    }

    ui->chkDeleteArchives->setChecked(m_settings->deleteArchives());
    ui->unrarPath->setText(m_settings->unrar());
    ui->makemkvconPath->setText(m_settings->makeMkvCon());

    ui->artistExtraFanarts->setValue(m_settings->extraFanartsMusicArtists());
}

void SettingsWindow::saveSettings()
{
    QList<DataFile> dataFiles;
    foreach (QLineEdit *lineEdit, findChildren<QLineEdit*>()) {
        if (lineEdit->property("dataFileType").isNull())
            continue;
        int pos = 0;
        int dataFileType = lineEdit->property("dataFileType").toInt();
        QStringList filenames = lineEdit->text().split(",", QString::SkipEmptyParts);
        foreach (const QString &filename, filenames) {
            DataFile df(dataFileType, filename.trimmed(), pos++);
            dataFiles << df;
        }
    }
    m_settings->setDataFiles(dataFiles);

    m_settings->setUseYoutubePluginUrls(ui->useYoutubePluginUrls->isChecked());
    m_settings->setAutoLoadStreamDetails(ui->chkAutoLoadStreamDetails->isChecked());
    m_settings->setDownloadActorImages(ui->chkDownloadActorImages->isChecked());
    m_settings->setIgnoreArticlesWhenSorting(ui->chkIgnoreArticlesWhenSorting->isChecked());
    m_settings->setCheckForUpdates(ui->chkCheckForUpdates->isChecked());
    m_settings->setShowAdultScrapers(ui->chkEnableAdultScrapers->isChecked());
    m_settings->setStartupSection(ui->comboStartupSection->itemData(ui->comboStartupSection->currentIndex()).toString());

    m_settings->setXbmcHost(ui->xbmcHost->text());
    m_settings->setXbmcPort(ui->xbmcPort->text().toInt());
    m_settings->setXbmcUser(ui->xbmcUser->text());
    m_settings->setXbmcPassword(ui->xbmcPassword->text());

    // Proxy
    m_settings->setUseProxy(ui->chkUseProxy->isChecked());
    m_settings->setProxyType(ui->proxyType->currentIndex());
    m_settings->setProxyHost(ui->proxyHost->text());
    m_settings->setProxyPort(ui->proxyPort->value());
    m_settings->setProxyUsername(ui->proxyUsername->text());
    m_settings->setProxyPassword(ui->proxyPassword->text());

    m_settings->setUsePlotForOutline(ui->usePlotForOutline->isChecked());

    // Save Directories
    QList<SettingsDir> movieDirectories;
    QList<SettingsDir> tvShowDirectories;
    QList<SettingsDir> concertDirectories;
    QList<SettingsDir> downloadDirectories;
    QList<SettingsDir> musicDirectories;
    for (int row=0, n=ui->dirs->rowCount() ; row<n ; ++row) {
        SettingsDir dir;
        dir.path = ui->dirs->item(row, 1)->text();
        dir.separateFolders = ui->dirs->item(row, 2)->checkState() == Qt::Checked;
        dir.autoReload = ui->dirs->item(row, 3)->checkState() == Qt::Checked;
        if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() == 0)
            movieDirectories.append(dir);
        else if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() == 1)
            tvShowDirectories.append(dir);
        else if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() == 2)
            concertDirectories.append(dir);
        else if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() == 3)
            downloadDirectories.append(dir);
        else if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() == 4)
            musicDirectories.append(dir);
    }
    m_settings->setMovieDirectories(movieDirectories);
    m_settings->setTvShowDirectories(tvShowDirectories);
    m_settings->setConcertDirectories(concertDirectories);
    m_settings->setDownloadDirectories(downloadDirectories);
    m_settings->setMusicDirectories(musicDirectories);

    // exclude words
    m_settings->setExcludeWords(ui->excludeWordsText->toPlainText());

    // Movie set artwork
    m_settings->setMovieSetArtworkType(static_cast<MovieSetArtworkType>(ui->comboMovieSetArtwork->itemData(ui->comboMovieSetArtwork->currentIndex()).toInt()));
    m_settings->setMovieSetArtworkDirectory(ui->movieSetArtworkDir->text());

    // Custom movie scraper
    QMap<int, QString> customMovieScraper;
    for (int row=0, n=ui->customScraperTable->rowCount() ; row<n ; ++row) {
        QComboBox *box = static_cast<QComboBox*>(ui->customScraperTable->cellWidget(row, 1));
        int info = box->itemData(0, Qt::UserRole+1).toInt();
        QString scraper = box->itemData(box->currentIndex()).toString();
        customMovieScraper.insert(info, scraper);
    }
    m_settings->setCustomMovieScraper(customMovieScraper);

    // tv scraper
    QMap<int, QString> tvScraper;
    for (int row=0, n=ui->tvScraperTable->rowCount() ; row<n ; ++row) {
        QComboBox *box = static_cast<QComboBox*>(ui->tvScraperTable->cellWidget(row, 1));
        int info = box->itemData(0, Qt::UserRole+1).toInt();
        QString scraper = box->itemData(box->currentIndex()).toString();
        tvScraper.insert(info, scraper);
    }
    m_settings->setCustomTvScraper(tvScraper);

    // Downloads
    m_settings->setUnrar(ui->unrarPath->text());
    m_settings->setMakeMkvCon(ui->makemkvconPath->text());
    m_settings->setDeleteArchives(ui->chkDeleteArchives->isChecked());

    m_settings->setExtraFanartsMusicArtists(ui->artistExtraFanarts->value());

    m_settings->saveSettings();

    Manager::instance()->movieFileSearcher()->setMovieDirectories(m_settings->movieDirectories());
    Manager::instance()->tvShowFileSearcher()->setMovieDirectories(m_settings->tvShowDirectories());
    Manager::instance()->concertFileSearcher()->setConcertDirectories(m_settings->concertDirectories());
    Manager::instance()->musicFileSearcher()->setMusicDirectories(m_settings->musicDirectories());
    NotificationBox::instance()->showMessage(tr("Settings saved"));
}

void SettingsWindow::addDir(QString dir, bool separateFolders, bool autoReload, SettingsDirType dirType)
{
    dir = QDir::toNativeSeparators(dir);
    if (!dir.isEmpty()) {
        bool exists = false;
        for (int i=0, n=ui->dirs->rowCount() ; i<n ; ++i) {
            if (ui->dirs->item(i, 1)->text() == dir)
                exists = true;
        }

        if (!exists) {
            int row = ui->dirs->rowCount();
            ui->dirs->insertRow(row);
            QTableWidgetItem *item = new QTableWidgetItem(dir);
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setToolTip(dir);
            QTableWidgetItem *itemCheck = new QTableWidgetItem();
            if (separateFolders)
                itemCheck->setCheckState(Qt::Checked);
            else
                itemCheck->setCheckState(Qt::Unchecked);

            QTableWidgetItem *itemCheckReload = new QTableWidgetItem();
            if (autoReload)
                itemCheckReload->setCheckState(Qt::Checked);
            else
                itemCheckReload->setCheckState(Qt::Unchecked);

            QComboBox *box = new QComboBox();
            box->setProperty("itemCheck", Storage::toVariant(box, itemCheck));
            box->setProperty("itemCheckReload", Storage::toVariant(box, itemCheckReload));
            box->addItems(QStringList() << tr("Movies") << tr("TV Shows") << tr("Concerts") << tr("Downloads") << tr("Music"));
            if (dirType == DirTypeMovies)
                box->setCurrentIndex(0);
            else if (dirType == DirTypeTvShows)
                box->setCurrentIndex(1);
            else if (dirType == DirTypeConcerts)
                box->setCurrentIndex(2);
            else if (dirType == DirTypeDownloads)
                box->setCurrentIndex(3);
            else if (dirType == DirTypeMusic)
                box->setCurrentIndex(4);

            ui->dirs->setCellWidget(row, 0, box);
            ui->dirs->setItem(row, 1, item);
            ui->dirs->setItem(row, 2, itemCheck);
            ui->dirs->setItem(row, 3, itemCheckReload);

            connect(box, SIGNAL(currentIndexChanged(int)), this, SLOT(onDirTypeChanged()));
            onDirTypeChanged(box);
        }
    }
}

void SettingsWindow::removeDir()
{
    int row = ui->dirs->currentRow();
    if (row < 0)
        return;
    ui->dirs->removeRow(row);
}

void SettingsWindow::organize()
{
    MovieFilesOrganizer* organizer = new MovieFilesOrganizer(this);

    int row = ui->dirs->currentRow();
    if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() != 0
            || ui->dirs->item(row, 2)->checkState() == Qt::Checked) {
        organizer->canceled(tr("Organizing movies does only work on " \
                                      "movies, not already sorted to " \
                                      "separate folders."));
        return;
    }

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(tr("Are you sure?"));
    msgBox.setInformativeText(tr("This operation sorts all movies in this directory to separate " \
                                 "sub-directories based on the file name. Click \"Ok\", if thats, what you want to do. "));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setButtonText(1, tr("Ok"));
    msgBox.setButtonText(2, tr("Cancel"));
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();

    switch (ret) {
      case QMessageBox::Ok:
        organizer->moveToDirs(ui->dirs->item(ui->dirs->currentRow(), 1)->text());
        ui->dirs->item(ui->dirs->currentRow(), 2)->setCheckState(Qt::Checked);
        break;
      case QMessageBox::Cancel:
        break;
      default:
        break;
    }
}

void SettingsWindow::dirListRowChanged(int currentRow)
{
    if (currentRow < 0 || currentRow >= ui->dirs->rowCount()) {
        ui->buttonRemoveDir->setDisabled(true);
        ui->buttonMovieFilesToDirs->setDisabled(true);
    } else {
        ui->buttonRemoveDir->setDisabled(false);
        if (ui->dirs->cellWidget(currentRow, 0) != 0
                && static_cast<QComboBox*>(ui->dirs->cellWidget(currentRow, 0))->currentIndex() == 0
                && ui->dirs->item(currentRow, 2)->checkState() == Qt::Unchecked) {
            ui->buttonMovieFilesToDirs->setDisabled(false);
        } else {
            ui->buttonMovieFilesToDirs->setDisabled(true);
        }
    }
}

void SettingsWindow::onUseProxy()
{
    bool enabled = ui->chkUseProxy->isChecked();
    ui->proxyType->setEnabled(enabled);
    ui->proxyHost->setEnabled(enabled);
    ui->proxyPort->setEnabled(enabled);
    ui->proxyUsername->setEnabled(enabled);
    ui->proxyPassword->setEnabled(enabled);
}

void SettingsWindow::chooseDirToAdd()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a directory containing your movies, TV show or concerts"), QDir::homePath());
    if (!dir.isEmpty())
        addDir(dir);
}

void SettingsWindow::onComboMovieSetArtworkChanged()
{
    int value = ui->comboMovieSetArtwork->itemData(ui->comboMovieSetArtwork->currentIndex()).toInt();
    ui->btnMovieSetArtworkDir->setEnabled(value == MovieSetArtworkSingleArtworkFolder);
    ui->movieSetArtworkDir->setEnabled(value == MovieSetArtworkSingleArtworkFolder);

    if (value == MovieSetArtworkSingleArtworkFolder) {
        ui->movieSetPosterFileName->setText("<setName>-folder.jpg");
        ui->movieSetFanartFileName->setText("<setName>-fanart.jpg");
    } else if (value == MovieSetArtworkSingleSetFolder) {
        ui->movieSetPosterFileName->setText("folder.jpg");
        ui->movieSetFanartFileName->setText("fanart.jpg");
    }
}

void SettingsWindow::onChooseMovieSetArtworkDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a directory where your movie set artwork is stored"), QDir::homePath());
    if (!dir.isEmpty())
        ui->movieSetArtworkDir->setText(dir);
}

void SettingsWindow::loadRemoteTemplates()
{
    ExportTemplateLoader::instance()->getRemoteTemplates();
}

void SettingsWindow::onTemplatesLoaded(QList<ExportTemplate*> templates)
{
    ui->exportTemplates->clearContents();
    ui->exportTemplates->setRowCount(0);

    foreach (ExportTemplate *exportTemplate, templates) {
        ExportTemplateWidget *widget = new ExportTemplateWidget(ui->exportTemplates);
        widget->setExportTemplate(exportTemplate);

        int row = ui->exportTemplates->rowCount();
        ui->exportTemplates->insertRow(row);
        ui->exportTemplates->setCellWidget(row, 0, widget);
        widget->adjustSize();
    }
}

void SettingsWindow::onTemplateInstalled(ExportTemplate *exportTemplate, bool success)
{
    if (success)
        ui->themesErrorMessage->setSuccessMessage(tr("Theme \"%1\" was successfully installed").arg(exportTemplate->name()));
    else
        ui->themesErrorMessage->setErrorMessage(tr("There was an error while processing the theme \"%1\"").arg(exportTemplate->name()));
}

void SettingsWindow::onTemplateUninstalled(ExportTemplate *exportTemplate, bool success)
{
    if (success)
        ui->themesErrorMessage->setSuccessMessage(tr("Theme \"%1\" was successfully uninstalled").arg(exportTemplate->name()));
    else
        ui->themesErrorMessage->setErrorMessage(tr("There was an error while processing the theme \"%1\"").arg(exportTemplate->name()));
}

QComboBox *SettingsWindow::comboForMovieScraperInfo(const int &info)
{
    QString currentScraper = m_settings->customMovieScraper().value(info, "notset");

    QComboBox *box = new QComboBox();
    int index = 0;
    if (info != MovieScraperInfos::Title) {
        box->addItem(tr("Don't use"), "");
        box->setItemData(0, info, Qt::UserRole+1);
        index = 1;
    }
    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->identifier() == "custom-movie")
            continue;
        if (scraper->scraperNativelySupports().contains(info)) {
            box->addItem(scraper->name(), scraper->identifier());
            box->setItemData(index, info, Qt::UserRole+1);
            if (scraper->identifier() == currentScraper || (currentScraper == "notset" && index == 1))
                box->setCurrentIndex(index);
            index++;
        }
    }

    QList<int> images;
    images << MovieScraperInfos::Backdrop << MovieScraperInfos::Logo << MovieScraperInfos::ClearArt
           << MovieScraperInfos::CdArt << MovieScraperInfos::Banner << MovieScraperInfos::Thumb
           << MovieScraperInfos::Poster;
    if (images.contains(info)) {
        foreach (ImageProviderInterface *img, Manager::instance()->imageProviders()) {
            if (img->identifier() == "images.fanarttv") {
                box->addItem(img->name(), img->identifier());
                box->setItemData(index, info, Qt::UserRole+1);
                if (img->identifier() == currentScraper || (currentScraper == "notset" && index == 1))
                    box->setCurrentIndex(index);
                index++;
                break;
            }
        }
    }

    return box;
}

QString SettingsWindow::titleForMovieScraperInfo(const int &info)
{
    switch (info) {
    case MovieScraperInfos::Title:
        return tr("Title");
    case MovieScraperInfos::Tagline:
        return tr("Tagline");
    case MovieScraperInfos::Rating:
        return tr("Rating");
    case MovieScraperInfos::Released:
        return tr("Released");
    case MovieScraperInfos::Runtime:
        return tr("Runtime");
    case MovieScraperInfos::Certification:
        return tr("Certification");
    case MovieScraperInfos::Trailer:
        return tr("Trailer");
    case MovieScraperInfos::Overview:
        return tr("Plot");
    case MovieScraperInfos::Poster:
        return tr("Poster");
    case MovieScraperInfos::Backdrop:
        return tr("Fanart");
    case MovieScraperInfos::Actors:
        return tr("Actors");
    case MovieScraperInfos::Genres:
        return tr("Genres");
    case MovieScraperInfos::Studios:
        return tr("Studios");
    case MovieScraperInfos::Countries:
        return tr("Countries");
    case MovieScraperInfos::Writer:
        return tr("Writer");
    case MovieScraperInfos::Director:
        return tr("Director");
    case MovieScraperInfos::Tags:
        return tr("Tags");
    case MovieScraperInfos::Set:
        return tr("Set");
    case MovieScraperInfos::Logo:
        return tr("Logo");
    case MovieScraperInfos::CdArt:
        return tr("Disc Art");
    case MovieScraperInfos::ClearArt:
        return tr("Clear Art");
    case MovieScraperInfos::Banner:
        return tr("Banner");
    case MovieScraperInfos::Thumb:
        return tr("Thumb");
    default:
        return tr("Unsupported");
    }
}

QComboBox *SettingsWindow::comboForTvScraperInfo(const int &info)
{
    QString currentScraper = m_settings->customTvScraper().value(info, "notset");

    QComboBox *box = new QComboBox();
    box->addItem("The TV DB", "tvdb");
    box->setItemData(0, info, Qt::UserRole+1);

    box->addItem("IMDB", "imdb");
    box->setItemData(1, info, Qt::UserRole+1);

    if (currentScraper == "imdb")
        box->setCurrentIndex(1);

    return box;
}

QString SettingsWindow::titleForTvScraperInfo(const int &info)
{
    switch (info) {
    case TvShowScraperInfos::Title:
        return tr("Title");
    case TvShowScraperInfos::Rating:
        return tr("Rating");
    case TvShowScraperInfos::FirstAired:
        return tr("First Aired");
    case TvShowScraperInfos::Runtime:
        return tr("Runtime");
    case TvShowScraperInfos::Director:
        return tr("Director");
    case TvShowScraperInfos::Writer:
        return tr("Writer");
    case TvShowScraperInfos::Certification:
        return tr("Certification");
    case TvShowScraperInfos::Overview:
        return tr("Plot");
    case TvShowScraperInfos::Genres:
        return tr("Genres");
    case TvShowScraperInfos::Actors:
        return tr("Actors");
    default:
        return tr("Unsupported");
    }
}

void SettingsWindow::onChooseUnrar()
{
    QString unrar = QFileDialog::getOpenFileName(this, tr("Choose unrar"), QDir::homePath());
    if (!unrar.isEmpty())
        ui->unrarPath->setText(unrar);
}

void SettingsWindow::onChooseMakeMkvCon()
{
    QString makeMkvCon = QFileDialog::getOpenFileName(this, tr("Choose makemkvcon"), QDir::homePath());
    if (!makeMkvCon.isEmpty())
        ui->makemkvconPath->setText(makeMkvCon);
}

void SettingsWindow::onDirTypeChanged(QComboBox *comboBox)
{
    QComboBox *box = comboBox;
    if (!box)
        box = static_cast<QComboBox*>(QObject::sender());
    if (!box)
        return;

    QTableWidgetItem *itemCheck = box->property("itemCheck").value<Storage*>()->tableWidgetItem();
    QTableWidgetItem *itemCheckReload = box->property("itemCheckReload").value<Storage*>()->tableWidgetItem();

    if (box->currentIndex() == 0) {
        itemCheck->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        itemCheckReload->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    } else if (box->currentIndex() == 1) {
        itemCheck->setFlags(Qt::NoItemFlags);
        itemCheck->setCheckState(Qt::Unchecked);
        itemCheckReload->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    } else if (box->currentIndex() == 2) {
        itemCheck->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        itemCheckReload->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    } else if (box->currentIndex() == 3) {
        itemCheck->setFlags(Qt::NoItemFlags);
        itemCheck->setCheckState(Qt::Unchecked);
        itemCheckReload->setFlags(Qt::NoItemFlags);
        itemCheckReload->setCheckState(Qt::Unchecked);
    } else if (box->currentIndex() == 4) {
        itemCheck->setFlags(Qt::NoItemFlags);
        itemCheck->setCheckState(Qt::Unchecked);
        itemCheckReload->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    }
}

void SettingsWindow::onShowAdultScrapers()
{
    bool show = ui->chkEnableAdultScrapers->isChecked();
    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->isAdult() && scraper->hasSettings()) {
            ui->gridLayoutScrapers->itemAtPosition(m_scraperRows.value(scraper), 0)->widget()->setVisible(show);
            ui->gridLayoutScrapers->itemAtPosition(m_scraperRows.value(scraper), 1)->widget()->setVisible(show);
        }
    }
}

void SettingsWindow::onPluginListUpdated(QList<PluginManager::Plugin> plugins)
{
    int currentRow = ui->pluginList->currentRow();
    if (currentRow < 0)
        currentRow = 0;

    ui->pluginList->blockSignals(true);
    ui->pluginList->clear();
    ui->pluginList->blockSignals(false);
    for (int i=3, n=ui->pluginSettings->count() ; i<n ; ++i)
        ui->pluginSettings->removeWidget(ui->pluginSettings->widget(i));
    ui->btnUninstallPlugin->setVisible(false);
    ui->btnUpdatePlugin->setVisible(false);

    for (int i=0, n=plugins.count() ; i<n ; ++i) {
        int page = 1;
        if (plugins[i].installed) {
            if (plugins[i].plugin->hasSettings())
                page = ui->pluginSettings->addWidget(plugins[i].plugin->settingsWidget());
            else
                page = 1;
        } else {
            page = 2;
        }
        PluginsWidget *widget = new PluginsWidget(ui->pluginList);
        widget->setPlugin(plugins[i]);

        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(widget->sizeHint());
        item->setData(Qt::UserRole, page);
        item->setData(Qt::UserRole+1, i);
        ui->pluginList->addItem(item);
        ui->pluginList->setItemWidget(item, widget);

        if (i==currentRow)
            ui->pluginList->setCurrentItem(item);
    }

    setPluginActionsEnabled(true);
}

void SettingsWindow::onPluginActivated(QListWidgetItem *item)
{
    ui->pluginSettings->setCurrentIndex(item->data(Qt::UserRole).toInt());
    int index = item->data(Qt::UserRole+1).toInt();
    if (index < 0 || index >= PluginManager::instance()->plugins().count())
        return;

    PluginManager::Plugin plugin = PluginManager::instance()->plugins().at(index);
    ui->btnInstallPlugin->setVisible(m_pluginsInstallable && !plugin.installed);
    ui->btnUninstallPlugin->setVisible(m_pluginsInstallable && plugin.installed);
    ui->btnUpdatePlugin->setVisible(m_pluginsInstallable && plugin.updateAvailable);
}

void SettingsWindow::onInstallPlugin()
{
    int index = ui->pluginList->currentItem()->data(Qt::UserRole+1).toInt();
    if (index < 0 || index >= PluginManager::instance()->plugins().count())
        return;
    m_pluginDialog->installPlugin(PluginManager::instance()->plugins().at(index));
}

void SettingsWindow::onUninstallPlugin()
{
    int index = ui->pluginList->currentItem()->data(Qt::UserRole+1).toInt();
    if (index < 0 || index >= PluginManager::instance()->plugins().count())
        return;
    PluginManager::instance()->uninstallPlugin(PluginManager::instance()->plugins().at(index));
}

void SettingsWindow::onUpdatePlugin()
{
    int index = ui->pluginList->currentItem()->data(Qt::UserRole+1).toInt();
    if (index < 0 || index >= PluginManager::instance()->plugins().count())
        return;
    m_pluginDialog->updatePlugin(PluginManager::instance()->plugins().at(index));
}

void SettingsWindow::setPluginActionsEnabled(const bool &enabled)
{
    ui->pluginList->setEnabled(enabled);
    ui->pluginSettings->setEnabled(enabled);
    ui->btnInstallPlugin->setEnabled(enabled);
    ui->btnUninstallPlugin->setEnabled(enabled);
    ui->btnUpdatePlugin->setEnabled(enabled);
}
