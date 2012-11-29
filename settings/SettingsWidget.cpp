#include "SettingsWidget.h"
#include "ui_SettingsWidget.h"

#include <QComboBox>
#include <QMessageBox>

#include "movies/FilesWidget.h"
#include "main/MainWindow.h"
#include "globals/Manager.h"
#include "main/MessageBox.h"
#include "tvShows/TvShowFilesWidget.h"
#include "data/MovieFilesOrganizer.h"


/**
 * @brief SettingsWidget::SettingsWidget
 * @param parent
 */
SettingsWidget::SettingsWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);

#ifdef Q_WS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    m_settings = Settings::instance(this);

    ui->dirs->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->dirs->horizontalHeaderItem(3)->setToolTip(tr("Items are in separate folders"));
    ui->dirs->horizontalHeaderItem(4)->setToolTip(tr("Automatically reload contents on start"));
    ui->dirs->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
    ui->dirs->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);

    int scraperCounter = 0;
    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->hasSettings()) {
            if (scraperCounter++ > 0) {
                QFrame *line = new QFrame();
                line->setFrameShape(QFrame::HLine);
                line->setFrameShadow(QFrame::Sunken);
                ui->verticalLayoutScrapers->addWidget(line);
            }
            QWidget *scraperSettings = new QWidget();
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

            ui->verticalLayoutScrapers->addWidget(new QLabel(scraper->name()));
            ui->verticalLayoutScrapers->addWidget(scraperSettings);
        }
    }
    foreach (TvScraperInterface *scraper, Manager::instance()->tvScrapers()) {
        if (scraper->hasSettings()) {
            if (scraperCounter++ > 0) {
                QFrame *line = new QFrame();
                line->setFrameShape(QFrame::HLine);
                line->setFrameShadow(QFrame::Sunken);
                ui->verticalLayoutScrapers->addWidget(line);
            }
            QWidget *scraperSettings = new QWidget();
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

            ui->verticalLayoutScrapers->addWidget(new QLabel(scraper->name()));
            ui->verticalLayoutScrapers->addWidget(scraperSettings);
        }
    }
    foreach (ConcertScraperInterface *scraper, Manager::instance()->concertScrapers()) {
        if (scraper->hasSettings()) {
            if (scraperCounter++ > 0) {
                QFrame *line = new QFrame();
                line->setFrameShape(QFrame::HLine);
                line->setFrameShadow(QFrame::Sunken);
                ui->verticalLayoutScrapers->addWidget(line);
            }
            QWidget *scraperSettings = new QWidget();
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

            ui->verticalLayoutScrapers->addWidget(new QLabel(scraper->name()));
            ui->verticalLayoutScrapers->addWidget(scraperSettings);
        }
    }

    // Setup file dialogs
    m_logFileDialog = new QFileDialog(this, tr("Logfile"), QDir::homePath(), tr("Logfiles (*.log *.txt)"));
    m_logFileDialog->setFileMode(QFileDialog::AnyFile);
    m_logFileDialog->selectFile("MediaElch.log");
    m_xbmcThumbnailDirDialog = new QFileDialog(this, tr("Choose a directory containing your Thumbnails"), QDir::homePath());;
    m_xbmcThumbnailDirDialog->setFileMode(QFileDialog::Directory);
    m_xbmcThumbnailDirDialog->setOption(QFileDialog::ShowDirsOnly, true);
    m_xbmcSqliteDatabaseDialog = new QFileDialog(this, tr("SQLite Database *.db"), QDir::homePath());
    m_xbmcSqliteDatabaseDialog->setFileMode(QFileDialog::ExistingFile);

    connect(m_logFileDialog, SIGNAL(fileSelected(QString)), this, SLOT(onDebugLogPathChosen(QString)));
    connect(m_xbmcThumbnailDirDialog, SIGNAL(fileSelected(QString)), this, SLOT(onChooseXbmcThumbnailPath(QString)));
    connect(m_xbmcSqliteDatabaseDialog, SIGNAL(fileSelected(QString)), this, SLOT(onChooseMediaCenterXbmcSqliteDatabase(QString)));

    connect(ui->buttonAddDir, SIGNAL(clicked()), this, SLOT(chooseDirToAdd()));
    connect(ui->buttonRemoveDir, SIGNAL(clicked()), this, SLOT(removeDir()));
    connect(ui->buttonMovieFilesToDirs, SIGNAL(clicked()), this, SLOT(organize()));
    connect(ui->dirs, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(dirListRowChanged(int)));

    connect(ui->chkActivateDebug, SIGNAL(clicked()), this, SLOT(onActivateDebugMode()));
    connect(ui->buttonChooseLogfile, SIGNAL(clicked()), m_logFileDialog, SLOT(open()));
    connect(ui->logfilePath, SIGNAL(textChanged(QString)), this, SLOT(onSetDebugLogPath(QString)));
    connect(ui->chkUseProxy, SIGNAL(clicked()), this, SLOT(onUseProxy()));
    connect(ui->chkAutoLoadStreamDetails, SIGNAL(clicked()), this, SLOT(onAutoLoadStreamDetails()));

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
 * @brief Executes the settings dialog
 * @return Result of QDialog::exec
 */
int SettingsWidget::exec()
{
    qDebug() << "Entered";
    loadSettings();
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height()-100);
    newSize.setWidth(parentWidget()->size().width()-200);
    resize(newSize);
    return QDialog::exec();
}

/**
 * @brief Reloads stored settings and rejects the dialog
 */
void SettingsWidget::reject()
{
    m_settings->loadSettings();
    QDialog::reject();
}

/**
 * @brief Saves the settings and accepts the dialog
 */
void SettingsWidget::accept()
{
    saveSettings();
    QDialog::accept();
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

    // Directories
    ui->dirs->setRowCount(0);
    ui->dirs->clearContents();
    QList<SettingsDir> movieDirectories = m_settings->movieDirectories();
    for (int i=0, n=movieDirectories.count() ; i<n ; ++i)
        addDir(movieDirectories.at(i).path, movieDirectories.at(i).mediaCenterPath, movieDirectories.at(i).separateFolders, movieDirectories.at(i).autoReload, DirTypeMovies);
    QList<SettingsDir> tvShowDirectories = m_settings->tvShowDirectories();
    for (int i=0, n=tvShowDirectories.count() ; i<n ; ++i)
        addDir(tvShowDirectories.at(i).path, tvShowDirectories.at(i).mediaCenterPath, tvShowDirectories.at(i).separateFolders, tvShowDirectories.at(i).autoReload, DirTypeTvShows);
    QList<SettingsDir> concertDirectories = m_settings->concertDirectories();
    for (int i=0, n=concertDirectories.count() ; i<n ; ++i)
        addDir(concertDirectories.at(i).path, concertDirectories.at(i).mediaCenterPath, concertDirectories.at(i).separateFolders, concertDirectories.at(i).autoReload, DirTypeConcerts);

    dirListRowChanged(ui->dirs->currentRow());

    // Exclude words
    ui->excludeWordsText->setPlainText(m_settings->excludeWords());

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

    // Data Files
    ui->movieNfoList->setDataFiles(m_settings->dataFiles(DataFileType::MovieNfo), DataFileType::MovieNfo);
    ui->movieBackdropList->setDataFiles(m_settings->dataFiles(DataFileType::MovieBackdrop), DataFileType::MovieBackdrop);
    ui->movieCdArtList->setDataFiles(m_settings->dataFiles(DataFileType::MovieCdArt), DataFileType::MovieCdArt);
    ui->movieClearArtList->setDataFiles(m_settings->dataFiles(DataFileType::MovieClearArt), DataFileType::MovieClearArt);
    ui->movieLogoList->setDataFiles(m_settings->dataFiles(DataFileType::MovieLogo), DataFileType::MovieLogo);
    ui->moviePosterList->setDataFiles(m_settings->dataFiles(DataFileType::MoviePoster), DataFileType::MoviePoster);
    ui->concertNfoList->setDataFiles(m_settings->dataFiles(DataFileType::ConcertNfo), DataFileType::ConcertNfo);
    ui->concertBackdropList->setDataFiles(m_settings->dataFiles(DataFileType::ConcertBackdrop), DataFileType::ConcertBackdrop);
    ui->concertCdArtList->setDataFiles(m_settings->dataFiles(DataFileType::ConcertCdArt), DataFileType::ConcertCdArt);
    ui->concertClearArtList->setDataFiles(m_settings->dataFiles(DataFileType::ConcertClearArt), DataFileType::ConcertClearArt);
    ui->concertLogoList->setDataFiles(m_settings->dataFiles(DataFileType::ConcertLogo), DataFileType::ConcertLogo);
    ui->concertPosterList->setDataFiles(m_settings->dataFiles(DataFileType::ConcertPoster), DataFileType::ConcertPoster);
    ui->tvShowNfoList->setDataFiles(m_settings->dataFiles(DataFileType::TvShowNfo), DataFileType::TvShowNfo);
    ui->tvShowBackdropList->setDataFiles(m_settings->dataFiles(DataFileType::TvShowBackdrop), DataFileType::TvShowBackdrop);
    ui->tvShowClearArtList->setDataFiles(m_settings->dataFiles(DataFileType::TvShowClearArt), DataFileType::TvShowClearArt);
    ui->tvShowLogoList->setDataFiles(m_settings->dataFiles(DataFileType::TvShowLogo), DataFileType::TvShowLogo);
    ui->tvShowPosterList->setDataFiles(m_settings->dataFiles(DataFileType::TvShowPoster), DataFileType::TvShowPoster);
    ui->tvShowBannerList->setDataFiles(m_settings->dataFiles(DataFileType::TvShowBanner), DataFileType::TvShowBanner);
    ui->tvShowCharacterArtList->setDataFiles(m_settings->dataFiles(DataFileType::TvShowCharacterArt), DataFileType::TvShowCharacterArt);
    ui->tvShowSeasonPosterList->setDataFiles(m_settings->dataFiles(DataFileType::TvShowSeasonPoster), DataFileType::TvShowSeasonPoster);
    ui->tvShowEpisodeNfoList->setDataFiles(m_settings->dataFiles(DataFileType::TvShowEpisodeNfo), DataFileType::TvShowEpisodeNfo);
    ui->tvShowEpisodeThumbList->setDataFiles(m_settings->dataFiles(DataFileType::TvShowEpisodeThumb), DataFileType::TvShowEpisodeThumb);

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
    int mediaCenterInterface = -1;
    if (ui->radioXbmcXml->isChecked())
        mediaCenterInterface = MediaCenterInterfaces::XbmcXml;
    else if (ui->radioXbmcMysql->isChecked())
        mediaCenterInterface = MediaCenterInterfaces::XbmcMysql;
    else if (ui->radioXbmcSqlite->isChecked())
        mediaCenterInterface = MediaCenterInterfaces::XbmcSqlite;
    m_settings->setMediaCenterInterface(mediaCenterInterface);

    QList<DataFile> dataFiles;
    dataFiles << ui->movieNfoList->dataFiles() << ui->movieBackdropList->dataFiles() << ui->movieCdArtList->dataFiles()
              << ui->movieClearArtList->dataFiles() << ui->movieLogoList->dataFiles() << ui->moviePosterList->dataFiles();
    dataFiles << ui->tvShowBackdropList->dataFiles() << ui->tvShowBannerList->dataFiles() << ui->tvShowCharacterArtList->dataFiles()
              << ui->tvShowClearArtList->dataFiles() << ui->tvShowEpisodeNfoList->dataFiles() << ui->tvShowEpisodeThumbList->dataFiles()
              << ui->tvShowLogoList->dataFiles() << ui->tvShowNfoList->dataFiles() << ui->tvShowPosterList->dataFiles()
              << ui->tvShowSeasonPosterList->dataFiles();
    dataFiles << ui->concertNfoList->dataFiles() << ui->concertBackdropList->dataFiles() << ui->concertCdArtList->dataFiles()
              << ui->concertClearArtList->dataFiles() << ui->concertLogoList->dataFiles() << ui->concertPosterList->dataFiles();
    m_settings->setDataFiles(dataFiles);

    m_settings->setXbmcMysqlHost(ui->inputHost->text());
    m_settings->setXbmcMysqlDatabase(ui->inputDatabase->text());
    m_settings->setXbmcMysqlUser(ui->inputUsername->text());
    m_settings->setXbmcMysqlPassword(ui->inputPassword->text());
    m_settings->setXbmcSqliteDatabase(ui->inputSqliteDatabase->text());
    m_settings->setUseYoutubePluginUrls(ui->useYoutubePluginUrls->isChecked());

    // Proxy
    m_settings->setUseProxy(ui->chkUseProxy->isChecked());
    m_settings->setProxyType(ui->proxyType->currentIndex());
    m_settings->setProxyHost(ui->proxyHost->text());
    m_settings->setProxyPort(ui->proxyPort->value());
    m_settings->setProxyUsername(ui->proxyUsername->text());
    m_settings->setProxyPassword(ui->proxyPassword->text());

    m_settings->setUsePlotForOutline(ui->usePlotForOutline->isChecked());

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

    // Save Directories
    QList<SettingsDir> movieDirectories;
    QList<SettingsDir> tvShowDirectories;
    QList<SettingsDir> concertDirectories;
    for (int row=0, n=ui->dirs->rowCount() ; row<n ; ++row) {
        SettingsDir dir;
        dir.path = ui->dirs->item(row, 1)->text();
        dir.mediaCenterPath = ui->dirs->item(row, 2)->text();
        dir.separateFolders = ui->dirs->item(row, 3)->checkState() == Qt::Checked;
        dir.autoReload = ui->dirs->item(row, 4)->checkState() == Qt::Checked;
        if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() == 0)
            movieDirectories.append(dir);
        else if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() == 1)
            tvShowDirectories.append(dir);
        else if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() == 2)
            concertDirectories.append(dir);
    }
    m_settings->setMovieDirectories(movieDirectories);
    m_settings->setTvShowDirectories(tvShowDirectories);
    m_settings->setConcertDirectories(concertDirectories);

    // exclude words
    m_settings->setExcludeWords(ui->excludeWordsText->toPlainText());

    m_settings->saveSettings();

    Manager::instance()->movieFileSearcher()->setMovieDirectories(m_settings->movieDirectories());
    Manager::instance()->tvShowFileSearcher()->setMovieDirectories(m_settings->tvShowDirectories());
    Manager::instance()->concertFileSearcher()->setConcertDirectories(m_settings->concertDirectories());
    MessageBox::instance()->showMessage(tr("Settings saved"));

    Manager::instance()->setupMediaCenterInterface();
}

/**
 * @brief Adds a directory
 * @param dir Directory to add
 * @param mediaCenterPath Media Center Path
 * @param separateFolders
 * @param dirType
 */
void SettingsWidget::addDir(QString dir, QString mediaCenterPath, bool separateFolders, bool autoReload, SettingsDirType dirType)
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
            item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
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
            box->addItems(QStringList() << tr("Movies") << tr("TV Shows") << tr("Concerts"));
            if (dirType == DirTypeMovies)
                box->setCurrentIndex(0);
            else if (dirType == DirTypeTvShows)
                box->setCurrentIndex(1);
            else if (dirType == DirTypeConcerts)
                box->setCurrentIndex(2);

            ui->dirs->setCellWidget(row, 0, box);
            ui->dirs->setItem(row, 1, item);
            ui->dirs->setItem(row, 2, new QTableWidgetItem(mediaCenterPath));
            ui->dirs->setItem(row, 3, itemCheck);
            ui->dirs->setItem(row, 4, itemCheckReload);
        }
    }
}

/**
 * @brief Removes a directory
 */
void SettingsWidget::removeDir()
{
    int row = ui->dirs->currentRow();
    if (row < 0)
        return;
    ui->dirs->removeRow(row);
}

/**
 * @brief Organize button clicked
 */
void SettingsWidget::organize()
{
    MovieFilesOrganizer* organizer = new MovieFilesOrganizer(this);
    qDebug() << "Organize Button clicked!" << ui->dirs
                ->item(ui->dirs->currentRow(), 1)->text();

    int row = ui->dirs->currentRow();
    if (static_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() != 0
            || ui->dirs->item(row, 3)->checkState() == Qt::Checked) {
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
        ui->dirs->item(ui->dirs->currentRow(), 3)->setCheckState(Qt::Checked);
        break;
      case QMessageBox::Cancel:
        break;
      default:
        break;
    }
}

/**
 * @brief Enables/disables the buttons to operate on dirs
 * @param currentRow Current row in the dir list
 */
void SettingsWidget::dirListRowChanged(int currentRow)
{
    if (currentRow < 0 || currentRow >= ui->dirs->rowCount()) {
        ui->buttonRemoveDir->setDisabled(true);
        ui->buttonMovieFilesToDirs->setDisabled(true);
    } else {
        ui->buttonRemoveDir->setDisabled(false);
        if (ui->dirs->cellWidget(currentRow, 0) != 0 && static_cast<QComboBox*>(ui->dirs->cellWidget(currentRow, 0))->currentIndex() == 0
                && ui->dirs->item(currentRow, 3)->checkState() == Qt::Unchecked) {
            ui->buttonMovieFilesToDirs->setDisabled(false);
        } else {
            ui->buttonMovieFilesToDirs->setDisabled(true);
        }
    }
}

/**
 * @brief Handles status of MediaCenter checkboxes and inputs
 */
void SettingsWidget::onMediaCenterXbmcXmlSelected()
{
    ui->labelWarning->setVisible(false);
    ui->radioXbmcXml->setChecked(true);
    ui->widgetXbmcMysql->setVisible(false);
    ui->widgetXbmcSqlite->setVisible(false);
    setXbmcThumbnailPathEnabled(false);
}

/**
 * @brief Handles status of MediaCenter checkboxes and inputs
 */
void SettingsWidget::onMediaCenterXbmcMysqlSelected()
{
    ui->labelWarning->setVisible(true);
    ui->radioXbmcMysql->setChecked(true);
    ui->widgetXbmcMysql->setVisible(true);
    ui->widgetXbmcSqlite->setVisible(false);
    setXbmcThumbnailPathEnabled(true);
}

/**
 * @brief Handles status of MediaCenter checkboxes and inputs
 */
void SettingsWidget::onMediaCenterXbmcSqliteSelected()
{
    ui->labelWarning->setVisible(true);
    ui->radioXbmcSqlite->setChecked(true);
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

/**
 * @brief Enables/Disables the proxy inputs
 */
void SettingsWidget::onUseProxy()
{
    bool enabled = ui->chkUseProxy->isChecked();
    ui->proxyType->setEnabled(enabled);
    ui->proxyHost->setEnabled(enabled);
    ui->proxyPort->setEnabled(enabled);
    ui->proxyUsername->setEnabled(enabled);
    ui->proxyPassword->setEnabled(enabled);
}

/**
 * @brief Shows a native file dialog and adds the chosen dir
 */
void SettingsWidget::chooseDirToAdd()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a directory containing your movies, TV show or concerts"), QDir::homePath());
    if (!dir.isEmpty())
        addDir(dir);
}

/**
 * @brief SettingsWidget::onAutoLoadStreamDetails
 */
void SettingsWidget::onAutoLoadStreamDetails()
{
    m_settings->setAutoLoadStreamDetails(ui->chkAutoLoadStreamDetails->isChecked());
}
