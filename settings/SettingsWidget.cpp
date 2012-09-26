#include "SettingsWidget.h"
#include "ui_SettingsWidget.h"

#include <QComboBox>

#include "movies/FilesWidget.h"
#include "main/MainWindow.h"
#include "globals/Manager.h"
#include "main/MessageBox.h"
#include "tvShows/TvShowFilesWidget.h"


/**
 * @brief SettingsWidget::SettingsWidget
 * @param parent
 */
SettingsWidget::SettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);

    m_settings = Settings::instance(this);

    ui->movieDirs->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->movieDirs->horizontalHeaderItem(2)->setToolTip(tr("Movies are in separate folders"));
    ui->movieDirs->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    ui->movieDirs->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
    ui->tvShowDirs->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->tvShowDirs->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->concertDirs->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->concertDirs->horizontalHeaderItem(2)->setToolTip(tr("Concerts are in separate folders"));
    ui->concertDirs->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    ui->concertDirs->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);

    int scraperCounter = 0;
    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->hasSettings()) {
            if (scraperCounter++ > 0) {
                QFrame *line = new QFrame(ui->groupBox_2);
                line->setFrameShape(QFrame::HLine);
                line->setFrameShadow(QFrame::Sunken);
                ui->verticalLayoutScrapers->addWidget(line);
            }
            QWidget *scraperSettings = new QWidget(ui->groupBox_2);
            QComboBox *settingsLanguageCombo = new QComboBox(scraperSettings);
            m_scraperCombos.insert(scraper, settingsLanguageCombo);
            QMapIterator<QString, QString> it(scraper->languages());
            while (it.hasNext()) {
                it.next();
                settingsLanguageCombo->addItem(it.key(), it.value());
            }
            settingsLanguageCombo->setParent(scraperSettings);
            QLabel *label = new QLabel(tr("Language"), scraperSettings);
            QHBoxLayout *hboxLayout = new QHBoxLayout(scraperSettings);
            QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
            hboxLayout->addWidget(label);
            hboxLayout->addWidget(settingsLanguageCombo);
            hboxLayout->addSpacerItem(spacer);
            scraperSettings->setLayout(hboxLayout);

            ui->verticalLayoutScrapers->addWidget(new QLabel(scraper->name(), ui->groupBox_2));
            ui->verticalLayoutScrapers->addWidget(scraperSettings);
        }
    }
    foreach (TvScraperInterface *scraper, Manager::instance()->tvScrapers()) {
        if (scraper->hasSettings()) {
            if (scraperCounter++ > 0) {
                QFrame *line = new QFrame(ui->groupBox_2);
                line->setFrameShape(QFrame::HLine);
                line->setFrameShadow(QFrame::Sunken);
                ui->verticalLayoutScrapers->addWidget(line);
            }
            QWidget *scraperSettings = new QWidget(ui->groupBox_2);
            QComboBox *settingsLanguageCombo = new QComboBox(scraperSettings);
            m_tvScraperCombos.insert(scraper, settingsLanguageCombo);
            QMapIterator<QString, QString> it(scraper->languages());
            while (it.hasNext()) {
                it.next();
                settingsLanguageCombo->addItem(it.key(), it.value());
            }
            settingsLanguageCombo->setParent(scraperSettings);
            QLabel *label = new QLabel(tr("Language"), scraperSettings);
            QHBoxLayout *hboxLayout = new QHBoxLayout(scraperSettings);
            QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
            hboxLayout->addWidget(label);
            hboxLayout->addWidget(settingsLanguageCombo);
            hboxLayout->addSpacerItem(spacer);
            scraperSettings->setLayout(hboxLayout);

            ui->verticalLayoutScrapers->addWidget(new QLabel(scraper->name(), ui->groupBox_2));
            ui->verticalLayoutScrapers->addWidget(scraperSettings);
        }
    }
    foreach (ConcertScraperInterface *scraper, Manager::instance()->concertScrapers()) {
        if (scraper->hasSettings()) {
            if (scraperCounter++ > 0) {
                QFrame *line = new QFrame(ui->groupBox_2);
                line->setFrameShape(QFrame::HLine);
                line->setFrameShadow(QFrame::Sunken);
                ui->verticalLayoutScrapers->addWidget(line);
            }
            QWidget *scraperSettings = new QWidget(ui->groupBox_2);
            QComboBox *settingsLanguageCombo = new QComboBox(scraperSettings);
            m_concertScraperCombos.insert(scraper, settingsLanguageCombo);
            QMapIterator<QString, QString> it(scraper->languages());
            while (it.hasNext()) {
                it.next();
                settingsLanguageCombo->addItem(it.key(), it.value());
            }
            settingsLanguageCombo->setParent(scraperSettings);
            QLabel *label = new QLabel(tr("Language"), scraperSettings);
            QHBoxLayout *hboxLayout = new QHBoxLayout(scraperSettings);
            QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
            hboxLayout->addWidget(label);
            hboxLayout->addWidget(settingsLanguageCombo);
            hboxLayout->addSpacerItem(spacer);
            scraperSettings->setLayout(hboxLayout);

            ui->verticalLayoutScrapers->addWidget(new QLabel(scraper->name(), ui->groupBox_2));
            ui->verticalLayoutScrapers->addWidget(scraperSettings);
        }
    }

    // Setup file dialogs
    m_logFileDialog = new QFileDialog(this, tr("Logfile"), QDir::homePath(), tr("Logfiles (*.log *.txt)"));
    m_logFileDialog->setFileMode(QFileDialog::AnyFile);
    m_movieDirDialog = new QFileDialog(this, tr("Choose a directory containing your movies"), QDir::homePath());
    m_movieDirDialog->setFileMode(QFileDialog::Directory);
    m_movieDirDialog->setOption(QFileDialog::ShowDirsOnly, true);
    m_tvShowDirDialog = new QFileDialog(this, tr("Choose a directory containing your TV shows"), QDir::homePath());
    m_tvShowDirDialog->setFileMode(QFileDialog::Directory);
    m_tvShowDirDialog->setOption(QFileDialog::ShowDirsOnly, true);
    m_concertDirDialog = new QFileDialog(this, tr("Choose a directory containing your concerts"), QDir::homePath());
    m_concertDirDialog->setFileMode(QFileDialog::Directory);
    m_concertDirDialog->setOption(QFileDialog::ShowDirsOnly, true);
    m_xbmcThumbnailDirDialog = new QFileDialog(this, tr("Choose a directory containing your Thumbnails"), QDir::homePath());;
    m_xbmcThumbnailDirDialog->setOption(QFileDialog::ShowDirsOnly, true);
    m_xbmcSqliteDatabaseDialog = new QFileDialog(this, tr("SQLite Database *.db"), QDir::homePath());
    m_xbmcSqliteDatabaseDialog->setFileMode(QFileDialog::ExistingFile);

    connect(m_logFileDialog, SIGNAL(fileSelected(QString)), this, SLOT(onDebugLogPathChosen(QString)));
    connect(m_movieDirDialog, SIGNAL(fileSelected(QString)), this, SLOT(addMovieDir(QString)));
    connect(m_tvShowDirDialog, SIGNAL(fileSelected(QString)), this, SLOT(addTvShowDir(QString)));
    connect(m_concertDirDialog, SIGNAL(fileSelected(QString)), this, SLOT(addConcertDir(QString)));
    connect(m_xbmcThumbnailDirDialog, SIGNAL(fileSelected(QString)), this, SLOT(onChooseXbmcThumbnailPath(QString)));
    connect(m_xbmcSqliteDatabaseDialog, SIGNAL(fileSelected(QString)), this, SLOT(onChooseMediaCenterXbmcSqliteDatabase(QString)));

    connect(ui->buttonAddMovieDir, SIGNAL(clicked()), m_movieDirDialog, SLOT(open()));
    connect(ui->buttonRemoveMovieDir, SIGNAL(clicked()), this, SLOT(removeMovieDir()));
    connect(ui->movieDirs, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(movieListRowChanged(int)));
    connect(ui->buttonAddTvShowDir, SIGNAL(clicked()), m_tvShowDirDialog, SLOT(open()));
    connect(ui->buttonRemoveTvShowDir, SIGNAL(clicked()), this, SLOT(removeTvShowDir()));
    connect(ui->tvShowDirs, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(tvShowListRowChanged(int)));
    connect(ui->buttonAddConcertDir, SIGNAL(clicked()), m_concertDirDialog, SLOT(open()));
    connect(ui->buttonRemoveConcertDir, SIGNAL(clicked()), this, SLOT(removeConcertDir()));
    connect(ui->concertDirs, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(concertListRowChanged(int)));
    connect(ui->movieDirs, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(movieMediaCenterPathChanged(QTableWidgetItem*)));
    connect(ui->tvShowDirs, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(tvShowMediaCenterPathChanged(QTableWidgetItem*)));
    connect(ui->concertDirs, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(concertMediaCenterPathChanged(QTableWidgetItem*)));
    connect(ui->chkActivateDebug, SIGNAL(clicked()), this, SLOT(onActivateDebugMode()));
    connect(ui->buttonChooseLogfile, SIGNAL(clicked()), m_logFileDialog, SLOT(open()));
    connect(ui->logfilePath, SIGNAL(textChanged(QString)), this, SLOT(onSetDebugLogPath(QString)));

    connect(ui->radioXbmcXml, SIGNAL(clicked()), this, SLOT(onMediaCenterXbmcXmlSelected()));
    connect(ui->radioXbmcMysql, SIGNAL(clicked()), this, SLOT(onMediaCenterXbmcMysqlSelected()));
    connect(ui->radioXbmcSqlite, SIGNAL(clicked()), this, SLOT(onMediaCenterXbmcSqliteSelected()));
    connect(ui->buttonSelectSqliteDatabase, SIGNAL(clicked()), m_xbmcSqliteDatabaseDialog, SLOT(open()));
    connect(ui->buttonSelectThumbnailPath, SIGNAL(clicked()), m_xbmcThumbnailDirDialog, SLOT(open()));

    loadSettings();
}

/**
 * @brief SettingsWidget::~SettingsWidget
 */
SettingsWidget::~SettingsWidget()
{
    delete ui;
}

/**
 * @brief Loads all settings
 */
void SettingsWidget::loadSettings()
{
    m_settings->loadSettings();

    // Debug
    ui->chkActivateDebug->setChecked(m_settings->debugModeActivated());
    ui->logfilePath->setText(m_settings->debugLogPath());
    onActivateDebugMode();

    // Movie Directories
    QList<SettingsDir> movieDirectories = m_settings->movieDirectories();
    ui->movieDirs->setRowCount(0);
    ui->movieDirs->clearContents();
    for (int i=0, n=movieDirectories.count() ; i<n ; ++i) {
        ui->movieDirs->insertRow(i);
        QTableWidgetItem *item = new QTableWidgetItem(movieDirectories.at(i).path);
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        item->setToolTip(QDir::toNativeSeparators(movieDirectories.at(i).path));
        QTableWidgetItem *item2 = new QTableWidgetItem;
        item2->setToolTip(tr("Movies are in separate folders"));
        if (movieDirectories.at(i).separateFolders)
            item2->setCheckState(Qt::Checked);
        else
            item2->setCheckState(Qt::Unchecked);
        ui->movieDirs->setItem(i, 0, item);
        ui->movieDirs->setItem(i, 1, new QTableWidgetItem(movieDirectories.at(i).mediaCenterPath));
        ui->movieDirs->setItem(i, 2, item2);
    }
    ui->buttonRemoveMovieDir->setEnabled(!movieDirectories.isEmpty());

    // TV Show Directories
    QList<SettingsDir> tvShowDirectories = m_settings->tvShowDirectories();
    ui->tvShowDirs->setRowCount(0);
    ui->tvShowDirs->clearContents();
    for (int i=0, n=tvShowDirectories.size() ; i<n ; ++i) {
        ui->tvShowDirs->insertRow(i);
        QTableWidgetItem *item = new QTableWidgetItem(tvShowDirectories.at(i).path);
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        item->setToolTip(QDir::toNativeSeparators(tvShowDirectories.at(i).path));
        ui->tvShowDirs->setItem(i, 0, item);
        ui->tvShowDirs->setItem(i, 1, new QTableWidgetItem(tvShowDirectories.at(i).mediaCenterPath));
    }
    ui->buttonRemoveTvShowDir->setEnabled(!tvShowDirectories.isEmpty());

    // Concert Directories
    QList<SettingsDir> concertDirectories = m_settings->concertDirectories();
    ui->concertDirs->setRowCount(0);
    ui->concertDirs->clearContents();
    for (int i=0, n=concertDirectories.count() ; i<n ; ++i) {
        ui->concertDirs->insertRow(i);
        QTableWidgetItem *item = new QTableWidgetItem(concertDirectories.at(i).path);
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        item->setToolTip(QDir::toNativeSeparators(concertDirectories.at(i).path));
        QTableWidgetItem *item2 = new QTableWidgetItem;
        item2->setToolTip(tr("Concerts are in separate folders"));
        if (concertDirectories.at(i).separateFolders)
            item2->setCheckState(Qt::Checked);
        else
            item2->setCheckState(Qt::Unchecked);
        ui->concertDirs->setItem(i, 0, item);
        ui->concertDirs->setItem(i, 1, new QTableWidgetItem(concertDirectories.at(i).mediaCenterPath));
        ui->concertDirs->setItem(i, 2, item2);
    }
    ui->buttonRemoveConcertDir->setEnabled(!concertDirectories.isEmpty());

    // MediaCenterInterface
    int mediaCenterInterface = m_settings->mediaCenterInterface();
    if (mediaCenterInterface == MediaCenterInterfaces::XbmcXml)
        onMediaCenterXbmcXmlSelected();
    else if (mediaCenterInterface == MediaCenterInterfaces::XbmcMysql)
        onMediaCenterXbmcMysqlSelected();
    else if (mediaCenterInterface == MediaCenterInterfaces::XbmcSqlite)
        onMediaCenterXbmcSqliteSelected();


    ui->inputDatabase->setText(m_settings->xbmcMysqlDatabase());
    ui->inputHost->setText(m_settings->xbmcMysqlHost());
    ui->inputUsername->setText(m_settings->xbmcMysqlUser());
    ui->inputPassword->setText(m_settings->xbmcMysqlPassword());
    ui->inputSqliteDatabase->setText(m_settings->xbmcSqliteDatabase());
    m_xbmcSqliteDatabaseDialog->selectFile(m_settings->xbmcSqliteDatabase());
    ui->useYoutubePluginUrls->setChecked(m_settings->useYoutubePluginUrls());

    ui->inputThumbnailPath->setText(m_settings->xbmcThumbnailPath());
    ui->inputThumbnailPath->setToolTip(m_settings->xbmcThumbnailPath());
    m_xbmcThumbnailDirDialog->selectFile(m_settings->xbmcThumbnailPath());

    ui->xmlNfos->clear();
    ui->xmlPosters->clear();
    ui->xmlFanarts->clear();
    ui->xmlTvShowPosters->clear();
    ui->xmlTvShowBanners->clear();
    foreach (DataFile *file, m_settings->movieNfoFiles()) {
        QListWidgetItem *item = new QListWidgetItem(file->name());
        item->setData(Qt::UserRole, file->id());
        item->setCheckState((file->enabled()) ? Qt::Checked : Qt::Unchecked);
        ui->xmlNfos->addItem(item);
    }

    foreach (DataFile *file, m_settings->moviePosterFiles()) {
        QListWidgetItem *item = new QListWidgetItem(file->name());
        item->setData(Qt::UserRole, file->id());
        item->setCheckState((file->enabled()) ? Qt::Checked : Qt::Unchecked);
        ui->xmlPosters->addItem(item);
    }

    foreach (DataFile *file, m_settings->movieFanartFiles()) {
        QListWidgetItem *item = new QListWidgetItem(file->name());
        item->setData(Qt::UserRole, file->id());
        item->setCheckState((file->enabled()) ? Qt::Checked : Qt::Unchecked);
        ui->xmlFanarts->addItem(item);
    }

    foreach (DataFile *file, m_settings->tvShowPosterFiles()) {
        QListWidgetItem *item = new QListWidgetItem(file->name());
        item->setData(Qt::UserRole, file->id());
        item->setCheckState((file->enabled()) ? Qt::Checked : Qt::Unchecked);
        ui->xmlTvShowPosters->addItem(item);
    }

    foreach (DataFile *file, m_settings->tvShowBannerFiles()) {
        QListWidgetItem *item = new QListWidgetItem(file->name());
        item->setData(Qt::UserRole, file->id());
        item->setCheckState((file->enabled()) ? Qt::Checked : Qt::Unchecked);
        ui->xmlTvShowBanners->addItem(item);
    }

    // Scrapers
    QMapIterator<ScraperInterface*, QComboBox*> it(m_scraperCombos);
    while (it.hasNext()) {
        it.next();
        for (int i=0, n=it.value()->count() ; i<n ; ++i) {
            if (it.value()->itemData(i).toString() == it.key()->language())
                it.value()->setCurrentIndex(i);
        }
    }
    QMapIterator<TvScraperInterface*, QComboBox*> itTv(m_tvScraperCombos);
    while (itTv.hasNext()) {
        itTv.next();
        for (int i=0, n=itTv.value()->count() ; i<n ; ++i) {
            if (itTv.value()->itemData(i).toString() == itTv.key()->language())
                itTv.value()->setCurrentIndex(i);
        }
    }
    QMapIterator<ConcertScraperInterface*, QComboBox*> itC(m_concertScraperCombos);
    while (itC.hasNext()) {
        itC.next();
        for (int i=0, n=itC.value()->count() ; i<n ; ++i) {
            if (itC.value()->itemData(i).toString() == itC.key()->language())
                itC.value()->setCurrentIndex(i);
        }
    }
}

/**
 * @brief Saves all settings
 */
void SettingsWidget::saveSettings()
{
    bool mediaInterfaceChanged = false;

    int mediaCenterInterface = -1;
    if (ui->radioXbmcXml->isChecked()) {
        if (m_settings->mediaCenterInterface() != MediaCenterInterfaces::XbmcXml)
            mediaInterfaceChanged = true;
        mediaCenterInterface = MediaCenterInterfaces::XbmcXml;
    } else if (ui->radioXbmcMysql->isChecked()) {
        if (m_settings->mediaCenterInterface() != MediaCenterInterfaces::XbmcMysql)
            mediaInterfaceChanged = true;
        mediaCenterInterface = MediaCenterInterfaces::XbmcMysql;
    } else if (ui->radioXbmcSqlite->isChecked()) {
        if (m_settings->mediaCenterInterface() != MediaCenterInterfaces::XbmcSqlite)
            mediaInterfaceChanged = true;
        mediaCenterInterface = MediaCenterInterfaces::XbmcSqlite;
    }
    m_settings->setMediaCenterInterface(mediaCenterInterface);

    if (ui->radioXbmcXml->isChecked()) {
        QList<DataFile*> movieNfoFiles = m_settings->movieNfoFiles();
        for (int i=0, n=ui->xmlNfos->count() ; i<n ; ++i) {
            foreach (DataFile *file, movieNfoFiles) {
                if (file->id() != ui->xmlNfos->item(i)->data(Qt::UserRole).toInt())
                    continue;
                file->setEnabled(ui->xmlNfos->item(i)->checkState() == Qt::Checked);
                file->setPos(i);
            }
        }
        m_settings->setMovieNfoFiles(movieNfoFiles);

        QList<DataFile*> moviePosterFiles = m_settings->moviePosterFiles();
        for (int i=0, n=ui->xmlPosters->count() ; i<n ; ++i) {
            foreach (DataFile *file, moviePosterFiles) {
                if (file->id() != ui->xmlPosters->item(i)->data(Qt::UserRole).toInt())
                    continue;
                file->setEnabled(ui->xmlPosters->item(i)->checkState() == Qt::Checked);
                file->setPos(i);
            }
        }
        m_settings->setMoviePosterFiles(moviePosterFiles);

        QList<DataFile*> movieFanartFiles = m_settings->movieFanartFiles();
        for (int i=0, n=ui->xmlFanarts->count() ; i<n ; ++i) {
            foreach (DataFile *file, movieFanartFiles) {
                if (file->id() != ui->xmlFanarts->item(i)->data(Qt::UserRole).toInt())
                    continue;
                file->setEnabled(ui->xmlFanarts->item(i)->checkState() == Qt::Checked);
                file->setPos(i);
            }
        }
        m_settings->setMovieFanartFiles(movieFanartFiles);

        QList<DataFile*> tvShowPosterFiles = m_settings->tvShowPosterFiles();
        for (int i=0, n=ui->xmlTvShowPosters->count() ; i<n ; ++i) {
            foreach (DataFile *file, tvShowPosterFiles) {
                if (file->id() != ui->xmlTvShowPosters->item(i)->data(Qt::UserRole).toInt())
                    continue;
                file->setEnabled(ui->xmlTvShowPosters->item(i)->checkState() == Qt::Checked);
                file->setPos(i);
            }
        }
        m_settings->setTvShowPosterFiles(tvShowPosterFiles);

        QList<DataFile*> tvShowBannerFiles = m_settings->tvShowBannerFiles();
        for (int i=0, n=ui->xmlTvShowBanners->count() ; i<n ; ++i) {
            foreach (DataFile *file, tvShowBannerFiles) {
                if (file->id() != ui->xmlTvShowBanners->item(i)->data(Qt::UserRole).toInt())
                    continue;
                file->setEnabled(ui->xmlTvShowBanners->item(i)->checkState() == Qt::Checked);
                file->setPos(i);
            }
        }
        m_settings->setTvShowBannerFiles(tvShowBannerFiles);
    }

    m_settings->setXbmcMysqlHost(ui->inputHost->text());
    m_settings->setXbmcMysqlDatabase(ui->inputDatabase->text());
    m_settings->setXbmcMysqlUser(ui->inputUsername->text());
    m_settings->setXbmcMysqlPassword(ui->inputPassword->text());
    m_settings->setXbmcSqliteDatabase(ui->inputSqliteDatabase->text());
    m_settings->setUseYoutubePluginUrls(ui->useYoutubePluginUrls->isChecked());

    // Scrapers
    QMapIterator<ScraperInterface*, QComboBox*> it(m_scraperCombos);
    while (it.hasNext()) {
        it.next();
        it.key()->setLanguage(it.value()->itemData(it.value()->currentIndex()).toString());
    }
    QMapIterator<TvScraperInterface*, QComboBox*> itTv(m_tvScraperCombos);
    while (itTv.hasNext()) {
        itTv.next();
        itTv.key()->setLanguage(itTv.value()->itemData(itTv.value()->currentIndex()).toString());
    }
    QMapIterator<ConcertScraperInterface*, QComboBox*> itC(m_concertScraperCombos);
    while (itC.hasNext()) {
        itC.next();
        itC.key()->setLanguage(itC.value()->itemData(itC.value()->currentIndex()).toString());
    }


    m_settings->saveSettings();

    Manager::instance()->movieFileSearcher()->setMovieDirectories(m_settings->movieDirectories());
    Manager::instance()->tvShowFileSearcher()->setMovieDirectories(m_settings->tvShowDirectories());
    Manager::instance()->concertFileSearcher()->setConcertDirectories(m_settings->concertDirectories());
    MessageBox::instance()->showMessage(tr("Settings saved"));

    Manager::instance()->setupMediaCenterInterface();
    if (mediaInterfaceChanged) {
        // TvShow File Searcher and concert file searcher are started when Movie File Searcher has finished @see MainWindow.cpp
        Manager::instance()->movieFileSearcher()->start();
    }
}

/**
 * @brief Adds a movie directory
 * @param dir Directory to add
 */
void SettingsWidget::addMovieDir(QString dir)
{
    dir = QDir::toNativeSeparators(dir);
    if (!dir.isEmpty()) {
        bool exists = false;
        QList<SettingsDir> movieDirectories = m_settings->movieDirectories();
        for (int i=0, n=movieDirectories.count() ; i<n ; ++i) {
            if (movieDirectories.at(i).path == dir)
                exists = true;
        }

        if (!exists) {
            SettingsDir sDir;
            sDir.path = dir;
            movieDirectories.append(sDir);
            int row = ui->movieDirs->rowCount();
            ui->movieDirs->insertRow(row);
            QTableWidgetItem *item = new QTableWidgetItem(dir);
            item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            item->setToolTip(dir);
            QTableWidgetItem *itemCheck = new QTableWidgetItem();
            itemCheck->setCheckState(Qt::Unchecked);
            ui->movieDirs->setItem(row, 0, item);
            ui->movieDirs->setItem(row, 1, new QTableWidgetItem(""));
            ui->movieDirs->setItem(row, 2, itemCheck);
            m_settings->setMovieDirectories(movieDirectories);
        }
    }
}

/**
 * @brief Removes a movie directory
 */
void SettingsWidget::removeMovieDir()
{
    int row = ui->movieDirs->currentRow();
    if (row < 0)
        return;

    QList<SettingsDir> movieDirectories = m_settings->movieDirectories();
    movieDirectories.removeAt(row);
    m_settings->setMovieDirectories(movieDirectories);
    ui->movieDirs->removeRow(row);
}

/**
 * @brief Enables/disables the button to remove a movie dir
 * @param currentRow Current row in the movie list
 */
void SettingsWidget::movieListRowChanged(int currentRow)
{
    ui->buttonRemoveMovieDir->setDisabled(currentRow < 0);
}

/**
 * @brief Adds a tv show dir
 * @param dir Directory to add
 */
void SettingsWidget::addTvShowDir(QString dir)
{
    dir = QDir::toNativeSeparators(dir);
    if (!dir.isEmpty()) {
        bool exists = false;
        QList<SettingsDir> tvShowDirectories = m_settings->tvShowDirectories();
        for (int i=0, n=tvShowDirectories.count() ; i<n ; ++i) {
            if (tvShowDirectories.at(i).path == dir)
                exists = true;
        }

        if (!exists) {
            SettingsDir sDir;
            sDir.path = dir;
            tvShowDirectories.append(sDir);
            int row = ui->tvShowDirs->rowCount();
            ui->tvShowDirs->insertRow(row);
            QTableWidgetItem *item = new QTableWidgetItem(dir);
            item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            item->setToolTip(dir);
            ui->tvShowDirs->setItem(row, 0, item);
            ui->tvShowDirs->setItem(row, 1, new QTableWidgetItem(""));
            m_settings->setTvShowDirectories(tvShowDirectories);
        }
    }
}

/**
 * @brief Removes a tv show dir
 */
void SettingsWidget::removeTvShowDir()
{
    int row = ui->tvShowDirs->currentRow();
    if (row<0)
        return;

    QList<SettingsDir> tvShowDirectories = m_settings->tvShowDirectories();
    tvShowDirectories.removeAt(row);
    m_settings->setTvShowDirectories(tvShowDirectories);
    ui->tvShowDirs->removeRow(row);
}

/**
 * @brief Enables/Disables the button to remove a tv show dir
 * @param currentRow Current selected row in the list of tv show dirs
 */
void SettingsWidget::tvShowListRowChanged(int currentRow)
{
    ui->buttonRemoveTvShowDir->setDisabled(currentRow < 0);
}

/**
 * @brief Adds a concert directory
 * @param dir Directory to add
 */
void SettingsWidget::addConcertDir(QString dir)
{
    dir = QDir::toNativeSeparators(dir);
    if (!dir.isEmpty()) {
        bool exists = false;
        QList<SettingsDir> concertDirectories = m_settings->concertDirectories();
        for (int i=0, n=concertDirectories.count() ; i<n ; ++i) {
            if (concertDirectories.at(i).path == dir)
                exists = true;
        }

        if (!exists) {
            SettingsDir sDir;
            sDir.path = dir;
            concertDirectories.append(sDir);
            int row = ui->concertDirs->rowCount();
            ui->concertDirs->insertRow(row);
            QTableWidgetItem *item = new QTableWidgetItem(dir);
            item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            item->setToolTip(dir);
            QTableWidgetItem *itemCheck = new QTableWidgetItem();
            itemCheck->setCheckState(Qt::Unchecked);
            ui->concertDirs->setItem(row, 0, item);
            ui->concertDirs->setItem(row, 1, new QTableWidgetItem(""));
            ui->concertDirs->setItem(row, 2, itemCheck);
            m_settings->setConcertDirectories(concertDirectories);
        }
    }
}

/**
 * @brief Removes a concert directory
 */
void SettingsWidget::removeConcertDir()
{
    int row = ui->concertDirs->currentRow();
    if (row < 0)
        return;

    QList<SettingsDir> concertDirectories = m_settings->concertDirectories();
    concertDirectories.removeAt(row);
    m_settings->setTvShowDirectories(concertDirectories);
    ui->concertDirs->removeRow(row);
}

/**
 * @brief Enables/disables the button to remove a concert dir
 * @param currentRow Current row in the concert list
 */
void SettingsWidget::concertListRowChanged(int currentRow)
{
    ui->buttonRemoveConcertDir->setDisabled(currentRow < 0);
}


/**
 * @brief Stores the values from the list for movie directories
 * @param item Current item
 */
void SettingsWidget::movieMediaCenterPathChanged(QTableWidgetItem *item)
{
    QList<SettingsDir> movieDirectories = m_settings->movieDirectories();
    if (item->row() < 0 || item->row() >= movieDirectories.count())
        return;

    if (item->column() == 1)
        movieDirectories[item->row()].mediaCenterPath = item->text();
    else if (item->column() == 2)
        movieDirectories[item->row()].separateFolders = item->checkState() == Qt::Checked;
    m_settings->setMovieDirectories(movieDirectories);
}

/**
 * @brief Stores the values from the list for tv show directories
 * @param item Current item
 */
void SettingsWidget::tvShowMediaCenterPathChanged(QTableWidgetItem *item)
{
    QList<SettingsDir> tvShowDirectories = m_settings->tvShowDirectories();
    if (item->row() < 0 || item->row() >= tvShowDirectories.count() || item->column() != 1)
        return;
    tvShowDirectories[item->row()].mediaCenterPath = item->text();
    m_settings->setTvShowDirectories(tvShowDirectories);
}

/**
 * @brief Stores the values from the list for concert directories
 * @param item Current item
 */
void SettingsWidget::concertMediaCenterPathChanged(QTableWidgetItem *item)
{
    QList<SettingsDir> concertDirectories = m_settings->concertDirectories();
    if (item->row() < 0 || item->row() >= concertDirectories.count())
        return;

    if (item->column() == 1)
        concertDirectories[item->row()].mediaCenterPath = item->text();
    else if (item->column() == 2)
        concertDirectories[item->row()].separateFolders = item->checkState() == Qt::Checked;
    m_settings->setConcertDirectories(concertDirectories);
}

/**
 * @brief Handles status of MediaCenter checkboxes and inputs
 */
void SettingsWidget::onMediaCenterXbmcXmlSelected()
{
    ui->radioXbmcXml->setChecked(true);
    ui->widgetXbmcXmlFiles->setVisible(true);
    ui->widgetXbmcMysql->setVisible(false);
    ui->widgetXbmcSqlite->setVisible(false);
    setXbmcThumbnailPathEnabled(false);
}

/**
 * @brief Handles status of MediaCenter checkboxes and inputs
 */
void SettingsWidget::onMediaCenterXbmcMysqlSelected()
{
    ui->radioXbmcMysql->setChecked(true);
    ui->widgetXbmcXmlFiles->setVisible(false);
    ui->widgetXbmcMysql->setVisible(true);
    ui->widgetXbmcSqlite->setVisible(false);
    setXbmcThumbnailPathEnabled(true);
}

/**
 * @brief Handles status of MediaCenter checkboxes and inputs
 */
void SettingsWidget::onMediaCenterXbmcSqliteSelected()
{
    ui->radioXbmcSqlite->setChecked(true);
    ui->widgetXbmcXmlFiles->setVisible(false);
    ui->widgetXbmcMysql->setVisible(false);
    ui->widgetXbmcSqlite->setVisible(true);
    setXbmcThumbnailPathEnabled(true);
}

/**
 * @brief Sets the SQLite database
 * @param file Database file
 */
void SettingsWidget::onChooseMediaCenterXbmcSqliteDatabase(QString file)
{
    if (!file.isEmpty()) {
        ui->inputSqliteDatabase->setText(file);
        m_settings->setXbmcSqliteDatabase(file);
    }
}

/**
 * @brief Shows a dialog to choose the thumbnail directory
 * @param dir Thumbnail directory to set
 */
void SettingsWidget::onChooseXbmcThumbnailPath(QString dir)
{
    if (!dir.isEmpty()) {
        m_settings->setXbmcThumbnailPath(dir);
        ui->inputThumbnailPath->setText(dir);
        ui->inputThumbnailPath->setToolTip(dir);
    }
}

/**
 * @brief Enables or disables the thumbnail path
 * @param enabled Status
 */
void SettingsWidget::setXbmcThumbnailPathEnabled(bool enabled)
{
    ui->inputThumbnailPath->setVisible(enabled);
    ui->buttonSelectThumbnailPath->setVisible(enabled);
    ui->labelXbmcThumbnailPath->setVisible(enabled);
    ui->labelXbmcThumbnailPathDesc->setVisible(enabled);
}

/**
 * @brief Toggles the status of logfile input and logfile select button based on the state of the checkbox
 */
void SettingsWidget::onActivateDebugMode()
{
    ui->logfilePath->setEnabled(ui->chkActivateDebug->isChecked());
    ui->buttonChooseLogfile->setEnabled(ui->chkActivateDebug->isChecked());
    m_settings->setDebugModeActivated(ui->chkActivateDebug->isChecked());
}

/**
 * @brief Sets the path to the logfile
 */
void SettingsWidget::onDebugLogPathChosen(QString file)
{
    file = QDir::toNativeSeparators(file);
    ui->logfilePath->setText(file);
}


/**
 * @brief Sets the path to the logfile
 * @param path Path to logfile
 */
void SettingsWidget::onSetDebugLogPath(QString path)
{
    m_settings->setDebugLogPath(path);
    m_logFileDialog->selectFile(path);
}
