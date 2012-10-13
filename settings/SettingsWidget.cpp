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
    m_dirDialog = new QFileDialog(this, tr("Choose a directory containing your movies, TV show or concerts"), QDir::homePath());
    m_dirDialog->setFileMode(QFileDialog::Directory);
    m_dirDialog->setOption(QFileDialog::ShowDirsOnly, true);
    m_xbmcThumbnailDirDialog = new QFileDialog(this, tr("Choose a directory containing your Thumbnails"), QDir::homePath());;
    m_xbmcThumbnailDirDialog->setFileMode(QFileDialog::Directory);
    m_xbmcThumbnailDirDialog->setOption(QFileDialog::ShowDirsOnly, true);
    m_xbmcSqliteDatabaseDialog = new QFileDialog(this, tr("SQLite Database *.db"), QDir::homePath());
    m_xbmcSqliteDatabaseDialog->setFileMode(QFileDialog::ExistingFile);

    connect(m_logFileDialog, SIGNAL(fileSelected(QString)), this, SLOT(onDebugLogPathChosen(QString)));
    connect(m_dirDialog, SIGNAL(fileSelected(QString)), this, SLOT(addDir(QString)));
    connect(m_xbmcThumbnailDirDialog, SIGNAL(fileSelected(QString)), this, SLOT(onChooseXbmcThumbnailPath(QString)));
    connect(m_xbmcSqliteDatabaseDialog, SIGNAL(fileSelected(QString)), this, SLOT(onChooseMediaCenterXbmcSqliteDatabase(QString)));

    connect(ui->buttonAddDir, SIGNAL(clicked()), m_dirDialog, SLOT(open()));
    connect(ui->buttonRemoveDir, SIGNAL(clicked()), this, SLOT(removeDir()));
    connect(ui->buttonMovieFilesToDirs, SIGNAL(clicked()), this, SLOT(organize()));
    connect(ui->dirs, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(dirListRowChanged(int)));

    connect(ui->chkActivateDebug, SIGNAL(clicked()), this, SLOT(onActivateDebugMode()));
    connect(ui->buttonChooseLogfile, SIGNAL(clicked()), m_logFileDialog, SLOT(open()));
    connect(ui->logfilePath, SIGNAL(textChanged(QString)), this, SLOT(onSetDebugLogPath(QString)));
    connect(ui->chkUseCache, SIGNAL(clicked()), this, SLOT(onActivateCache()));
    connect(ui->btnClearCache, SIGNAL(clicked()), this, SLOT(onClearCache()));
    connect(ui->chkUseProxy, SIGNAL(clicked()), this, SLOT(onUseProxy()));

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

    // Cache
    ui->chkUseCache->setChecked(m_settings->useCache());
    onActivateCache();

    // Proxy
    ui->chkUseProxy->setChecked(m_settings->useProxy());
    ui->proxyType->setCurrentIndex(m_settings->proxyType());
    ui->proxyHost->setText(m_settings->proxyHost());
    ui->proxyPort->setValue(m_settings->proxyPort());
    ui->proxyUsername->setText(m_settings->proxyUsername());
    ui->proxyPassword->setText(m_settings->proxyPassword());
    onUseProxy();

    // Directories
    ui->dirs->setRowCount(0);
    ui->dirs->clearContents();
    QList<SettingsDir> movieDirectories = m_settings->movieDirectories();
    for (int i=0, n=movieDirectories.count() ; i<n ; ++i)
        addDir(movieDirectories.at(i).path, movieDirectories.at(i).mediaCenterPath, movieDirectories.at(i).separateFolders, DirTypeMovies);
    QList<SettingsDir> tvShowDirectories = m_settings->tvShowDirectories();
    for (int i=0, n=tvShowDirectories.count() ; i<n ; ++i)
        addDir(tvShowDirectories.at(i).path, tvShowDirectories.at(i).mediaCenterPath, tvShowDirectories.at(i).separateFolders, DirTypeTvShows);
    QList<SettingsDir> concertDirectories = m_settings->concertDirectories();
    for (int i=0, n=concertDirectories.count() ; i<n ; ++i)
        addDir(concertDirectories.at(i).path, concertDirectories.at(i).mediaCenterPath, concertDirectories.at(i).separateFolders, DirTypeConcerts);

    ui->buttonRemoveDir->setEnabled(ui->dirs->rowCount() > 0);

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
    int mediaCenterInterface = -1;
    if (ui->radioXbmcXml->isChecked())
        mediaCenterInterface = MediaCenterInterfaces::XbmcXml;
    else if (ui->radioXbmcMysql->isChecked())
        mediaCenterInterface = MediaCenterInterfaces::XbmcMysql;
    else if (ui->radioXbmcSqlite->isChecked())
        mediaCenterInterface = MediaCenterInterfaces::XbmcSqlite;
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

    // Proxy
    m_settings->setUseProxy(ui->chkUseProxy->isChecked());
    m_settings->setProxyType(ui->proxyType->currentIndex());
    m_settings->setProxyHost(ui->proxyHost->text());
    m_settings->setProxyPort(ui->proxyPort->value());
    m_settings->setProxyUsername(ui->proxyUsername->text());
    m_settings->setProxyPassword(ui->proxyPassword->text());

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
void SettingsWidget::addDir(QString dir, QString mediaCenterPath, bool separateFolders, SettingsDirType dirType)
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
    qDebug() << "ORGANIZEE!";
}

/**
 * @brief Enables/disables the buttons to operate on dirs
 * @param currentRow Current row in the dir list
 */
void SettingsWidget::dirListRowChanged(int currentRow)
{
    if (currentRow < 0) {
        ui->buttonRemoveDir->setDisabled(true);
        ui->buttonMovieFilesToDirs->setDisabled(true);

    }
    else {
        ui->buttonRemoveDir->setDisabled(false);

        QComboBox *temp = ((QComboBox*)ui->dirs->cellWidget(currentRow, 0));
        if (temp->currentIndex() == 0) {
            ui->buttonMovieFilesToDirs->setDisabled(false);
        }
        else
            ui->buttonMovieFilesToDirs->setDisabled(true);
    }
    //if (((QComboBox)ui->dirs->cellWidget(currentRow, 0))->currentIndex() == 0)
      //  qDebug() << "YEAH";

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
 * @brief Toggles status of the clear cache button
 */
void SettingsWidget::onActivateCache()
{
    ui->btnClearCache->setEnabled(ui->chkUseCache->isChecked());
    m_settings->setUseCache(ui->chkUseCache->isChecked());
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
 * @brief Clears the cache database
 */
void SettingsWidget::onClearCache()
{
    Manager::instance()->clearCacheDatabase();
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
