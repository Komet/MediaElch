#include "SettingsWidget.h"
#include "ui_SettingsWidget.h"
#include <QFileDialog>
#include "FilesWidget.h"
#include "Manager.h"
#include "MessageBox.h"
#include "TvShowFilesWidget.h"

SettingsWidget *SettingsWidget::m_instance;

SettingsWidget::SettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);
    m_instance = this;

    QFont font = ui->labelMovies->font();
    font.setPointSize(font.pointSize()+3);
    ui->labelMovies->setFont(font);
    ui->labelScrapers->setFont(font);
    ui->labelTvShows->setFont(font);

    ui->movieDirs->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->movieDirs->horizontalHeaderItem(2)->setToolTip(tr("Movies are in separate folders"));
    ui->movieDirs->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    ui->movieDirs->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
    ui->tvShowDirs->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->tvShowDirs->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->hasSettings()) {
            QWidget *scraperSettings = scraper->settingsWidget();
            scraperSettings->setParent(ui->groupBox_2);
            ui->verticalLayoutScrapers->addWidget(new QLabel(scraper->name(), ui->groupBox_2));
            ui->verticalLayoutScrapers->addWidget(scraperSettings);
            QFrame *line = new QFrame(ui->groupBox_2);
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            ui->verticalLayoutScrapers->addWidget(line);
        }
    }
    foreach (TvScraperInterface *scraper, Manager::instance()->tvScrapers()) {
        if (scraper->hasSettings()) {
            QWidget *scraperSettings = scraper->settingsWidget();
            scraperSettings->setParent(ui->groupBox_2);
            ui->verticalLayoutScrapers->addWidget(new QLabel(scraper->name(), ui->groupBox_2));
            ui->verticalLayoutScrapers->addWidget(scraperSettings);
            QFrame *line = new QFrame(ui->groupBox_2);
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            ui->verticalLayoutScrapers->addWidget(line);
        }
    }
    ui->verticalLayoutScrapers->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

    connect(ui->buttonAddMovieDir, SIGNAL(clicked()), this, SLOT(addMovieDir()));
    connect(ui->buttonRemoveMovieDir, SIGNAL(clicked()), this, SLOT(removeMovieDir()));
    connect(ui->movieDirs, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(movieListRowChanged(int)));
    connect(ui->buttonAddTvShowDir, SIGNAL(clicked()), this, SLOT(addTvShowDir()));
    connect(ui->buttonRemoveTvShowDir, SIGNAL(clicked()), this, SLOT(removeTvShowDir()));
    connect(ui->tvShowDirs, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(tvShowListRowChanged(int)));
    connect(ui->movieDirs, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(movieMediaCenterPathChanged(QTableWidgetItem*)));
    connect(ui->tvShowDirs, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(tvShowMediaCenterPathChanged(QTableWidgetItem*)));

    connect(ui->radioXbmcXml, SIGNAL(clicked()), this, SLOT(onMediaCenterXbmcXmlSelected()));
    connect(ui->radioXbmcMysql, SIGNAL(clicked()), this, SLOT(onMediaCenterXbmcMysqlSelected()));
    connect(ui->radioXbmcSqlite, SIGNAL(clicked()), this, SLOT(onMediaCenterXbmcSqliteSelected()));
    connect(ui->buttonSelectSqliteDatabase, SIGNAL(clicked()), this, SLOT(onChooseMediaCenterXbmcSqliteDatabase()));
    connect(ui->buttonSelectThumbnailPath, SIGNAL(clicked()), this, SLOT(onChooseXbmcThumbnailPath()));

    this->loadSettings();
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}

SettingsWidget *SettingsWidget::instance()
{
    return m_instance;
}

void SettingsWidget::loadSettings()
{
    // Globals
    m_firstTime = m_settings.value("FirstTime", true).toBool();
    m_mainWindowSize = m_settings.value("MainWindowSize").toSize();
    m_mainWindowPosition = m_settings.value("MainWindowPosition").toPoint();
    m_movieSplitterState = m_settings.value("MovieSplitterState").toByteArray();
    m_tvShowSplitterState = m_settings.value("TvShowSplitterState").toByteArray();
    m_movieSetsSplitterState = m_settings.value("MovieSetsSplitterState").toByteArray();

    // Movie Directories
    m_movieDirectories.clear();
    ui->movieDirs->setRowCount(0);
    ui->movieDirs->clearContents();
    int moviesSize = m_settings.beginReadArray("Directories/Movies");
    for (int i=0 ; i<moviesSize ; ++i) {
        m_settings.setArrayIndex(i);
        ui->movieDirs->insertRow(i);
        QTableWidgetItem *item = new QTableWidgetItem(m_settings.value("path").toString());
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        item->setToolTip(m_settings.value("path").toString());
        QTableWidgetItem *item2 = new QTableWidgetItem;
        item2->setToolTip(tr("Movies are in separate folders"));
        if (m_settings.value("sepFolders", false).toBool())
            item2->setCheckState(Qt::Checked);
        else
            item2->setCheckState(Qt::Unchecked);
        ui->movieDirs->setItem(i, 0, item);
        ui->movieDirs->setItem(i, 1, new QTableWidgetItem(m_settings.value("mediaCenterPath").toString()));
        ui->movieDirs->setItem(i, 2, item2);
        SettingsDir dir;
        dir.path = m_settings.value("path").toString();
        dir.mediaCenterPath = m_settings.value("mediaCenterPath").toString();
        dir.separateFolders = m_settings.value("sepFolders", false).toBool();
        m_movieDirectories.append(dir);
    }
    m_settings.endArray();
    ui->buttonRemoveMovieDir->setEnabled(!m_movieDirectories.isEmpty());

    // TV Show Directories
    m_tvShowDirectories.clear();
    ui->tvShowDirs->setRowCount(0);
    ui->tvShowDirs->clearContents();
    int tvShowSize = m_settings.beginReadArray("Directories/TvShows");
    for (int i=0 ; i<tvShowSize ; ++i) {
        m_settings.setArrayIndex(i);
        ui->tvShowDirs->insertRow(i);
        QTableWidgetItem *item = new QTableWidgetItem(m_settings.value("path").toString());
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        item->setToolTip(m_settings.value("path").toString());
        ui->tvShowDirs->setItem(i, 0, item);
        ui->tvShowDirs->setItem(i, 1, new QTableWidgetItem(m_settings.value("mediaCenterPath").toString()));
        SettingsDir dir;
        dir.path = m_settings.value("path").toString();
        dir.mediaCenterPath = m_settings.value("mediaCenterPath").toString();
        m_tvShowDirectories.append(dir);
    }
    m_settings.endArray();
    ui->buttonRemoveTvShowDir->setEnabled(!m_tvShowDirectories.isEmpty());

    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->hasSettings())
            scraper->loadSettings();
    }
    foreach (TvScraperInterface *scraper, Manager::instance()->tvScrapers()) {
        if (scraper->hasSettings())
            scraper->loadSettings();
    }

    // MediaCenterInterface
    m_mediaCenterInterface = m_settings.value("MediaCenterInterface", MediaCenterInterfaces::XbmcXml).toInt();
    if (m_mediaCenterInterface == MediaCenterInterfaces::XbmcXml)
        onMediaCenterXbmcXmlSelected();
    else if (m_mediaCenterInterface == MediaCenterInterfaces::XbmcMysql)
        onMediaCenterXbmcMysqlSelected();
    else if (m_mediaCenterInterface == MediaCenterInterfaces::XbmcSqlite)
        onMediaCenterXbmcSqliteSelected();

    m_xbmcMysqlHost      = m_settings.value("XbmcMysql/Host").toString();
    m_xbmcMysqlDatabase  = m_settings.value("XbmcMysql/Database").toString();
    m_xbmcMysqlUser      = m_settings.value("XbmcMysql/User").toString();
    m_xbmcMysqlPassword  = m_settings.value("XbmcMysql/Password").toString();
    m_xbmcSqliteDatabase = m_settings.value("XbmcSqlite/Database").toString();

    ui->inputDatabase->setText(m_xbmcMysqlDatabase);
    ui->inputHost->setText(m_xbmcMysqlHost);
    ui->inputUsername->setText(m_xbmcMysqlUser);
    ui->inputPassword->setText(m_xbmcMysqlPassword);
    ui->inputSqliteDatabase->setText(m_xbmcSqliteDatabase);

    m_xbmcThumbnailPath = m_settings.value("XbmcThumbnailpath").toString();
    ui->inputThumbnailPath->setText(m_xbmcThumbnailPath);
    ui->inputThumbnailPath->setToolTip(m_xbmcThumbnailPath);
}

void SettingsWidget::saveSettings()
{
    bool mediaInterfaceChanged = false;
    m_settings.setValue("FirstTime", false);

    m_settings.beginWriteArray("Directories/Movies");
    for (int i=0, n=m_movieDirectories.count() ; i<n ; ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("path", m_movieDirectories.at(i).path);
        m_settings.setValue("mediaCenterPath", m_movieDirectories.at(i).mediaCenterPath);
        m_settings.setValue("sepFolders", m_movieDirectories.at(i).separateFolders);
    }
    m_settings.endArray();

    m_settings.beginWriteArray("Directories/TvShows");
    for (int i=0, n=m_tvShowDirectories.count() ; i<n ; ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("path", m_tvShowDirectories.at(i).path);
        m_settings.setValue("mediaCenterPath", m_tvShowDirectories.at(i).mediaCenterPath);
    }
    m_settings.endArray();

    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->hasSettings())
            scraper->saveSettings();
    }
    foreach (TvScraperInterface *scraper, Manager::instance()->tvScrapers()) {
        if (scraper->hasSettings())
            scraper->saveSettings();
    }

    if (ui->radioXbmcXml->isChecked()) {
        if (m_mediaCenterInterface != MediaCenterInterfaces::XbmcXml)
            mediaInterfaceChanged = true;
        m_mediaCenterInterface = MediaCenterInterfaces::XbmcXml;
    } else if (ui->radioXbmcMysql->isChecked()) {
        if (m_mediaCenterInterface != MediaCenterInterfaces::XbmcMysql)
            mediaInterfaceChanged = true;
        m_mediaCenterInterface = MediaCenterInterfaces::XbmcMysql;
    } else if (ui->radioXbmcSqlite->isChecked()) {
        if (m_mediaCenterInterface != MediaCenterInterfaces::XbmcSqlite)
            mediaInterfaceChanged = true;
        m_mediaCenterInterface = MediaCenterInterfaces::XbmcSqlite;
    }
    m_xbmcMysqlHost      = ui->inputHost->text();
    m_xbmcMysqlDatabase  = ui->inputDatabase->text();
    m_xbmcMysqlUser      = ui->inputUsername->text();
    m_xbmcMysqlPassword  = ui->inputPassword->text();
    m_xbmcSqliteDatabase = ui->inputSqliteDatabase->text();

    m_settings.setValue("XbmcMysql/Host", m_xbmcMysqlHost);
    m_settings.setValue("XbmcMysql/Database", m_xbmcMysqlDatabase);
    m_settings.setValue("XbmcMysql/User", m_xbmcMysqlUser);
    m_settings.setValue("XbmcMysql/Password", m_xbmcMysqlPassword);
    m_settings.setValue("XbmcSqlite/Database", m_xbmcSqliteDatabase);
    m_settings.setValue("MediaCenterInterface", m_mediaCenterInterface);
    m_settings.setValue("XbmcThumbnailpath", m_xbmcThumbnailPath);

    Manager::instance()->movieFileSearcher()->setMovieDirectories(this->movieDirectories());
    Manager::instance()->tvShowFileSearcher()->setMovieDirectories(this->tvShowDirectories());
    MessageBox::instance()->showMessage(tr("Settings saved"));

    Manager::instance()->setupMediaCenterInterface();
    if (mediaInterfaceChanged) {
        // TvShow File Searcher is started when Movie File Searcher has finished @see MainWindow.cpp
        QTimer::singleShot(0, FilesWidget::instance(), SLOT(startSearch()));
    }
}

void SettingsWidget::addMovieDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a directory containing your movies"), QDir::homePath());
    if (!dir.isEmpty()) {
        bool exists = false;
        for (int i=0, n=m_movieDirectories.count() ; i<n ; ++i) {
            if (m_movieDirectories.at(i).path == dir)
                exists = true;
        }

        if (!exists) {
            SettingsDir sDir;
            sDir.path = dir;
            m_movieDirectories.append(sDir);
            int row = ui->movieDirs->rowCount();
            ui->movieDirs->insertRow(row);
            QTableWidgetItem *item = new QTableWidgetItem(dir);
            item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            item->setToolTip(dir);
            ui->movieDirs->setItem(row, 0, item);
            ui->movieDirs->setItem(row, 1, new QTableWidgetItem(""));
        }
    }
}

void SettingsWidget::removeMovieDir()
{
    int row = ui->movieDirs->currentRow();
    if (row < 0)
        return;

    m_movieDirectories.removeAt(row);
    ui->movieDirs->removeRow(row);
}

void SettingsWidget::movieListRowChanged(int currentRow)
{
    ui->buttonRemoveMovieDir->setDisabled(currentRow < 0);
}

void SettingsWidget::addTvShowDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a directory containing your TV shows"), QDir::homePath());
    if (!dir.isEmpty()) {
        bool exists = false;
        for (int i=0, n=m_tvShowDirectories.count() ; i<n ; ++i) {
            if (m_tvShowDirectories.at(i).path == dir)
                exists = true;
        }

        if (!exists) {
            SettingsDir sDir;
            sDir.path = dir;
            m_tvShowDirectories.append(sDir);
            int row = ui->tvShowDirs->rowCount();
            ui->tvShowDirs->insertRow(row);
            QTableWidgetItem *item = new QTableWidgetItem(dir);
            item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            item->setToolTip(dir);
            ui->tvShowDirs->setItem(row, 0, item);
            ui->tvShowDirs->setItem(row, 1, new QTableWidgetItem(""));
        }
    }
}

void SettingsWidget::removeTvShowDir()
{
    int row = ui->tvShowDirs->currentRow();
    if (row<0)
        return;

    m_tvShowDirectories.removeAt(row);
    ui->tvShowDirs->removeRow(row);
}

void SettingsWidget::tvShowListRowChanged(int currentRow)
{
    ui->buttonRemoveTvShowDir->setDisabled(currentRow < 0);
}

void SettingsWidget::movieMediaCenterPathChanged(QTableWidgetItem *item)
{
    if (item->row() < 0 || item->row() >= m_movieDirectories.count())
        return;

    if (item->column() == 1)
        m_movieDirectories[item->row()].mediaCenterPath = item->text();
    else if (item->column() == 2)
        m_movieDirectories[item->row()].separateFolders = item->checkState() == Qt::Checked;
}

void SettingsWidget::tvShowMediaCenterPathChanged(QTableWidgetItem *item)
{
    if (item->row() < 0 || item->row() >= m_tvShowDirectories.count() || item->column() != 1)
        return;
    m_tvShowDirectories[item->row()].mediaCenterPath = item->text();
}

void SettingsWidget::onMediaCenterXbmcXmlSelected()
{
    ui->radioXbmcXml->setChecked(true);
    ui->widgetXbmcMysql->setEnabled(false);
    ui->widgetXbmcSqlite->setEnabled(false);
    setXbmcThumbnailPathEnabled(false);
}

void SettingsWidget::onMediaCenterXbmcMysqlSelected()
{
    ui->radioXbmcMysql->setChecked(true);
    ui->widgetXbmcMysql->setEnabled(true);
    ui->widgetXbmcSqlite->setEnabled(false);
    setXbmcThumbnailPathEnabled(true);
}

void SettingsWidget::onMediaCenterXbmcSqliteSelected()
{
    ui->radioXbmcSqlite->setChecked(true);
    ui->widgetXbmcMysql->setEnabled(false);
    ui->widgetXbmcSqlite->setEnabled(true);
    setXbmcThumbnailPathEnabled(true);
}

void SettingsWidget::onChooseMediaCenterXbmcSqliteDatabase()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("SQLite Database *.db"));
    if (dialog.exec()) {
        QStringList files = dialog.selectedFiles();
        if (files.count() == 1) {
            ui->inputSqliteDatabase->setText(files.at(0));
            m_xbmcSqliteDatabase = files.at(0);
        }
    }
}

void SettingsWidget::onChooseXbmcThumbnailPath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a directory containing your Thumbnails"), QDir::homePath());
    if (!dir.isEmpty()) {
        m_xbmcThumbnailPath = dir;
        ui->inputThumbnailPath->setText(dir);
        ui->inputThumbnailPath->setToolTip(dir);
    }
}

void SettingsWidget::setXbmcThumbnailPathEnabled(bool enabled)
{
    ui->inputThumbnailPath->setEnabled(enabled);
    ui->buttonSelectThumbnailPath->setEnabled(enabled);
    ui->labelXbmcThumbnailPath->setEnabled(enabled);
    ui->labelXbmcThumbnailPathDesc->setEnabled(enabled);
}

/*** GETTER ***/

QSize SettingsWidget::mainWindowSize()
{
    return m_mainWindowSize;
}

QPoint SettingsWidget::mainWindowPosition()
{
    return m_mainWindowPosition;
}

QByteArray SettingsWidget::movieSplitterState()
{
    return m_movieSplitterState;
}

QByteArray SettingsWidget::tvShowSplitterState()
{
    return m_tvShowSplitterState;
}

QByteArray SettingsWidget::movieSetsSplitterState()
{
    return m_movieSetsSplitterState;
}

QList<SettingsDir> SettingsWidget::movieDirectories()
{
    return m_movieDirectories;
}

QList<SettingsDir> SettingsWidget::tvShowDirectories()
{
    return m_tvShowDirectories;
}

bool SettingsWidget::firstTime()
{
    return m_firstTime;
}

int SettingsWidget::mediaCenterInterface()
{
    return m_mediaCenterInterface;
}

QString SettingsWidget::xbmcMysqlHost()
{
    return m_xbmcMysqlHost;
}

QString SettingsWidget::xbmcMysqlDatabase()
{
    return m_xbmcMysqlDatabase;
}

QString SettingsWidget::xbmcMysqlUser()
{
    return m_xbmcMysqlUser;
}

QString SettingsWidget::xbmcMysqlPassword()
{
    return m_xbmcMysqlPassword;
}

QString SettingsWidget::xbmcSqliteDatabase()
{
    return m_xbmcSqliteDatabase;
}

QString SettingsWidget::xbmcThumbnailPath()
{
    return m_xbmcThumbnailPath;
}

/*** SETTER ***/

void SettingsWidget::setMainWindowSize(QSize mainWindowSize)
{
    int height = mainWindowSize.height();
    // don't know where 42px are left...
#ifdef Q_WS_MAC
    height += 42;
#endif
    mainWindowSize.setHeight(height);
    m_mainWindowSize = mainWindowSize;
    m_settings.setValue("MainWindowSize", mainWindowSize);
}

void SettingsWidget::setMainWindowPosition(QPoint mainWindowPosition)
{
    m_mainWindowPosition = mainWindowPosition;
    m_settings.setValue("MainWindowPosition", mainWindowPosition);
}

void SettingsWidget::setMovieSplitterState(QByteArray state)
{
    m_movieSplitterState = state;
    m_settings.setValue("MovieSplitterState", state);
}

void SettingsWidget::setTvShowSplitterState(QByteArray state)
{
    m_tvShowSplitterState = state;
    m_settings.setValue("TvShowSplitterState", state);
}

void SettingsWidget::setMovieSetsSplitterState(QByteArray state)
{
    m_movieSetsSplitterState = state;
    m_settings.setValue("MovieSetsSplitterState", state);
}
