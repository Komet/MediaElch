#include "SettingsWidget.h"
#include "ui_SettingsWidget.h"

#include <QComboBox>
#include <QIntValidator>
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

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    m_settings = Settings::instance(this);

    ui->xbmcPort->setValidator(new QIntValidator(0, 99999, ui->xbmcPort));

#if QT_VERSION >= 0x050000
    ui->dirs->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->dirs->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
#else
    ui->dirs->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->dirs->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
#endif
    ui->dirs->horizontalHeaderItem(2)->setToolTip(tr("Items are in separate folders"));
    ui->dirs->horizontalHeaderItem(3)->setToolTip(tr("Automatically reload contents on start"));

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

    ui->comboMovieSetArtwork->setItemData(0, MovieSetArtworkSingleSetFolder);
    ui->comboMovieSetArtwork->setItemData(1, MovieSetArtworkSingleArtworkFolder);

    // Media Status Columns
    for (int i=MediaStatusFirst, n=MediaStatusLast ; i<=n ; ++i) {
        QListWidgetItem *item = new QListWidgetItem(MovieModel::mediaStatusToText(static_cast<MediaStatusColumns>(i)));
        item->setData(Qt::UserRole, i);
        item->setCheckState(Qt::Unchecked);
        ui->mediaStatusColumns->addItem(item);
    }

    connect(ui->buttonAddDir, SIGNAL(clicked()), this, SLOT(chooseDirToAdd()));
    connect(ui->buttonRemoveDir, SIGNAL(clicked()), this, SLOT(removeDir()));
    connect(ui->buttonMovieFilesToDirs, SIGNAL(clicked()), this, SLOT(organize()));
    connect(ui->dirs, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(dirListRowChanged(int)));
    connect(ui->comboMovieSetArtwork, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboMovieSetArtworkChanged()));
    connect(ui->btnMovieSetArtworkDir, SIGNAL(clicked()), this, SLOT(onChooseMovieSetArtworkDir()));
    connect(ui->chkUseProxy, SIGNAL(clicked()), this, SLOT(onUseProxy()));
    connect(ui->btnDefaultsEden, SIGNAL(clicked()), this, SLOT(onDefaultsEden()));
    connect(ui->btnDefaultsFrodo, SIGNAL(clicked()), this, SLOT(onDefaultsFrodo()));

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

    fillDataFiles();

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

    // Movie set artwork
    for (int i=0, n=ui->comboMovieSetArtwork->count() ; i<n ; ++i) {
        if (ui->comboMovieSetArtwork->itemData(i).toInt() == m_settings->movieSetArtworkType()) {
            ui->comboMovieSetArtwork->setCurrentIndex(i);
            break;
        }
    }
    ui->movieSetArtworkDir->setText(m_settings->movieSetArtworkDirectory());
    ui->movieSetPosterFileName->setText(m_settings->movieSetPosterFileName());
    ui->movieSetFanartFileName->setText(m_settings->movieSetFanartFileName());
    onComboMovieSetArtworkChanged();

    // Media Status Columns
    for (int i=0, n=ui->mediaStatusColumns->count() ; i<n ; ++i) {
        ui->mediaStatusColumns->item(i)->setCheckState(
            m_settings->mediaStatusColumns().contains(static_cast<MediaStatusColumns>(ui->mediaStatusColumns->item(i)->data(Qt::UserRole).toInt())) ?
                        Qt::Checked : Qt::Unchecked);
    }
}

/**
 * @brief Saves all settings
 */
void SettingsWidget::saveSettings()
{
    QList<DataFile> dataFiles;
    dataFiles << ui->movieNfoList->dataFiles() << ui->movieBackdropList->dataFiles() << ui->movieCdArtList->dataFiles()
              << ui->movieClearArtList->dataFiles() << ui->movieLogoList->dataFiles() << ui->moviePosterList->dataFiles();
    dataFiles << ui->tvShowBackdropList->dataFiles() << ui->tvShowBannerList->dataFiles() << ui->tvShowCharacterArtList->dataFiles()
              << ui->tvShowClearArtList->dataFiles() << ui->tvShowEpisodeNfoList->dataFiles() << ui->tvShowEpisodeThumbList->dataFiles()
              << ui->tvShowLogoList->dataFiles() << ui->tvShowNfoList->dataFiles() << ui->tvShowPosterList->dataFiles()
              << ui->tvShowSeasonPosterList->dataFiles() << ui->tvShowSeasonBackdropList->dataFiles() << ui->tvShowSeasonBannerList->dataFiles();
    dataFiles << ui->concertNfoList->dataFiles() << ui->concertBackdropList->dataFiles() << ui->concertCdArtList->dataFiles()
              << ui->concertClearArtList->dataFiles() << ui->concertLogoList->dataFiles() << ui->concertPosterList->dataFiles();
    m_settings->setDataFiles(dataFiles);

    m_settings->setUseYoutubePluginUrls(ui->useYoutubePluginUrls->isChecked());
    m_settings->setAutoLoadStreamDetails(ui->chkAutoLoadStreamDetails->isChecked());
    m_settings->setDownloadActorImages(ui->chkDownloadActorImages->isChecked());
    m_settings->setIgnoreArticlesWhenSorting(ui->chkIgnoreArticlesWhenSorting->isChecked());

    m_settings->setXbmcHost(ui->xbmcHost->text());
    m_settings->setXbmcPort(ui->xbmcPort->text().toInt());

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
        dir.separateFolders = ui->dirs->item(row, 2)->checkState() == Qt::Checked;
        dir.autoReload = ui->dirs->item(row, 3)->checkState() == Qt::Checked;
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

    // Movie set artwork
    m_settings->setMovieSetArtworkType(static_cast<MovieSetArtworkType>(ui->comboMovieSetArtwork->itemData(ui->comboMovieSetArtwork->currentIndex()).toInt()));
    m_settings->setMovieSetArtworkDirectory(ui->movieSetArtworkDir->text());
    m_settings->setMovieSetPosterFileName(ui->movieSetPosterFileName->text());
    m_settings->setMovieSetFanartFileName(ui->movieSetFanartFileName->text());

    QList<MediaStatusColumns> columns;
    for (int i=0, n=ui->mediaStatusColumns->count() ; i<n ; ++i) {
        if (ui->mediaStatusColumns->item(i)->checkState() == Qt::Checked)
            columns.append(static_cast<MediaStatusColumns>(ui->mediaStatusColumns->item(i)->data(Qt::UserRole).toInt()));
    }
    m_settings->setMediaStatusColumns(columns);

    m_settings->saveSettings();

    Manager::instance()->movieFileSearcher()->setMovieDirectories(m_settings->movieDirectories());
    Manager::instance()->tvShowFileSearcher()->setMovieDirectories(m_settings->tvShowDirectories());
    Manager::instance()->concertFileSearcher()->setConcertDirectories(m_settings->concertDirectories());
    MessageBox::instance()->showMessage(tr("Settings saved"));
}

/**
 * @brief Adds a directory
 * @param dir Directory to add
 * @param separateFolders
 * @param dirType
 */
void SettingsWidget::addDir(QString dir, bool separateFolders, bool autoReload, SettingsDirType dirType)
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
            ui->dirs->setItem(row, 2, itemCheck);
            ui->dirs->setItem(row, 3, itemCheckReload);
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
        if (ui->dirs->cellWidget(currentRow, 0) != 0
                && static_cast<QComboBox*>(ui->dirs->cellWidget(currentRow, 0))->currentIndex() == 0
                && ui->dirs->item(currentRow, 2)->checkState() == Qt::Unchecked) {
            ui->buttonMovieFilesToDirs->setDisabled(false);
        } else {
            ui->buttonMovieFilesToDirs->setDisabled(true);
        }
    }
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

void SettingsWidget::onDefaultsEden()
{
    m_settings->loadEdenDefaults();
    fillDataFiles();
}

void SettingsWidget::onDefaultsFrodo()
{
    m_settings->loadFrodoDefaults();
    fillDataFiles();
}

void SettingsWidget::fillDataFiles()
{
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
    ui->tvShowSeasonBackdropList->setDataFiles(m_settings->dataFiles(DataFileType::TvShowSeasonBackdrop), DataFileType::TvShowSeasonBackdrop);
    ui->tvShowSeasonBannerList->setDataFiles(m_settings->dataFiles(DataFileType::TvShowSeasonBanner), DataFileType::TvShowSeasonBanner);
    ui->tvShowEpisodeNfoList->setDataFiles(m_settings->dataFiles(DataFileType::TvShowEpisodeNfo), DataFileType::TvShowEpisodeNfo);
    ui->tvShowEpisodeThumbList->setDataFiles(m_settings->dataFiles(DataFileType::TvShowEpisodeThumb), DataFileType::TvShowEpisodeThumb);
}

void SettingsWidget::onComboMovieSetArtworkChanged()
{
    ui->btnMovieSetArtworkDir->setEnabled(ui->comboMovieSetArtwork->itemData(ui->comboMovieSetArtwork->currentIndex()).toInt() == MovieSetArtworkSingleArtworkFolder);
    ui->movieSetArtworkDir->setEnabled(ui->comboMovieSetArtwork->itemData(ui->comboMovieSetArtwork->currentIndex()).toInt() == MovieSetArtworkSingleArtworkFolder);
}

void SettingsWidget::onChooseMovieSetArtworkDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a directory where your movie set artwork is stored"), QDir::homePath());
    if (!dir.isEmpty())
        ui->movieSetArtworkDir->setText(dir);
}
