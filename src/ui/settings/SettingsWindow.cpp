#include "SettingsWindow.h"
#include "ui_SettingsWindow.h"

#include <QAction>
#include <QDebug>
#include <QFileDialog>

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
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

    ui->settingsTabs->setCurrentIndex(0);

    m_settings = Settings::instance(this);
    ui->globalSettings->setSettings(*m_settings);
    ui->exportSettings->setSettings(*m_settings);
    ui->importSettings->setSettings(*m_settings);
    ui->scraperSettings->setSettings(*m_settings);

    ui->xbmcPort->setValidator(new QIntValidator(0, 99999, ui->xbmcPort));

    ui->comboMovieSetArtwork->setItemData(0, static_cast<int>(MovieSetArtworkType::SingleSetFolder));
    ui->comboMovieSetArtwork->setItemData(1, static_cast<int>(MovieSetArtworkType::SingleArtworkFolder));

    Helper::removeFocusRect(ui->settingsTabs->widget(9));

    // clang-format off
    connect(ui->comboMovieSetArtwork,   SIGNAL(currentIndexChanged(int)),  this, SLOT(onComboMovieSetArtworkChanged()));
    connect(ui->btnMovieSetArtworkDir,  &QAbstractButton::clicked, this, &SettingsWindow::onChooseMovieSetArtworkDir);
    connect(ui->chkUseProxy,            &QAbstractButton::clicked, this, &SettingsWindow::onUseProxy);
    connect(ui->btnCancel,              &QAbstractButton::clicked, this, &SettingsWindow::onCancel);
    connect(ui->btnSave,                &QAbstractButton::clicked, this, &SettingsWindow::onSave);
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
    ui->scraperSettings->loadSettings();

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

    ui->globalSettings->saveSettings();
    ui->exportSettings->saveSettings();
    ui->importSettings->saveSettings();
    ui->scraperSettings->saveSettings();

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
