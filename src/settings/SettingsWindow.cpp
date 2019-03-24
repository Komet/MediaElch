#include "SettingsWindow.h"
#include "ui_SettingsWindow.h"

#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include "data/Storage.h"
#include "export/ExportTemplate.h"
#include "export/ExportTemplateLoader.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "movies/MovieFilesOrganizer.h"
#include "scrapers/concert/ConcertScraperInterface.h"
#include "scrapers/movie/CustomMovieScraper.h"
#include "scrapers/movie/IMDB.h"
#include "scrapers/movie/MovieScraperInterface.h"
#include "scrapers/tv_show/TheTvDb.h"
#include "scrapers/tv_show/TvScraperInterface.h"
#include "settings/DataFile.h"
#include "settings/ExportTemplateWidget.h"
#include "settings/Settings.h"
#include "ui/notifications/NotificationBox.h"

SettingsWindow::SettingsWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::SettingsWindow),
    m_buttonColor{QColor(128, 129, 132)},
    m_buttonActiveColor{QColor(70, 155, 198)}
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    QFont smallFont = ui->labelGlobal->font();
    smallFont.setPointSize(smallFont.pointSize() - 1);
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
    // ui->stackedWidget->setAnimation(QEasingCurve::Linear);
    // ui->stackedWidget->setSpeed(200);

    m_settings = Settings::instance(this);

    ui->xbmcPort->setValidator(new QIntValidator(0, 99999, ui->xbmcPort));
    ui->dirs->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->dirs->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->exportTemplates->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    int scraperCounter = 0;
    for (auto* scraper : Manager::instance()->movieScrapers()) {
        if (scraper->hasSettings()) {
            QLabel* name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            m_scraperRows.insert(scraper, scraperCounter);
            scraperCounter++;
        }
    }
    for (auto* scraper : Manager::instance()->tvScrapers()) {
        if (scraper->hasSettings()) {
            QLabel* name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            scraperCounter++;
        }
    }
    for (auto* scraper : Manager::instance()->concertScrapers()) {
        if (scraper->hasSettings()) {
            QLabel* name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            scraperCounter++;
        }
    }
    for (auto* scraper : Manager::instance()->musicScrapers()) {
        if (scraper->hasSettings()) {
            QLabel* name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            scraperCounter++;
        }
    }

    for (ImageProviderInterface* scraper : Manager::instance()->imageProviders()) {
        if (scraper->hasSettings()) {
            QLabel* name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            scraperCounter++;
        }
    }

    ui->comboMovieSetArtwork->setItemData(0, static_cast<int>(MovieSetArtworkType::SingleSetFolder));
    ui->comboMovieSetArtwork->setItemData(1, static_cast<int>(MovieSetArtworkType::SingleArtworkFolder));

    Helper::instance()->removeFocusRect(ui->stackedWidget->widget(9));

    // clang-format off
    connect(ui->buttonAddDir,           &QAbstractButton::clicked, this, &SettingsWindow::chooseDirToAdd);
    connect(ui->buttonRemoveDir,        &QAbstractButton::clicked, this, &SettingsWindow::removeDir);
    connect(ui->buttonMovieFilesToDirs, &QAbstractButton::clicked, this, &SettingsWindow::organize);
    connect(ui->dirs,                   &QTableWidget::currentCellChanged, this, &SettingsWindow::dirListRowChanged);
    connect(ui->comboMovieSetArtwork,   SIGNAL(currentIndexChanged(int)),  this, SLOT(onComboMovieSetArtworkChanged()));
    connect(ui->btnMovieSetArtworkDir,  &QAbstractButton::clicked, this, &SettingsWindow::onChooseMovieSetArtworkDir);
    connect(ui->chkUseProxy,            &QAbstractButton::clicked, this, &SettingsWindow::onUseProxy);
    connect(ui->btnCancel,              &QAbstractButton::clicked, this, &SettingsWindow::onCancel);
    connect(ui->btnSave,                &QAbstractButton::clicked, this, &SettingsWindow::onSave);
    connect(ExportTemplateLoader::instance(this), SIGNAL(sigTemplatesLoaded(QVector<ExportTemplate *>)), this, SLOT(onTemplatesLoaded(QVector<ExportTemplate *>)));
    connect(ExportTemplateLoader::instance(this), &ExportTemplateLoader::sigTemplateInstalled,   this, &SettingsWindow::onTemplateInstalled);
    connect(ExportTemplateLoader::instance(this), &ExportTemplateLoader::sigTemplateUninstalled, this, &SettingsWindow::onTemplateUninstalled);
    connect(ui->btnChooseUnrar,         &QAbstractButton::clicked, this, &SettingsWindow::onChooseUnrar);
    connect(ui->btnChooseMakemkvcon,    &QAbstractButton::clicked, this, &SettingsWindow::onChooseMakeMkvCon);
    connect(ui->chkEnableAdultScrapers, &QAbstractButton::clicked, this, &SettingsWindow::onShowAdultScrapers);
    // clang-format on

    ui->movieNfo->setProperty("dataFileType", static_cast<int>(DataFileType::MovieNfo));
    ui->moviePoster->setProperty("dataFileType", static_cast<int>(DataFileType::MoviePoster));
    ui->movieBackdrop->setProperty("dataFileType", static_cast<int>(DataFileType::MovieBackdrop));
    ui->movieCdArt->setProperty("dataFileType", static_cast<int>(DataFileType::MovieCdArt));
    ui->movieClearArt->setProperty("dataFileType", static_cast<int>(DataFileType::MovieClearArt));
    ui->movieLogo->setProperty("dataFileType", static_cast<int>(DataFileType::MovieLogo));
    ui->movieBanner->setProperty("dataFileType", static_cast<int>(DataFileType::MovieBanner));
    ui->movieThumb->setProperty("dataFileType", static_cast<int>(DataFileType::MovieThumb));
    ui->movieSetPosterFileName->setProperty("dataFileType", static_cast<int>(DataFileType::MovieSetPoster));
    ui->movieSetFanartFileName->setProperty("dataFileType", static_cast<int>(DataFileType::MovieSetBackdrop));
    ui->showBackdrop->setProperty("dataFileType", static_cast<int>(DataFileType::TvShowBackdrop));
    ui->showBanner->setProperty("dataFileType", static_cast<int>(DataFileType::TvShowBanner));
    ui->showCharacterArt->setProperty("dataFileType", static_cast<int>(DataFileType::TvShowCharacterArt));
    ui->showClearArt->setProperty("dataFileType", static_cast<int>(DataFileType::TvShowClearArt));
    ui->showEpisodeNfo->setProperty("dataFileType", static_cast<int>(DataFileType::TvShowEpisodeNfo));
    ui->showEpisodeThumbnail->setProperty("dataFileType", static_cast<int>(DataFileType::TvShowEpisodeThumb));
    ui->showLogo->setProperty("dataFileType", static_cast<int>(DataFileType::TvShowLogo));
    ui->showThumb->setProperty("dataFileType", static_cast<int>(DataFileType::TvShowThumb));
    ui->showNfo->setProperty("dataFileType", static_cast<int>(DataFileType::TvShowNfo));
    ui->showPoster->setProperty("dataFileType", static_cast<int>(DataFileType::TvShowPoster));
    ui->showSeasonBackdrop->setProperty("dataFileType", static_cast<int>(DataFileType::TvShowSeasonBackdrop));
    ui->showSeasonBanner->setProperty("dataFileType", static_cast<int>(DataFileType::TvShowSeasonBanner));
    ui->showSeasonPoster->setProperty("dataFileType", static_cast<int>(DataFileType::TvShowSeasonPoster));
    ui->showSeasonThumb->setProperty("dataFileType", static_cast<int>(DataFileType::TvShowSeasonThumb));
    ui->concertNfo->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertNfo));
    ui->concertPoster->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertPoster));
    ui->concertBackdrop->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertBackdrop));
    ui->concertLogo->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertLogo));
    ui->concertClearArt->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertClearArt));
    ui->concertDiscArt->setProperty("dataFileType", static_cast<int>(DataFileType::ConcertCdArt));
    ui->artistFanart->setProperty("dataFileType", static_cast<int>(DataFileType::ArtistFanart));
    ui->artistLogo->setProperty("dataFileType", static_cast<int>(DataFileType::ArtistLogo));
    ui->artistThumb->setProperty("dataFileType", static_cast<int>(DataFileType::ArtistThumb));
    ui->albumThumb->setProperty("dataFileType", static_cast<int>(DataFileType::AlbumThumb));
    ui->albumDiscArt->setProperty("dataFileType", static_cast<int>(DataFileType::AlbumCdArt));

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
    for (QAction* action : findChildren<QAction*>()) {
        if (!action->property("page").isValid()) {
            continue;
        }
        action->setIcon(Manager::instance()->iconFont()->icon(action->property("iconName").toString(), m_buttonColor));
    }
    ui->actionGlobal->setIcon(
        Manager::instance()->iconFont()->icon(ui->actionGlobal->property("iconName").toString(), m_buttonActiveColor));

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
    if (Settings::instance()->settingsWindowSize().isValid()
        && !Settings::instance()->settingsWindowPosition().isNull()) {
        move(Settings::instance()->settingsWindowPosition());
        resize(Settings::instance()->settingsWindowSize());
    }
    QMainWindow::show();
}

void SettingsWindow::closeEvent(QCloseEvent* event)
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
    auto triggeredAction = static_cast<QAction*>(sender());
    for (QAction* action : ui->toolBar->actions())
        action->setIcon(Manager::instance()->iconFont()->icon(action->property("iconName").toString(), m_buttonColor));
    triggeredAction->setIcon(
        Manager::instance()->iconFont()->icon(triggeredAction->property("iconName").toString(), m_buttonActiveColor));
    // ui->stackedWidget->slideInIdx(triggeredAction->property("page").toInt());
    ui->stackedWidget->setCurrentIndex(triggeredAction->property("page").toInt());
}

void SettingsWindow::loadSettings()
{
    m_settings->loadSettings();

    // Stream Details
    ui->chkAutoLoadStreamDetails->setChecked(m_settings->autoLoadStreamDetails());

    // Proxy
    const auto& netSettings = m_settings->networkSettings();
    ui->chkUseProxy->setChecked(netSettings.useProxy());
    ui->proxyType->setCurrentIndex(netSettings.proxyType());
    ui->proxyHost->setText(netSettings.proxyHost());
    ui->proxyPort->setValue(netSettings.proxyPort());
    ui->proxyUsername->setText(netSettings.proxyUsername());
    ui->proxyPassword->setText(netSettings.proxyPassword());
    onUseProxy();

    ui->usePlotForOutline->setChecked(m_settings->usePlotForOutline());
    ui->chkDownloadActorImages->setChecked(m_settings->downloadActorImages());
    ui->chkIgnoreArticlesWhenSorting->setChecked(m_settings->ignoreArticlesWhenSorting());
    ui->chkCheckForUpdates->setChecked(m_settings->checkForUpdates());
    ui->chkEnableAdultScrapers->setChecked(m_settings->showAdultScrapers());
    onShowAdultScrapers();

    for (int i = 0, n = ui->comboStartupSection->count(); i < n; ++i) {
        if (ui->comboStartupSection->itemData(i, Qt::UserRole) == m_settings->startupSection()) {
            ui->comboStartupSection->setCurrentIndex(i);
            break;
        }
    }

    // Directories
    ui->dirs->setRowCount(0);
    ui->dirs->clearContents();
    QVector<SettingsDir> movieDirectories = m_settings->directorySettings().movieDirectories();
    for (int i = 0, n = movieDirectories.count(); i < n; ++i) {
        addDir(movieDirectories.at(i).path,
            movieDirectories.at(i).separateFolders,
            movieDirectories.at(i).autoReload,
            SettingsDirType::Movies);
    }
    QVector<SettingsDir> tvShowDirectories = m_settings->directorySettings().tvShowDirectories();
    for (int i = 0, n = tvShowDirectories.count(); i < n; ++i) {
        addDir(tvShowDirectories.at(i).path,
            tvShowDirectories.at(i).separateFolders,
            tvShowDirectories.at(i).autoReload,
            SettingsDirType::TvShows);
    }
    QVector<SettingsDir> concertDirectories = m_settings->directorySettings().concertDirectories();
    for (int i = 0, n = concertDirectories.count(); i < n; ++i) {
        addDir(concertDirectories.at(i).path,
            concertDirectories.at(i).separateFolders,
            concertDirectories.at(i).autoReload,
            SettingsDirType::Concerts);
    }
    QVector<SettingsDir> downloadDirectories = m_settings->directorySettings().downloadDirectories();
    for (int i = 0, n = downloadDirectories.count(); i < n; ++i) {
        addDir(downloadDirectories.at(i).path, false, false, SettingsDirType::Downloads);
    }
    QVector<SettingsDir> musicDirectories = m_settings->directorySettings().musicDirectories();
    for (int i = 0, n = musicDirectories.count(); i < n; ++i) {
        addDir(musicDirectories.at(i).path,
            musicDirectories.at(i).separateFolders,
            musicDirectories.at(i).autoReload,
            SettingsDirType::Music);
    }

    dirListRowChanged(ui->dirs->currentRow());

    // Exclude words
    ui->excludeWordsText->setPlainText(m_settings->excludeWords());

    ui->useYoutubePluginUrls->setChecked(m_settings->useYoutubePluginUrls());

    // XBMC
    ui->xbmcHost->setText(m_settings->kodiSettings().xbmcHost());
    if (m_settings->kodiSettings().xbmcPort() != 0) {
        ui->xbmcPort->setText(QString::number(m_settings->kodiSettings().xbmcPort()));
    } else {
        ui->xbmcPort->clear();
    }
    ui->xbmcUser->setText(m_settings->kodiSettings().xbmcUser());
    ui->xbmcPassword->setText(m_settings->kodiSettings().xbmcPassword());

    // Movie set artwork
    for (int i = 0, n = ui->comboMovieSetArtwork->count(); i < n; ++i) {
        if (MovieSetArtworkType(ui->comboMovieSetArtwork->itemData(i).toInt()) == m_settings->movieSetArtworkType()) {
            ui->comboMovieSetArtwork->setCurrentIndex(i);
            break;
        }
    }
    ui->movieSetArtworkDir->setText(m_settings->movieSetArtworkDirectory());
    onComboMovieSetArtworkChanged();

    for (auto lineEdit : findChildren<QLineEdit*>()) {
        if (lineEdit->property("dataFileType").isNull()) {
            continue;
        }
        DataFileType dataFileType = DataFileType(lineEdit->property("dataFileType").toInt());
        QVector<DataFile> dataFiles = m_settings->dataFiles(dataFileType);
        QStringList filenames;
        for (DataFile dataFile : dataFiles) {
            filenames << dataFile.fileName();
        }
        lineEdit->setText(filenames.join(","));
    }

    QVector<MovieScraperInfos> infos = {MovieScraperInfos::Title,
        MovieScraperInfos::Set,
        MovieScraperInfos::Tagline,
        MovieScraperInfos::Rating,
        MovieScraperInfos::Released,
        MovieScraperInfos::Runtime,
        MovieScraperInfos::Director,
        MovieScraperInfos::Writer,
        MovieScraperInfos::Certification,
        MovieScraperInfos::Trailer,
        MovieScraperInfos::Overview,
        MovieScraperInfos::Poster,
        MovieScraperInfos::Backdrop,
        MovieScraperInfos::Actors,
        MovieScraperInfos::Genres,
        MovieScraperInfos::Studios,
        MovieScraperInfos::Countries,
        MovieScraperInfos::Logo,
        MovieScraperInfos::ClearArt,
        MovieScraperInfos::CdArt,
        MovieScraperInfos::Banner,
        MovieScraperInfos::Thumb};

    ui->customScraperTable->clearContents();
    ui->customScraperTable->setRowCount(0);

    for (const auto info : infos) {
        int row = ui->customScraperTable->rowCount();
        ui->customScraperTable->insertRow(row);
        ui->customScraperTable->setItem(row, 0, new QTableWidgetItem(titleForMovieScraperInfo(info)));
        ui->customScraperTable->setCellWidget(row, 1, comboForMovieScraperInfo(info));
    }


    QVector<TvShowScraperInfos> tvInfos = {TvShowScraperInfos::Title,
        TvShowScraperInfos::Rating,
        TvShowScraperInfos::FirstAired,
        TvShowScraperInfos::Runtime,
        TvShowScraperInfos::Director,
        TvShowScraperInfos::Writer,
        TvShowScraperInfos::Certification,
        TvShowScraperInfos::Overview,
        TvShowScraperInfos::Genres,
        TvShowScraperInfos::Tags,
        TvShowScraperInfos::Actors};

    ui->tvScraperTable->clearContents();
    ui->tvScraperTable->setRowCount(0);

    for (const auto info : tvInfos) {
        int row = ui->tvScraperTable->rowCount();
        ui->tvScraperTable->insertRow(row);
        ui->tvScraperTable->setItem(row, 0, new QTableWidgetItem(titleForTvScraperInfo(info)));
        ui->tvScraperTable->setCellWidget(row, 1, comboForTvScraperInfo(info));
    }

    ui->chkDeleteArchives->setChecked(m_settings->deleteArchives());
    ui->unrarPath->setText(m_settings->importSettings().unrar());
    ui->makemkvconPath->setText(m_settings->importSettings().makeMkvCon());

    ui->artistExtraFanarts->setValue(m_settings->extraFanartsMusicArtists());
}

void SettingsWindow::saveSettings()
{
    QVector<DataFile> dataFiles;
    for (QLineEdit* lineEdit : findChildren<QLineEdit*>()) {
        if (lineEdit->property("dataFileType").isNull()) {
            continue;
        }
        int pos = 0;
        DataFileType dataFileType = DataFileType(lineEdit->property("dataFileType").toInt());
        QStringList filenames = lineEdit->text().split(",", QString::SkipEmptyParts);
        for (const QString& filename : filenames) {
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
    m_settings->setStartupSection(
        ui->comboStartupSection->itemData(ui->comboStartupSection->currentIndex()).toString());

    m_settings->kodiSettings().setXbmcHost(ui->xbmcHost->text());
    m_settings->kodiSettings().setXbmcPort(ui->xbmcPort->text().toInt());
    m_settings->kodiSettings().setXbmcUser(ui->xbmcUser->text());
    m_settings->kodiSettings().setXbmcPassword(ui->xbmcPassword->text());

    // Proxy
    m_settings->networkSettings().setUseProxy(ui->chkUseProxy->isChecked());
    m_settings->networkSettings().setProxyType(ui->proxyType->currentIndex());
    m_settings->networkSettings().setProxyHost(ui->proxyHost->text());
    m_settings->networkSettings().setProxyPort(ui->proxyPort->value());
    m_settings->networkSettings().setProxyUsername(ui->proxyUsername->text());
    m_settings->networkSettings().setProxyPassword(ui->proxyPassword->text());

    m_settings->setUsePlotForOutline(ui->usePlotForOutline->isChecked());

    // Save Directories
    QVector<SettingsDir> movieDirectories;
    QVector<SettingsDir> tvShowDirectories;
    QVector<SettingsDir> concertDirectories;
    QVector<SettingsDir> downloadDirectories;
    QVector<SettingsDir> musicDirectories;
    for (int row = 0, n = ui->dirs->rowCount(); row < n; ++row) {
        SettingsDir dir;
        dir.path = ui->dirs->item(row, 1)->text();
        dir.separateFolders = ui->dirs->item(row, 2)->checkState() == Qt::Checked;
        dir.autoReload = ui->dirs->item(row, 3)->checkState() == Qt::Checked;
        if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() == 0) {
            movieDirectories.append(dir);
        } else if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() == 1) {
            tvShowDirectories.append(dir);
        } else if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() == 2) {
            concertDirectories.append(dir);
        } else if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() == 3) {
            downloadDirectories.append(dir);
        } else if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() == 4) {
            musicDirectories.append(dir);
        }
    }
    m_settings->directorySettings().setMovieDirectories(movieDirectories);
    m_settings->directorySettings().setTvShowDirectories(tvShowDirectories);
    m_settings->directorySettings().setConcertDirectories(concertDirectories);
    m_settings->directorySettings().setDownloadDirectories(downloadDirectories);
    m_settings->directorySettings().setMusicDirectories(musicDirectories);

    // exclude words
    m_settings->setExcludeWords(ui->excludeWordsText->toPlainText());

    // Movie set artwork
    m_settings->setMovieSetArtworkType(static_cast<MovieSetArtworkType>(
        ui->comboMovieSetArtwork->itemData(ui->comboMovieSetArtwork->currentIndex()).toInt()));
    m_settings->setMovieSetArtworkDirectory(ui->movieSetArtworkDir->text());

    // Custom movie scraper
    QMap<MovieScraperInfos, QString> customMovieScraper;
    for (int row = 0, n = ui->customScraperTable->rowCount(); row < n; ++row) {
        auto box = static_cast<QComboBox*>(ui->customScraperTable->cellWidget(row, 1));
        MovieScraperInfos info = MovieScraperInfos(box->itemData(0, Qt::UserRole + 1).toInt());
        QString scraper = box->itemData(box->currentIndex()).toString();
        customMovieScraper.insert(info, scraper);
    }
    m_settings->setCustomMovieScraper(customMovieScraper);

    // tv scraper
    QMap<TvShowScraperInfos, QString> tvScraper;
    for (int row = 0, n = ui->tvScraperTable->rowCount(); row < n; ++row) {
        auto box = static_cast<QComboBox*>(ui->tvScraperTable->cellWidget(row, 1));
        TvShowScraperInfos info = TvShowScraperInfos(box->itemData(0, Qt::UserRole + 1).toInt());
        QString scraper = box->itemData(box->currentIndex()).toString();
        tvScraper.insert(info, scraper);
    }
    m_settings->setCustomTvScraper(tvScraper);

    // Downloads
    m_settings->importSettings().setUnrar(ui->unrarPath->text());
    m_settings->importSettings().setMakeMkvCon(ui->makemkvconPath->text());
    m_settings->setDeleteArchives(ui->chkDeleteArchives->isChecked());

    m_settings->setExtraFanartsMusicArtists(ui->artistExtraFanarts->value());

    m_settings->saveSettings();

    Manager::instance()->movieFileSearcher()->setMovieDirectories(m_settings->directorySettings().movieDirectories());
    Manager::instance()->tvShowFileSearcher()->setTvShowDirectories(
        m_settings->directorySettings().tvShowDirectories());
    Manager::instance()->concertFileSearcher()->setConcertDirectories(
        m_settings->directorySettings().concertDirectories());
    Manager::instance()->musicFileSearcher()->setMusicDirectories(m_settings->directorySettings().musicDirectories());
    NotificationBox::instance()->showMessage(tr("Settings saved"));
}

void SettingsWindow::addDir(QString dir, bool separateFolders, bool autoReload, SettingsDirType dirType)
{
    dir = QDir::toNativeSeparators(dir);
    if (!dir.isEmpty()) {
        bool exists = false;
        for (int i = 0, n = ui->dirs->rowCount(); i < n; ++i) {
            if (ui->dirs->item(i, 1)->text() == dir) {
                exists = true;
            }
        }

        if (!exists) {
            int row = ui->dirs->rowCount();
            ui->dirs->insertRow(row);
            auto item = new QTableWidgetItem(dir);
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setToolTip(dir);
            auto itemCheck = new QTableWidgetItem();
            if (separateFolders) {
                itemCheck->setCheckState(Qt::Checked);
            } else {
                itemCheck->setCheckState(Qt::Unchecked);
            }

            auto itemCheckReload = new QTableWidgetItem();
            if (autoReload) {
                itemCheckReload->setCheckState(Qt::Checked);
            } else {
                itemCheckReload->setCheckState(Qt::Unchecked);
            }

            auto box = new QComboBox();
            box->setProperty("itemCheck", Storage::toVariant(box, itemCheck));
            box->setProperty("itemCheckReload", Storage::toVariant(box, itemCheckReload));
            box->addItems(
                QStringList() << tr("Movies") << tr("TV Shows") << tr("Concerts") << tr("Downloads") << tr("Music"));
            if (dirType == SettingsDirType::Movies) {
                box->setCurrentIndex(0);
            } else if (dirType == SettingsDirType::TvShows) {
                box->setCurrentIndex(1);
            } else if (dirType == SettingsDirType::Concerts) {
                box->setCurrentIndex(2);
            } else if (dirType == SettingsDirType::Downloads) {
                box->setCurrentIndex(3);
            } else if (dirType == SettingsDirType::Music) {
                box->setCurrentIndex(4);
            }

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
    if (row < 0) {
        return;
    }
    ui->dirs->removeRow(row);
}

void SettingsWindow::organize()
{
    auto organizer = new MovieFilesOrganizer(this);

    int row = ui->dirs->currentRow();
    if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() != 0
        || ui->dirs->item(row, 2)->checkState() == Qt::Checked) {
        organizer->canceled(tr("Organizing movies does only work on "
                               "movies, not already sorted to "
                               "separate folders."));
        return;
    }

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(tr("Are you sure?"));
    msgBox.setInformativeText(
        tr("This operation sorts all movies in this directory to separate "
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
    case QMessageBox::Cancel: break;
    default: break;
    }
}

void SettingsWindow::dirListRowChanged(int currentRow)
{
    if (currentRow < 0 || currentRow >= ui->dirs->rowCount()) {
        ui->buttonRemoveDir->setDisabled(true);
        ui->buttonMovieFilesToDirs->setDisabled(true);
    } else {
        ui->buttonRemoveDir->setDisabled(false);
        if (ui->dirs->cellWidget(currentRow, 0) != nullptr
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
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Choose a directory containing your movies, TV show or concerts"), QDir::homePath());
    if (!dir.isEmpty()) {
        addDir(dir);
    }
}

void SettingsWindow::onComboMovieSetArtworkChanged()
{
    MovieSetArtworkType value =
        MovieSetArtworkType(ui->comboMovieSetArtwork->itemData(ui->comboMovieSetArtwork->currentIndex()).toInt());
    ui->btnMovieSetArtworkDir->setEnabled(value == MovieSetArtworkType::SingleArtworkFolder);
    ui->movieSetArtworkDir->setEnabled(value == MovieSetArtworkType::SingleArtworkFolder);

    if (value == MovieSetArtworkType::SingleArtworkFolder) {
        ui->movieSetPosterFileName->setText("<setName>-folder.jpg");
        ui->movieSetFanartFileName->setText("<setName>-fanart.jpg");
    } else if (value == MovieSetArtworkType::SingleSetFolder) {
        ui->movieSetPosterFileName->setText("folder.jpg");
        ui->movieSetFanartFileName->setText("fanart.jpg");
    }
}

void SettingsWindow::onChooseMovieSetArtworkDir()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Choose a directory where your movie set artwork is stored"), QDir::homePath());
    if (!dir.isEmpty()) {
        ui->movieSetArtworkDir->setText(dir);
    }
}

void SettingsWindow::loadRemoteTemplates()
{
    ExportTemplateLoader::instance()->getRemoteTemplates();
}

void SettingsWindow::onTemplatesLoaded(QVector<ExportTemplate*> templates)
{
    ui->exportTemplates->clearContents();
    ui->exportTemplates->setRowCount(0);

    for (ExportTemplate* exportTemplate : templates) {
        auto widget = new ExportTemplateWidget(ui->exportTemplates);
        widget->setExportTemplate(exportTemplate);

        int row = ui->exportTemplates->rowCount();
        ui->exportTemplates->insertRow(row);
        ui->exportTemplates->setCellWidget(row, 0, widget);
        widget->adjustSize();
    }
}

void SettingsWindow::onTemplateInstalled(ExportTemplate* exportTemplate, bool success)
{
    if (success) {
        ui->themesErrorMessage->setSuccessMessage(
            tr("Theme \"%1\" was successfully installed").arg(exportTemplate->name()));
    } else {
        ui->themesErrorMessage->setErrorMessage(
            tr("There was an error while processing the theme \"%1\"").arg(exportTemplate->name()));
    }
}

void SettingsWindow::onTemplateUninstalled(ExportTemplate* exportTemplate, bool success)
{
    if (success) {
        ui->themesErrorMessage->setSuccessMessage(
            tr("Theme \"%1\" was successfully uninstalled").arg(exportTemplate->name()));
    } else {
        ui->themesErrorMessage->setErrorMessage(
            tr("There was an error while processing the theme \"%1\"").arg(exportTemplate->name()));
    }
}

QComboBox* SettingsWindow::comboForMovieScraperInfo(const MovieScraperInfos info)
{
    QString currentScraper = m_settings->customMovieScraper().value(info, "notset");

    auto box = new QComboBox();
    int index = 0;
    if (info != MovieScraperInfos::Title) {
        box->addItem(tr("Don't use"), "");
        box->setItemData(0, static_cast<int>(info), Qt::UserRole + 1);
        index = 1;
    }
    for (auto* scraper : Manager::instance()->movieScrapers()) {
        if (scraper->identifier() == CustomMovieScraper::scraperIdentifier) {
            continue;
        }
        if (scraper->scraperNativelySupports().contains(info)) {
            box->addItem(scraper->name(), scraper->identifier());
            box->setItemData(index, static_cast<int>(info), Qt::UserRole + 1);
            if (scraper->identifier() == currentScraper || (currentScraper == "notset" && index == 1)) {
                box->setCurrentIndex(index);
            }
            index++;
        }
    }

    QVector<MovieScraperInfos> images{MovieScraperInfos::Backdrop,
        MovieScraperInfos::Logo,
        MovieScraperInfos::ClearArt,
        MovieScraperInfos::CdArt,
        MovieScraperInfos::Banner,
        MovieScraperInfos::Thumb,
        MovieScraperInfos::Poster};

    if (images.contains(info)) {
        for (const auto img : Manager::instance()->imageProviders()) {
            if (img->identifier() == "images.fanarttv") {
                box->addItem(img->name(), img->identifier());
                box->setItemData(index, static_cast<int>(info), Qt::UserRole + 1);
                if (img->identifier() == currentScraper || (currentScraper == "notset" && index == 1)) {
                    box->setCurrentIndex(index);
                }
                index++;
                break;
            }
        }
    }

    return box;
}

QString SettingsWindow::titleForMovieScraperInfo(MovieScraperInfos info)
{
    switch (info) {
    case MovieScraperInfos::Title: return tr("Title");
    case MovieScraperInfos::Tagline: return tr("Tagline");
    case MovieScraperInfos::Rating: return tr("Rating");
    case MovieScraperInfos::Released: return tr("Released");
    case MovieScraperInfos::Runtime: return tr("Runtime");
    case MovieScraperInfos::Certification: return tr("Certification");
    case MovieScraperInfos::Trailer: return tr("Trailer");
    case MovieScraperInfos::Overview: return tr("Plot");
    case MovieScraperInfos::Poster: return tr("Poster");
    case MovieScraperInfos::Backdrop: return tr("Fanart");
    case MovieScraperInfos::Actors: return tr("Actors");
    case MovieScraperInfos::Genres: return tr("Genres");
    case MovieScraperInfos::Studios: return tr("Studios");
    case MovieScraperInfos::Countries: return tr("Countries");
    case MovieScraperInfos::Writer: return tr("Writer");
    case MovieScraperInfos::Director: return tr("Director");
    case MovieScraperInfos::Tags: return tr("Tags");
    case MovieScraperInfos::Set: return tr("Set");
    case MovieScraperInfos::Logo: return tr("Logo");
    case MovieScraperInfos::CdArt: return tr("Disc Art");
    case MovieScraperInfos::ClearArt: return tr("Clear Art");
    case MovieScraperInfos::Banner: return tr("Banner");
    case MovieScraperInfos::Thumb: return tr("Thumb");
    default: return tr("Unsupported");
    }
}

QComboBox* SettingsWindow::comboForTvScraperInfo(const TvShowScraperInfos info)
{
    QString currentScraper = m_settings->customTvScraper().value(info, "notset");

    auto box = new QComboBox();
    box->addItem("The TV DB", TheTvDb::scraperIdentifier);
    box->setItemData(0, static_cast<int>(info), Qt::UserRole + 1);

    box->addItem("IMDB", IMDB::scraperIdentifier);
    box->setItemData(1, static_cast<int>(info), Qt::UserRole + 1);

    if (currentScraper == IMDB::scraperIdentifier) {
        box->setCurrentIndex(1);
    }

    return box;
}

QString SettingsWindow::titleForTvScraperInfo(const TvShowScraperInfos info)
{
    switch (info) {
    case TvShowScraperInfos::Title: return tr("Title");
    case TvShowScraperInfos::Rating: return tr("Rating");
    case TvShowScraperInfos::FirstAired: return tr("First Aired");
    case TvShowScraperInfos::Runtime: return tr("Runtime");
    case TvShowScraperInfos::Director: return tr("Director");
    case TvShowScraperInfos::Writer: return tr("Writer");
    case TvShowScraperInfos::Certification: return tr("Certification");
    case TvShowScraperInfos::Overview: return tr("Plot");
    case TvShowScraperInfos::Genres: return tr("Genres");
    case TvShowScraperInfos::Tags: return tr("Tags");
    case TvShowScraperInfos::Actors: return tr("Actors");
    default: return tr("Unsupported");
    }
}

void SettingsWindow::onChooseUnrar()
{
    QString unrar = QFileDialog::getOpenFileName(this, tr("Choose unrar"), QDir::homePath());
    if (!unrar.isEmpty()) {
        ui->unrarPath->setText(unrar);
    }
}

void SettingsWindow::onChooseMakeMkvCon()
{
    QString makeMkvCon = QFileDialog::getOpenFileName(this, tr("Choose makemkvcon"), QDir::homePath());
    if (!makeMkvCon.isEmpty()) {
        ui->makemkvconPath->setText(makeMkvCon);
    }
}

void SettingsWindow::onDirTypeChanged(QComboBox* comboBox)
{
    QComboBox* box = comboBox;
    if (!box) {
        box = static_cast<QComboBox*>(QObject::sender());
    }
    if (!box) {
        return;
    }

    QTableWidgetItem* itemCheck = box->property("itemCheck").value<Storage*>()->tableWidgetItem();
    QTableWidgetItem* itemCheckReload = box->property("itemCheckReload").value<Storage*>()->tableWidgetItem();

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
    for (const auto* scraper : Manager::instance()->movieScrapers()) {
        if (scraper->isAdult() && scraper->hasSettings()) {
            ui->gridLayoutScrapers->itemAtPosition(m_scraperRows.value(scraper), 0)->widget()->setVisible(show);
            ui->gridLayoutScrapers->itemAtPosition(m_scraperRows.value(scraper), 1)->widget()->setVisible(show);
        }
    }
}
