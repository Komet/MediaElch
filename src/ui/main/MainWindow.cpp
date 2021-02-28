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
#include "ui/main/QuickOpen.h"
#include "ui/main/Update.h"
#include "ui/media_centers/KodiSync.h"
#include "ui/movies/MovieMultiScrapeDialog.h"
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
    m_aboutDialog = new AboutDialog(this);
#ifdef Q_OS_MACOS
    auto* macMenuBar = new QMenuBar();
    QMenu* menu = macMenuBar->addMenu("File");
    QAction* mAbout = menu->addAction("About");
    mAbout->setMenuRole(QAction::AboutRole);
    connect(mAbout, &QAction::triggered, m_aboutDialog, &AboutDialog::exec);

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
    resize(1200, 676);

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
    qInfo() << "Updater is disabled; MediaElch will not check for updates!";
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

    auto* commandModelAction = new QAction("Test", this);
    commandModelAction->setIcon(QIcon::fromTheme(QStringLiteral("quickopen")));
    commandModelAction->setText(tr("&Quick Open"));
    commandModelAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
    connect(commandModelAction, &QAction::triggered, this, &MainWindow::onCommandBarOpen);
    addAction(commandModelAction);
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
    MainWidgets current = currentTab();

    if (current == MainWidgets::Movies) {
        if (ui->movieFilesWidget->selectedMovies().count() > 1) {
            ui->movieFilesWidget->multiScrape();
        } else {
            QTimer::singleShot(0, ui->movieWidget, &MovieWidget::startScraperSearch);
        }

    } else if (current == MainWidgets::TvShows) {
        if (ui->tvShowFilesWidget->selectedEpisodes(false).count() + ui->tvShowFilesWidget->selectedShows().count()
            > 1) {
            ui->tvShowFilesWidget->multiScrape();
        } else {
            QTimer::singleShot(0, ui->tvShowWidget, &TvShowWidget::onStartScraperSearch);
        }

    } else if (current == MainWidgets::Concerts) {
        QTimer::singleShot(0, ui->concertWidget, &ConcertWidget::onStartScraperSearch);

    } else if (current == MainWidgets::Music) {
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

    switch (currentTab()) {
    case MainWidgets::Movies: ui->movieWidget->saveInformation(); break;
    case MainWidgets::MovieSets: ui->setsWidget->saveSet(); break;
    case MainWidgets::Genres: ui->genreWidget->onSaveInformation(); break;
    case MainWidgets::Certifications: ui->certificationWidget->onSaveInformation(); break;
    case MainWidgets::TvShows: ui->tvShowWidget->onSaveInformation(); break;
    case MainWidgets::Concerts: ui->concertWidget->onSaveInformation(); break;
    case MainWidgets::Music: ui->musicWidget->onSaveInformation(); break;
    case MainWidgets::Duplicates: break; // Duplicates section does not have "save"
    case MainWidgets::Downloads: break;  // Downloads section does not have "save"
    }

    setNewMarks();
}

void MainWindow::onActionSaveAll()
{
    switch (currentTab()) {
    case MainWidgets::Movies: ui->movieWidget->saveAll(); break;
    case MainWidgets::TvShows: ui->tvShowWidget->onSaveAll(); break;
    case MainWidgets::Concerts: ui->concertWidget->onSaveAll(); break;
    case MainWidgets::Music: ui->musicWidget->onSaveAll(); break;
    case MainWidgets::MovieSets: break;      // not supported, yet
    case MainWidgets::Genres: break;         // not supported, yet
    case MainWidgets::Certifications: break; // not supported, yet
    case MainWidgets::Duplicates: break;     // not supported, yet
    case MainWidgets::Downloads: break;      // Downloads section does not have "save"
    }

    setNewMarks();
}

void MainWindow::onActionReload()
{
    MainWidgets current = currentTab();

    if (current == MainWidgets::Downloads) {
        ui->downloadsWidget->scanDownloadFolders();
        return;
    }

    FileScannerDialog::ReloadType type = FileScannerDialog::ReloadType::Movies;

    switch (current) {
    case MainWidgets::Movies: type = FileScannerDialog::ReloadType::Movies; break;
    case MainWidgets::TvShows: type = FileScannerDialog::ReloadType::TvShows; break;
    case MainWidgets::Concerts: type = FileScannerDialog::ReloadType::Concerts; break;
    case MainWidgets::Music: type = FileScannerDialog::ReloadType::Music; break;
    case MainWidgets::MovieSets:         // not supported, yet
    case MainWidgets::Genres:            // not supported, yet
    case MainWidgets::Certifications:    // not supported, yet
    case MainWidgets::Duplicates:        // not supported, yet
    case MainWidgets::Downloads: return; // already handled; no reload
    }

    m_fileScannerDialog->setForceReload(true);
    m_fileScannerDialog->setReloadType(type);
    m_fileScannerDialog->exec();
}

void MainWindow::onActionRename()
{
    switch (currentTab()) {
    case MainWidgets::Movies: {
        m_renamer->setRenameType(Renamer::RenameType::Movies);
        m_renamer->setMovies(ui->movieFilesWidget->selectedMovies());
        break;
    }
    case MainWidgets::TvShows: {
        m_renamer->setRenameType(Renamer::RenameType::TvShows);
        m_renamer->setShows(ui->tvShowFilesWidget->selectedShows());
        m_renamer->setEpisodes(ui->tvShowFilesWidget->selectedEpisodes());
        break;
    }
    case MainWidgets::Concerts: {
        m_renamer->setRenameType(Renamer::RenameType::Concerts);
        m_renamer->setConcerts(ui->concertFilesWidget->selectedConcerts());
        break;
    }
    case MainWidgets::Music:             // not supported, yet
    case MainWidgets::MovieSets:         // not supported, yet
    case MainWidgets::Genres:            // not supported, yet
    case MainWidgets::Certifications:    // not supported, yet
    case MainWidgets::Duplicates:        // not supported
    case MainWidgets::Downloads: return; // not supported -> return
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
    switch (currentTab()) {
    case MainWidgets::Movies: ui->movieFilesWidget->setFilter(filters, text); break;
    case MainWidgets::TvShows: ui->tvShowFilesWidget->setFilter(filters, text); break;
    case MainWidgets::Concerts: ui->concertFilesWidget->setFilter(filters, text); break;
    case MainWidgets::Music: ui->musicFilesWidget->setFilter(filters, text); break;
    case MainWidgets::MovieSets:         // not supported, yet
    case MainWidgets::Genres:            // not supported, yet
    case MainWidgets::Duplicates:        // not supported, yet
    case MainWidgets::Certifications:    // not supported, yet
    case MainWidgets::Downloads: return; // not supported -> return
    }
}

/**
 * \brief Sets the status of the save and save all action
 * \param enabled Status
 * \param widget Widget to set the status for
 */
void MainWindow::onSetSaveEnabled(bool enabled, MainWidgets widget)
{
    m_actions[widget][MainActions::Save] = enabled;

    if (widget != MainWidgets::MovieSets && widget != MainWidgets::Certifications) {
        m_actions[widget][MainActions::SaveAll] = enabled;
        if (widget != MainWidgets::Music) {
            m_actions[widget][MainActions::Rename] = enabled;
        }
    }

    if (widget != currentTab()) {
        return;
    }

    switch (widget) {
    case MainWidgets::Movies:
    case MainWidgets::TvShows:
    case MainWidgets::Concerts: {
        ui->navbar->setActionSaveEnabled(enabled);
        ui->navbar->setActionSaveAllEnabled(enabled);
        ui->navbar->setActionRenameEnabled(enabled);
        break;
    }
    case MainWidgets::Music: {
        ui->navbar->setActionSaveEnabled(enabled);
        ui->navbar->setActionSaveAllEnabled(enabled);
        break;
    }

    case MainWidgets::MovieSets:
    case MainWidgets::Genres:
    case MainWidgets::Certifications: {
        ui->navbar->setActionSaveEnabled(enabled);
        break;
    }

    case MainWidgets::Downloads: return;
    case MainWidgets::Duplicates: return;
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

    if (widget != currentTab()) {
        return;
    }

    switch (widget) {
    case MainWidgets::Movies:
    case MainWidgets::TvShows:
    case MainWidgets::Concerts:
    case MainWidgets::Music: ui->navbar->setActionSearchEnabled(enabled); break;
    case MainWidgets::MovieSets:
    case MainWidgets::Genres:
    case MainWidgets::Certifications:
    case MainWidgets::Duplicates:
    case MainWidgets::Downloads: break;
    }
}


void MainWindow::moveSplitter(int pos, int index)
{
    Q_UNUSED(index)

    QList<int> sizes;
    const QList<QSplitter*> splitters{ui->movieSplitter,
        ui->tvShowSplitter,
        ui->setsWidget->splitter(),
        ui->genreWidget->splitter(),
        ui->certificationWidget->splitter(),
        ui->concertSplitter,
        ui->musicSplitter};

    for (const QSplitter* splitter : splitters) {
        if (splitter->sizes().at(0) == pos) {
            sizes = splitter->sizes();
            break;
        }
    }

    for (QSplitter* splitter : splitters) {
        splitter->setSizes(sizes);
    }

    // TODO:
    // Why was the model updated here? Do refresh the widgets?
    Manager::instance()->movieModel()->update();
    Manager::instance()->concertModel()->update();
}

void MainWindow::setNewMarks()
{
    auto* manager = Manager::instance();

    auto setMark = [&](QToolButton* btn, int count) {
        if (btn != nullptr) {
            const QColor color = btn->property("isActive").toBool() ? m_buttonActiveColor : m_buttonColor;
            const QString iconStar = (count > 0) ? "star" : "";
            const QString iconName = btn->property("iconName").toString();
            btn->setIcon(manager->iconFont()->icon(iconName, color, iconStar, count));
        }
    };

    setMark(ui->buttonMovies, manager->movieModel()->countNewMovies());
    setMark(ui->buttonTvshows, manager->tvShowModel()->hasNewShowOrEpisode());
    setMark(ui->buttonConcerts, manager->concertModel()->countNewConcerts());
    setMark(ui->buttonMusic, manager->musicModel()->hasNewArtistsOrAlbums());
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
    const QVector<TvShow*> shows = Manager::instance()->tvShowModel()->tvShows();
    for (TvShow* show : shows) {
        if (show->showMissingEpisodes()) {
            TvShowUpdater::instance()->updateShow(show);
        }
    }
}

void MainWindow::onCommandBarOpen()
{
    // TODO: At the moment we only support movies
    if (currentTab() != MainWidgets::Movies) {
        return;
    }

    auto* commandBar = new mediaelch::QuickOpen(this);
    connect(commandBar, &mediaelch::QuickOpen::itemSelected, this, [this](QModelIndex index) {
        ui->movieFilesWidget->selectIndex(index);
    });
    connect(commandBar, &mediaelch::QuickOpen::closed, this, [commandBar]() { commandBar->deleteLater(); });
    commandBar->setModel(Manager::instance()->movieModel());
    centralWidget()->setFocusProxy(commandBar);
}

MainWidgets MainWindow::currentTab() const
{
    auto* currentWidget = ui->stackedWidget->currentWidget();

    if (currentWidget == ui->moviePage) {
        return MainWidgets::Movies;
    }
    if (currentWidget == ui->movieSetsPage) {
        return MainWidgets::MovieSets;
    }
    if (currentWidget == ui->tvShowPage) {
        return MainWidgets::TvShows;
    }
    if (currentWidget == ui->concertsPage) {
        return MainWidgets::Concerts;
    }
    if (currentWidget == ui->genresPage) {
        return MainWidgets::Genres;
    }
    if (currentWidget == ui->certificationsPage) {
        return MainWidgets::Certifications;
    }
    if (currentWidget == ui->downloadsPage) {
        return MainWidgets::Downloads;
    }
    if (currentWidget == ui->musicPage) {
        return MainWidgets::Music;
    }
    if (currentWidget == ui->duplicatesPage) {
        return MainWidgets::Duplicates;
    }
    qCritical() << "[MainWindow] Unknown tab is selected! Index:" << ui->stackedWidget->currentIndex();
    return MainWidgets::Movies;
}

void MainWindow::onMenu(QToolButton* button)
{
    const auto buttons = ui->menuWidget->findChildren<QToolButton*>();
    for (QToolButton* btn : buttons) {
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
    case 8:
        // Duplicates
        widget = MainWidgets::Duplicates;
        break;
    default: qWarning() << "Unhandled page in main window." << page; break;
    }

    ui->navbar->setActionSearchEnabled(m_actions[widget][MainActions::Search]);
    ui->navbar->setActionSaveEnabled(m_actions[widget][MainActions::Save]);
    ui->navbar->setActionSaveAllEnabled(m_actions[widget][MainActions::SaveAll]);
    ui->navbar->setActionRenameEnabled(m_actions[widget][MainActions::Rename]);
    ui->navbar->setFilterWidgetEnabled(m_actions[widget][MainActions::FilterWidget]);
    ui->navbar->setActiveWidget(widget);
}
