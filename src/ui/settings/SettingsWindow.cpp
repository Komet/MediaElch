#include "SettingsWindow.h"
#include "ui_SettingsWindow.h"

#include <QAction>
#include <QDebug>
#include <QFileDialog>

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "scrapers/concert/ConcertScraperInterface.h"
#include "scrapers/movie/CustomMovieScraper.h"
#include "scrapers/movie/IMDB.h"
#include "scrapers/movie/MovieScraperInterface.h"
#include "scrapers/tv_show/TheTvDb.h"
#include "scrapers/tv_show/TvScraperInterface.h"
#include "settings/DataFile.h"
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

    ui->settingsTabs->setCurrentIndex(0);

    m_settings = Settings::instance(this);
    ui->globalSettings->setSettings(*m_settings);
    ui->exportSettings->setSettings(*m_settings);
    ui->importSettings->setSettings(*m_settings);

    ui->xbmcPort->setValidator(new QIntValidator(0, 99999, ui->xbmcPort));

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

    Helper::removeFocusRect(ui->settingsTabs->widget(9));

    // clang-format off
    connect(ui->comboMovieSetArtwork,   SIGNAL(currentIndexChanged(int)),  this, SLOT(onComboMovieSetArtworkChanged()));
    connect(ui->btnMovieSetArtworkDir,  &QAbstractButton::clicked, this, &SettingsWindow::onChooseMovieSetArtworkDir);
    connect(ui->chkUseProxy,            &QAbstractButton::clicked, this, &SettingsWindow::onUseProxy);
    connect(ui->btnCancel,              &QAbstractButton::clicked, this, &SettingsWindow::onCancel);
    connect(ui->btnSave,                &QAbstractButton::clicked, this, &SettingsWindow::onSave);
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
    ui->exportSettings->show();
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
    for (QAction* action : ui->toolBar->actions()) {
        action->setIcon(Manager::instance()->iconFont()->icon(action->property("iconName").toString(), m_buttonColor));
    }
    triggeredAction->setIcon(
        Manager::instance()->iconFont()->icon(triggeredAction->property("iconName").toString(), m_buttonActiveColor));
    ui->settingsTabs->setCurrentIndex(triggeredAction->property("page").toInt());
}

void SettingsWindow::loadSettings()
{
    m_settings->loadSettings();
    ui->globalSettings->loadSettings();
    ui->exportSettings->loadSettings();
    ui->importSettings->loadSettings();

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
    ui->chkEnableAdultScrapers->setChecked(m_settings->showAdultScrapers());
    onShowAdultScrapers();

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
    m_settings->setShowAdultScrapers(ui->chkEnableAdultScrapers->isChecked());

    ui->globalSettings->saveSettings();
    ui->exportSettings->saveSettings();
    ui->importSettings->saveSettings();

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

    m_settings->setExtraFanartsMusicArtists(ui->artistExtraFanarts->value());

    m_settings->saveSettings();

    auto* manager = Manager::instance();
    auto& dirs = m_settings->directorySettings();
    manager->movieFileSearcher()->setMovieDirectories(dirs.movieDirectories());
    manager->tvShowFileSearcher()->setTvShowDirectories(dirs.tvShowDirectories());
    manager->concertFileSearcher()->setConcertDirectories(dirs.concertDirectories());
    manager->musicFileSearcher()->setMusicDirectories(dirs.musicDirectories());
    NotificationBox::instance()->showMessage(tr("Settings saved"));
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
