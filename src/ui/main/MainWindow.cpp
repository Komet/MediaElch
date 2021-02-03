#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "data/Storage.h"
#include "file/NameFormatter.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "media_centers/MediaCenterInterface.h"
#include "scrapers/movie/MovieScraper.h"
#include "settings/Settings.h"
#include "tv_shows/TvShowUpdater.h"
#include "ui/concerts/ConcertSearch.h"
#include "ui/export/CsvExportDialog.h"
#include "ui/main/Update.h"
#include "ui/media_centers/KodiSync.h"
#include "ui/movies/MovieSearch.h"
#include "ui/music/MusicMultiScrapeDialog.h"
#include "ui/music/MusicSearch.h"
#include "ui/notifications/NotificationBox.h"
#include "ui/notifications/Notificator.h"

#include <QCheckBox>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QMessageBox>
#include <QPainter>
#include <QShortcut>
#include <QTimer>
#include <QToolBar>

#ifdef Q_OS_MAC
#    include <QMenuBar>
#endif

MainWindow* MainWindow::m_instance = nullptr;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
#ifdef Q_OS_MACOS
    auto* macMenuBar = new QMenuBar();
    QMenu* menu = macMenuBar->addMenu("File");
    QAction* mAbout = menu->addAction("About");
    mAbout->setMenuRole(QAction::AboutRole);
    auto* aboutDialog = new AboutDialog(this);
    connect(mAbout, &QAction::triggered, aboutDialog, &AboutDialog::exec);

    QMenu* help = macMenuBar->addMenu("Help");
    const auto addHelpUrl = [help](const QString& str, const QString& url) {
        auto* action = help->addAction(str);
        connect(action, &QAction::triggered, [url]() { QDesktopServices::openUrl(QUrl(url, QUrl::StrictMode)); });
    };

    addHelpUrl(tr("FAQ"), "https://mediaelch.github.io/mediaelch-doc/faq.html");
    addHelpUrl(tr("Troubleshooting"), "https://mediaelch.github.io/mediaelch-doc/troubleshooting.html");
    addHelpUrl(tr("Report Issue"), "https://mediaelch.github.io/mediaelch-doc/contributing/bug-reports.html");
    help->addSeparator();
    addHelpUrl(tr("Release Notes"), "https://mediaelch.github.io/mediaelch-doc/release-notes.html");
    addHelpUrl(tr("Documentation"), "https://mediaelch.github.io/mediaelch-doc/");
    addHelpUrl(tr("Blog"), "https://mediaelch.github.io/mediaelch-blog/posts/");
    addHelpUrl(tr("Official Kodi Forum"), "https://forum.kodi.tv/");
    help->addSeparator();
    addHelpUrl(tr("View License"), "https://mediaelch.github.io/mediaelch-doc/license.html");
#endif

    ui->setupUi(this);
    setMinimumHeight(500);

    MainWindow::m_instance = this;
    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    qInfo() << "MediaElch version" << QApplication::applicationVersion() << "starting up";

    QMap<MainActions, bool> allActions;
    allActions.insert(MainActions::Search, false);
    allActions.insert(MainActions::Save, false);
    allActions.insert(MainActions::SaveAll, false);
    allActions.insert(MainActions::FilterWidget, false);
    allActions.insert(MainActions::Rename, false);
    allActions.insert(MainActions::Export, false);

    // initialize all widgets with all actions set to false
    m_actions.insert(MainWidgets::Movies, allActions);
    m_actions.insert(MainWidgets::MovieSets, allActions);
    m_actions.insert(MainWidgets::TvShows, allActions);
    m_actions.insert(MainWidgets::Concerts, allActions);
    m_actions.insert(MainWidgets::Music, allActions);
    m_actions.insert(MainWidgets::Genres, allActions);
    m_actions.insert(MainWidgets::Certifications, allActions);
    m_actions.insert(MainWidgets::Downloads, allActions);

    // enable filtering for some widgets
    m_actions[MainWidgets::Movies][MainActions::FilterWidget] = true;
    m_actions[MainWidgets::TvShows][MainActions::FilterWidget] = true;
    m_actions[MainWidgets::Concerts][MainActions::FilterWidget] = true;
    m_actions[MainWidgets::Music][MainActions::FilterWidget] = true;

    m_settings = Settings::instance(this);
    m_aboutDialog = new AboutDialog(this);
    m_supportDialog = new SupportDialog(this);
    m_settingsWindow = new SettingsWindow(this);
    m_fileScannerDialog = new FileScannerDialog(this);
    m_xbmcSync = new KodiSync(Settings::instance()->kodiSettings(), this);
    m_renamer = new RenamerDialog(this);
    setupToolbar();

    NotificationBox::instance(this)->reposition(this->size());
    Manager::instance();
    Notificator::instance(nullptr, ui->centralWidget);

    if (!m_settings->mainSplitterState().isNull()) {
        ui->movieSplitter->restoreState(m_settings->mainSplitterState());
        ui->tvShowSplitter->restoreState(m_settings->mainSplitterState());
        ui->setsWidget->splitter()->restoreState(m_settings->mainSplitterState());
        ui->concertSplitter->restoreState(m_settings->mainSplitterState());
        ui->genreWidget->splitter()->restoreState(m_settings->mainSplitterState());
        ui->certificationWidget->splitter()->restoreState(m_settings->mainSplitterState());
        ui->musicSplitter->restoreState(m_settings->mainSplitterState());
    } else {
        QList<int> size{200, 600};
        ui->movieSplitter->setSizes(size);
        ui->tvShowSplitter->setSizes(size);
        ui->setsWidget->splitter()->setSizes(size);
        ui->concertSplitter->setSizes(size);
        ui->genreWidget->splitter()->setSizes(size);
        ui->certificationWidget->splitter()->setSizes(size);
        ui->musicSplitter->setSizes(size);
    }

    if (m_settings->mainWindowSize().isValid() && !m_settings->mainWindowPosition().isNull()) {
        resize(m_settings->mainWindowSize());
        move(m_settings->mainWindowPosition());
#ifdef Q_OS_WIN
        if (m_settings->mainWindowMaximized()) {
            showMaximized();
        }
#endif
    }
    // Size for Screenshots
    // resize(1200, 676);

    const auto onMenuFromSender = [this]() {
        auto* button = dynamic_cast<QToolButton*>(QObject::sender());
        if (button == nullptr) {
            return;
        }
        onMenu(button);
    };

    m_buttonActiveColor = QColor(70, 155, 198);
    m_buttonColor = QColor(128, 129, 132);
    for (QToolButton* btn : ui->menuWidget->findChildren<QToolButton*>()) {
        connect(btn, &QToolButton::clicked, this, onMenuFromSender);
        btn->setIcon(Manager::instance()->iconFont()->icon(btn->property("iconName").toString(), m_buttonColor));
    }

    // clang-format off
    connect(ui->movieFilesWidget, &MovieFilesWidget::movieSelected,   ui->movieWidget, &MovieWidget::setMovie);
    connect(ui->movieFilesWidget, &MovieFilesWidget::movieSelected,   ui->movieWidget, &MovieWidget::setEnabledTrue);
    connect(ui->movieFilesWidget, &MovieFilesWidget::noMovieSelected, ui->movieWidget, &MovieWidget::clear);
    connect(ui->movieFilesWidget, &MovieFilesWidget::noMovieSelected, ui->movieWidget, &MovieWidget::setDisabledTrue);
    connect(ui->movieFilesWidget, &MovieFilesWidget::sigStartSearch,  this,            &MainWindow::onActionSearch);

    connect(ui->concertFilesWidget, &ConcertFilesWidget::concertSelected,   ui->concertWidget, &ConcertWidget::setConcert);
    connect(ui->concertFilesWidget, &ConcertFilesWidget::concertSelected,   ui->concertWidget, &ConcertWidget::setEnabledTrue);
    connect(ui->concertFilesWidget, &ConcertFilesWidget::noConcertSelected, ui->concertWidget, &ConcertWidget::clear);
    connect(ui->concertFilesWidget, &ConcertFilesWidget::noConcertSelected, ui->concertWidget, &ConcertWidget::setDisabledTrue);

    connect(ui->musicFilesWidget, &MusicFilesWidget::sigArtistSelected,  ui->musicWidget, &MusicWidget::onArtistSelected);
    connect(ui->musicFilesWidget, &MusicFilesWidget::sigAlbumSelected,   ui->musicWidget, &MusicWidget::onAlbumSelected);
    connect(ui->musicFilesWidget, &MusicFilesWidget::sigArtistSelected,  ui->musicWidget, &MusicWidget::onArtistSetEnabledTrue);
    connect(ui->musicFilesWidget, &MusicFilesWidget::sigAlbumSelected,   ui->musicWidget, &MusicWidget::onAlbumSetEnabledTrue);
    connect(ui->musicFilesWidget, &MusicFilesWidget::sigNothingSelected, ui->musicWidget, &MusicWidget::onClear);
    connect(ui->musicFilesWidget, &MusicFilesWidget::sigNothingSelected, ui->musicWidget, &MusicWidget::onSetDisabledTrue);

    connect(ui->tvShowFilesWidget, &TvShowFilesWidget::sigTvShowSelected,       ui->tvShowWidget, &TvShowWidget::onTvShowSelected);
    connect(ui->tvShowFilesWidget, &TvShowFilesWidget::sigSeasonSelected,       ui->tvShowWidget, &TvShowWidget::onSeasonSelected);
    connect(ui->tvShowFilesWidget, &TvShowFilesWidget::sigEpisodeSelected,      ui->tvShowWidget, &TvShowWidget::onEpisodeSelected);
    connect(ui->tvShowFilesWidget, &TvShowFilesWidget::sigNothingSelected,      ui->tvShowWidget, &TvShowWidget::onClear);
    connect(ui->tvShowFilesWidget, &TvShowFilesWidget::sigNothingSelected,      ui->tvShowWidget, &TvShowWidget::onSetDisabledTrue);
    connect(ui->tvShowFilesWidget, &TvShowFilesWidget::sigStartSearch,          this,             &MainWindow::onActionSearch);
    connect(ui->tvShowFilesWidget, &TvShowFilesWidget::sigTvShowSelected,       this, [this](TvShow* show){
        ui->tvShowWidget->onTvShowSetEnabledTrue(show);
    });

    connect(ui->tvShowFilesWidget, &TvShowFilesWidget::sigEpisodeSelected,      ui->tvShowWidget, &TvShowWidget::onEpisodeSetEnabledTrue);
    connect(ui->tvShowFilesWidget, &TvShowFilesWidget::sigSeasonSelected,
            ui->tvShowWidget, elchOverload<TvShow *, SeasonNumber>(&TvShowWidget::onTvShowSetEnabledTrue));

    connect(ui->movieWidget, &MovieWidget::actorDownloadProgress, this, &MainWindow::progressProgress);
    connect(ui->movieWidget, &MovieWidget::actorDownloadStarted,  this, &MainWindow::progressStarted);
    connect(ui->movieWidget, &MovieWidget::actorDownloadFinished, this, &MainWindow::progressFinished);

    connect(ui->tvShowWidget, &TvShowWidget::sigDownloadsStarted,  this, &MainWindow::progressStarted);
    connect(ui->tvShowWidget, &TvShowWidget::sigDownloadsProgress, this, &MainWindow::progressProgress);
    connect(ui->tvShowWidget, &TvShowWidget::sigDownloadsFinished, this, &MainWindow::progressFinished);

    connect(ui->musicWidget, &MusicWidget::sigDownloadsStarted,  this, &MainWindow::progressStarted);
    connect(ui->musicWidget, &MusicWidget::sigDownloadsProgress, this, &MainWindow::progressProgress);
    connect(ui->musicWidget, &MusicWidget::sigDownloadsFinished, this, &MainWindow::progressFinished);

    connect(ui->navbar, &Navbar::sigFilterChanged, this, &MainWindow::onFilterChanged);

    connect(ui->movieSplitter,                   &QSplitter::splitterMoved, this, &MainWindow::moveSplitter);
    connect(ui->tvShowSplitter,                  &QSplitter::splitterMoved, this, &MainWindow::moveSplitter);
    connect(ui->setsWidget->splitter(),          &QSplitter::splitterMoved, this, &MainWindow::moveSplitter);
    connect(ui->genreWidget->splitter(),         &QSplitter::splitterMoved, this, &MainWindow::moveSplitter);
    connect(ui->certificationWidget->splitter(), &QSplitter::splitterMoved, this, &MainWindow::moveSplitter);
    connect(ui->concertSplitter,                 &QSplitter::splitterMoved, this, &MainWindow::moveSplitter);
    connect(ui->musicSplitter,                   &QSplitter::splitterMoved, this, &MainWindow::moveSplitter);

    connect(Manager::instance()->tvShowFileSearcher(), &TvShowFileSearcher::tvShowsLoaded, [this]() { ui->tvShowFilesWidget->renewModel(true); });
    connect(Manager::instance()->tvShowFileSearcher(), &TvShowFileSearcher::tvShowsLoaded, this, &MainWindow::updateTvShows);
    connect(m_fileScannerDialog,                       &QDialog::accepted,                 this, &MainWindow::setNewMarks);
    connect(ui->downloadsWidget,                       &DownloadsWidget::sigScanFinished,  this, &MainWindow::setNewMarks);

    connect(m_xbmcSync, &KodiSync::sigTriggerReload, this, &MainWindow::onTriggerReloadAll);
    connect(m_xbmcSync, &KodiSync::sigFinished,      this, &MainWindow::onKodiSyncFinished);

    connect(m_renamer, &RenamerDialog::sigFilesRenamed, this, &MainWindow::onFilesRenamed);

    connect(m_settingsWindow, &SettingsWindow::sigSaved, this, &MainWindow::onRenewModels, Qt::QueuedConnection);

    connect(ui->setsWidget,            &SetsWidget::sigJumpToMovie,          this, &MainWindow::onJumpToMovie);
    connect(ui->certificationWidget,   &CertificationWidget::sigJumpToMovie, this, &MainWindow::onJumpToMovie);
    connect(ui->genreWidget,           &GenreWidget::sigJumpToMovie,         this, &MainWindow::onJumpToMovie);
    connect(ui->movieDuplicatesWidget, &MovieDuplicates::sigJumpToMovie,     this, &MainWindow::onJumpToMovie);
    // clang-format on

#ifdef Q_OS_WIN
    setStyleSheet(styleSheet() + " #centralWidget { border-bottom: 1px solid rgba(0, 0, 0, 100); } ");

    QFont font = ui->labelMovies->font();
    font.setPointSize(font.pointSize() - 3);
    font.setBold(true);
    ui->labelMovies->setFont(font);
    ui->labelConcerts->setFont(font);
    ui->labelShows->setFont(font);
    ui->labelMusic->setFont(font);
    ui->labelDownloads->setFont(font);

    for (QToolButton* btn : ui->menuWidget->findChildren<QToolButton*>()) {
        btn->setIconSize(QSize(32, 32));
    }
    ui->navbar->setFixedHeight(56);
#endif

    if (Settings::instance()->startupSection() == "tvshows") {
        onMenu(ui->buttonTvshows);
    } else if (Settings::instance()->startupSection() == "concerts") {
        onMenu(ui->buttonConcerts);
    } else if (Settings::instance()->startupSection() == "music") {
        onMenu(ui->buttonMusic);
    } else if (Settings::instance()->startupSection() == "import") {
        onMenu(ui->buttonDownloads);
    } else {
        onMenu(ui->buttonMovies);
    }

    // hack. without only the fileScannerDialog pops up and blocks until it has finished
    show();

    // Start scanning for files
    QTimer::singleShot(0, m_fileScannerDialog, &FileScannerDialog::exec);

#ifdef MEDIAELCH_UPDATER
    if (Settings::instance()->checkForUpdates()) {
        qInfo() << "Searching for updates";
        Update::instance()->checkForUpdate();
    }
#else
    qDebug() << "Updater is disabled";
#endif
}

/**
 * \brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
    delete ui;
}

MainWindow* MainWindow::instance()
{
    return MainWindow::m_instance;
}

/**
 * \brief Repositions the MessageBox
 */
void MainWindow::resizeEvent(QResizeEvent* event)
{
    bool isBigWindow = event->size().width() >= 1500;
    ui->movieWidget->setBigWindow(isBigWindow);
    ui->tvShowWidget->setBigWindow(isBigWindow);
    ui->concertWidget->setBigWindow(isBigWindow);
    ui->musicWidget->setBigWindow(isBigWindow);

    NotificationBox::instance()->reposition(event->size());
    QWidget::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* /*event*/)
{
    m_settings->setMainWindowSize(size());
    m_settings->setMainWindowPosition(pos());
    m_settings->setMainSplitterState(ui->movieSplitter->saveState());
    m_settings->setMainWindowMaximized(isMaximized());
}

void MainWindow::setupToolbar()
{
    // clang-format off
    connect(ui->navbar, &Navbar::sigSearch,    this,             &MainWindow::onActionSearch);
    connect(ui->navbar, &Navbar::sigSave,      this,             &MainWindow::onActionSave);
    connect(ui->navbar, &Navbar::sigSaveAll,   this,             &MainWindow::onActionSaveAll);
    connect(ui->navbar, &Navbar::sigReload,    this,             &MainWindow::onActionReload);
    connect(ui->navbar, &Navbar::sigAbout,     m_aboutDialog,    &AboutDialog::exec);
    connect(ui->navbar, &Navbar::sigSettings,  m_settingsWindow, &SettingsWindow::show);
    connect(ui->navbar, &Navbar::sigLike,      m_supportDialog,  &QDialog::exec);
    connect(ui->navbar, &Navbar::sigSync,      this,             &MainWindow::onActionXbmc);
    connect(ui->navbar, &Navbar::sigRename,    this,             &MainWindow::onActionRename);
    // clang-format on

    connect(ui->navbar, &Navbar::sigExport, this, [this]() {
        auto* exportDialog = new ExportDialog(this);
        exportDialog->exec();
        exportDialog->deleteLater();
    });

    // TODO: There is currently no GUI-way to do this.
    QShortcut* shortcut = new QShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_E, this);
    QObject::connect(shortcut, &QShortcut::activated, this, [this]() {
        auto* csvExportDialog = new CsvExportDialog(*m_settings, this);
        csvExportDialog->exec();
        csvExportDialog->deleteLater();
    });

    ui->navbar->setActionSearchEnabled(false);
    ui->navbar->setActionSaveEnabled(false);
    ui->navbar->setActionSaveAllEnabled(false);
    ui->navbar->setActionRenameEnabled(false);
}

/**
 * \brief Called when a subwidget starts a progress, displays a progress MessageBox
 * \param msg Message
 * \param id (Unique) Id of the progress
 */
void MainWindow::progressStarted(QString msg, int id)
{
    qDebug() << "Entered, msg=" << msg << "id=" << id;
    NotificationBox::instance()->showProgressBar(msg, id);
}

/**
 * \brief Updates the progress MessageBox
 * \param current Current value
 * \param max Maximum value
 * \param id (Unique) Id of the progress
 */
void MainWindow::progressProgress(int current, int max, int id)
{
    NotificationBox::instance()->progressBarProgress(current, max, id);
}

/**
 * \brief Called when a progress has finished
 * \param id (Unique) Id of the progress
 */
void MainWindow::progressFinished(int id)
{
    NotificationBox::instance()->hideProgressBar(id);
}

/**
 * \brief Called when the action "Search" was clicked
 * Delegates the event down to the current subwidget
 */
void MainWindow::onActionSearch()
{
    qDebug() << "Entered, currentIndex=" << ui->stackedWidget->currentIndex();
    if (ui->stackedWidget->currentIndex() == 0) {
        if (ui->movieFilesWidget->selectedMovies().count() > 1) {
            ui->movieFilesWidget->multiScrape();
        } else {
            QTimer::singleShot(0, ui->movieWidget, &MovieWidget::startScraperSearch);
        }
    } else if (ui->stackedWidget->currentIndex() == 1) {
        if (ui->tvShowFilesWidget->selectedEpisodes(false).count() + ui->tvShowFilesWidget->selectedShows().count()
            > 1) {
            ui->tvShowFilesWidget->multiScrape();
        } else {
            QTimer::singleShot(0, ui->tvShowWidget, &TvShowWidget::onStartScraperSearch);
        }
    } else if (ui->stackedWidget->currentIndex() == 3) {
        QTimer::singleShot(0, ui->concertWidget, &ConcertWidget::onStartScraperSearch);
    } else if (ui->stackedWidget->currentIndex() == 7) {
        if ((ui->musicFilesWidget->selectedArtists().count() + ui->musicFilesWidget->selectedAlbums().count()) > 1) {
            ui->musicFilesWidget->multiScrape();
        } else {
            QTimer::singleShot(0, ui->musicWidget, &MusicWidget::onStartScraperSearch);
        }
    }
}

/**
 * \brief Called when the action "Save" was clicked
 * Delegates the event down to the current subwidget
 */
void MainWindow::onActionSave()
{
    qDebug() << "Entered, currentIndex=" << ui->stackedWidget->currentIndex();
    if (ui->stackedWidget->currentIndex() == 0) {
        ui->movieWidget->saveInformation();
    } else if (ui->stackedWidget->currentIndex() == 1) {
        ui->tvShowWidget->onSaveInformation();
    } else if (ui->stackedWidget->currentIndex() == 2) {
        ui->setsWidget->saveSet();
    } else if (ui->stackedWidget->currentIndex() == 3) {
        ui->concertWidget->onSaveInformation();
    } else if (ui->stackedWidget->currentIndex() == 4) {
        ui->genreWidget->onSaveInformation();
    } else if (ui->stackedWidget->currentIndex() == 5) {
        ui->certificationWidget->onSaveInformation();
    } else if (ui->stackedWidget->currentIndex() == 7) {
        ui->musicWidget->onSaveInformation();
    }
    setNewMarks();
}

/**
 * \brief Called when the action "Save all" was clicked
 * Delegates the event down to the current subwidget
 */
void MainWindow::onActionSaveAll()
{
    qDebug() << "Entered, currentIndex=" << ui->stackedWidget->currentIndex();
    if (ui->stackedWidget->currentIndex() == 0) {
        ui->movieWidget->saveAll();
    } else if (ui->stackedWidget->currentIndex() == 1) {
        ui->tvShowWidget->onSaveAll();
    } else if (ui->stackedWidget->currentIndex() == 3) {
        ui->concertWidget->onSaveAll();
    } else if (ui->stackedWidget->currentIndex() == 7) {
        ui->musicWidget->onSaveAll();
    }
    setNewMarks();
}

/**
 * \brief Executes the file scanner dialog
 */
void MainWindow::onActionReload()
{
    if (ui->stackedWidget->currentIndex() == 6) {
        ui->downloadsWidget->scanDownloadFolders();
        return;
    }

    m_fileScannerDialog->setForceReload(true);

    if (ui->stackedWidget->currentIndex() == 0) {
        m_fileScannerDialog->setReloadType(FileScannerDialog::ReloadType::Movies);
    } else if (ui->stackedWidget->currentIndex() == 1) {
        m_fileScannerDialog->setReloadType(FileScannerDialog::ReloadType::TvShows);
    } else if (ui->stackedWidget->currentIndex() == 3) {
        m_fileScannerDialog->setReloadType(FileScannerDialog::ReloadType::Concerts);
    } else if (ui->stackedWidget->currentIndex() == 7) {
        m_fileScannerDialog->setReloadType(FileScannerDialog::ReloadType::Music);
    }

    m_fileScannerDialog->exec();
}

void MainWindow::onActionRename()
{
    if (ui->stackedWidget->currentIndex() == 0) {
        m_renamer->setRenameType(Renamer::RenameType::Movies);
        m_renamer->setMovies(ui->movieFilesWidget->selectedMovies());

    } else if (ui->stackedWidget->currentIndex() == 1) {
        m_renamer->setRenameType(Renamer::RenameType::TvShows);
        m_renamer->setShows(ui->tvShowFilesWidget->selectedShows());
        m_renamer->setEpisodes(ui->tvShowFilesWidget->selectedEpisodes());

    } else if (ui->stackedWidget->currentIndex() == 3) {
        m_renamer->setRenameType(Renamer::RenameType::Concerts);
        m_renamer->setConcerts(ui->concertFilesWidget->selectedConcerts());

    } else {
        return;
    }
    m_renamer->exec();
}

/**
 * \brief Called when the filter text was changed or a filter was added/removed
 * Delegates the event down to the current subwidget
 * \param filters List of filters
 * \param text Filter text
 */
void MainWindow::onFilterChanged(QVector<Filter*> filters, QString text)
{
    if (ui->stackedWidget->currentIndex() == 0) {
        ui->movieFilesWidget->setFilter(filters, text);
    } else if (ui->stackedWidget->currentIndex() == 1) {
        ui->tvShowFilesWidget->setFilter(filters, text);
    } else if (ui->stackedWidget->currentIndex() == 3) {
        ui->concertFilesWidget->setFilter(filters, text);
    } else if (ui->stackedWidget->currentIndex() == 7) {
        ui->musicFilesWidget->setFilter(filters, text);
    }
}

/**
 * \brief Sets the status of the save and save all action
 * \param enabled Status
 * \param widget Widget to set the status for
 */
void MainWindow::onSetSaveEnabled(bool enabled, MainWidgets widget)
{
    qDebug() << "Entered, enabled=" << enabled;

    m_actions[widget][MainActions::Save] = enabled;

    if (widget != MainWidgets::MovieSets && widget != MainWidgets::Certifications) {
        m_actions[widget][MainActions::SaveAll] = enabled;
        if (widget != MainWidgets::Music) {
            m_actions[widget][MainActions::Rename] = enabled;
        }
    }

    if ((widget == MainWidgets::Movies && ui->stackedWidget->currentIndex() == 0)
        || (widget == MainWidgets::TvShows && ui->stackedWidget->currentIndex() == 1)
        || (widget == MainWidgets::Music && ui->stackedWidget->currentIndex() == 7)
        || (widget == MainWidgets::Concerts && ui->stackedWidget->currentIndex() == 3)) {
        ui->navbar->setActionSaveEnabled(enabled);
        ui->navbar->setActionSaveAllEnabled(enabled);
        if (widget != MainWidgets::Music) {
            ui->navbar->setActionRenameEnabled(enabled);
        }
    }
    if ((widget == MainWidgets::MovieSets && ui->stackedWidget->currentIndex() == 2)
        || (widget == MainWidgets::Certifications && ui->stackedWidget->currentIndex() == 5)
        || (widget == MainWidgets::Genres && ui->stackedWidget->currentIndex() == 4)) {
        ui->navbar->setActionSaveEnabled(enabled);
    }
}

/**
 * \brief Sets the status of the search action
 * \param enabled Status
 * \param widget Widget to set the status for
 */
void MainWindow::onSetSearchEnabled(bool enabled, MainWidgets widget)
{
    qDebug() << "[MainWindow] Search field:" << (enabled ? "enabled" : "disabled");
    m_actions[widget][MainActions::Search] = enabled;

    if ((widget == MainWidgets::Movies && ui->stackedWidget->currentIndex() == 0)
        || (widget == MainWidgets::TvShows && ui->stackedWidget->currentIndex() == 1)
        || (widget == MainWidgets::Concerts && ui->stackedWidget->currentIndex() == 3)
        || (widget == MainWidgets::Music && ui->stackedWidget->currentIndex() == 7)) {
        ui->navbar->setActionSearchEnabled(enabled);
    }
}

/**
 * \brief Moves all splitters
 */
void MainWindow::moveSplitter(int pos, int index)
{
    Q_UNUSED(index)
    QList<int> sizes;
    QList<QSplitter*> splitters;
    splitters << ui->movieSplitter << ui->tvShowSplitter << ui->setsWidget->splitter() << ui->genreWidget->splitter()
              << ui->certificationWidget->splitter() << ui->concertSplitter << ui->musicSplitter;
    for (const QSplitter* splitter : splitters) {
        if (splitter->sizes().at(0) == pos) {
            sizes = splitter->sizes();
            break;
        }
    }

    for (QSplitter* splitter : splitters) {
        splitter->setSizes(sizes);
    }

    Manager::instance()->movieModel()->update();
    Manager::instance()->concertModel()->update();
}

/// \brief Sets or removes the new mark in the main menu on the left
void MainWindow::setNewMarks()
{
    auto* mngr = Manager::instance();

    auto setMark = [&](QToolButton* btn, int count) {
        if (btn != nullptr) {
            const QColor color = btn->property("isActive").toBool() ? m_buttonActiveColor : m_buttonColor;
            const QString iconStar = (count > 0) ? "star" : "";
            const QString iconName = btn->property("iconName").toString();
            btn->setIcon(mngr->iconFont()->icon(iconName, color, iconStar, count));
        }
    };

    setMark(ui->buttonMovies, mngr->movieModel()->countNewMovies());
    setMark(ui->buttonTvshows, mngr->tvShowModel()->hasNewShowOrEpisode());
    setMark(ui->buttonConcerts, mngr->concertModel()->countNewConcerts());
    setMark(ui->buttonMusic, mngr->musicModel()->hasNewArtistsOrAlbums());
    setMark(ui->buttonDownloads, ui->downloadsWidget->hasNewItems());

    ui->movieFilesWidget->setAlphaListData();
    ui->concertFilesWidget->setAlphaListData();
}

void MainWindow::onActionXbmc()
{
    m_xbmcSync->exec();
}

void MainWindow::onTriggerReloadAll()
{
    m_fileScannerDialog->setForceReload(true);
    m_fileScannerDialog->setReloadType(FileScannerDialog::ReloadType::All);
    m_fileScannerDialog->exec();
}

void MainWindow::onKodiSyncFinished()
{
    ui->movieFilesWidget->movieSelectedEmitter();
    ui->tvShowFilesWidget->emitLastSelection();
    ui->concertFilesWidget->concertSelectedEmitter();
}

void MainWindow::onFilesRenamed(Renamer::RenameType type)
{
    if (m_renamer->renameErrorOccured()) {
        m_fileScannerDialog->setForceReload(true);
        if (type == Renamer::RenameType::Movies) {
            m_fileScannerDialog->setReloadType(FileScannerDialog::ReloadType::Movies);
        } else if (type == Renamer::RenameType::Concerts) {
            m_fileScannerDialog->setReloadType(FileScannerDialog::ReloadType::Concerts);
        } else if (type == Renamer::RenameType::TvShows) {
            m_fileScannerDialog->setReloadType(FileScannerDialog::ReloadType::TvShows);
        } else if (type == Renamer::RenameType::All) {
            m_fileScannerDialog->setReloadType(FileScannerDialog::ReloadType::All);
        }
        m_fileScannerDialog->exec();
    } else {
        if (type == Renamer::RenameType::Movies) {
            ui->movieWidget->updateMovieInfo();
        } else if (type == Renamer::RenameType::Concerts) {
            ui->concertWidget->updateConcertInfo();
        } else if (type == Renamer::RenameType::TvShows) {
            ui->tvShowWidget->updateInfo();
        }
    }
}

void MainWindow::onRenewModels()
{
    ui->movieFilesWidget->renewModel();
    ui->tvShowFilesWidget->renewModel();
    ui->concertFilesWidget->renewModel();
    ui->downloadsWidget->scanDownloadFolders();
}

void MainWindow::onJumpToMovie(Movie* movie)
{
    onMenu(ui->buttonMovies);
    ui->movieFilesWidget->selectMovie(movie);
}

void MainWindow::updateTvShows()
{
    for (TvShow* show : Manager::instance()->tvShowModel()->tvShows()) {
        if (show->showMissingEpisodes()) {
            TvShowUpdater::instance()->updateShow(show);
        }
    }
}

void MainWindow::onMenu(QToolButton* button)
{
    for (QToolButton* btn : ui->menuWidget->findChildren<QToolButton*>()) {
        btn->setIcon(Manager::instance()->iconFont()->icon(btn->property("iconName").toString(), m_buttonColor));
        btn->setProperty("isActive", false);
    }
    button->setIcon(
        Manager::instance()->iconFont()->icon(button->property("iconName").toString(), m_buttonActiveColor));
    button->setProperty("isActive", true);
    setNewMarks();

    int page = button->property("page").toInt();
    ui->stackedWidget->setCurrentIndex(page);

    ui->navbar->setActionReloadEnabled(page == 0 || page == 1 || page == 3 || page == 6 || page == 7);
    MainWidgets widget = MainWidgets::Movies;
    switch (page) {
    case 0:
        // Movies
        ui->navbar->setReloadToolTip(
            tr("Reload all Movies (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));
        widget = MainWidgets::Movies;
        break;
    case 1:
        // Tv Shows
        ui->navbar->setReloadToolTip(
            tr("Reload all TV Shows (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));
        widget = MainWidgets::TvShows;
        break;
    case 2:
        // Movie Sets
        widget = MainWidgets::MovieSets;
        ui->setsWidget->loadSets();
        break;
    case 3:
        // Concerts
        ui->navbar->setReloadToolTip(
            tr("Reload all Concerts (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));
        widget = MainWidgets::Concerts;
        break;
    case 4:
        // Genres
        widget = MainWidgets::Genres;
        ui->genreWidget->loadGenres();
        break;
    case 5:
        // Certification
        widget = MainWidgets::Certifications;
        ui->certificationWidget->loadCertifications();
        break;
    case 6:
        // Import
        widget = MainWidgets::Downloads;
        ui->navbar->setReloadToolTip(tr("Reload all Downloads (%1)")
                                         .arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));
        break;
    case 7:
        // Music
        ui->navbar->setReloadToolTip(
            tr("Reload Music (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));
        widget = MainWidgets::Music;
        break;
    default: qWarning() << "Unhandled page in main window."; break;
    }

    ui->navbar->setActionSearchEnabled(m_actions[widget][MainActions::Search]);
    ui->navbar->setActionSaveEnabled(m_actions[widget][MainActions::Save]);
    ui->navbar->setActionSaveAllEnabled(m_actions[widget][MainActions::SaveAll]);
    ui->navbar->setActionRenameEnabled(m_actions[widget][MainActions::Rename]);
    ui->navbar->setFilterWidgetEnabled(m_actions[widget][MainActions::FilterWidget]);
    ui->navbar->setActiveWidget(widget);
}
