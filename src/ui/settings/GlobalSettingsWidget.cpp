#include "ui/settings/GlobalSettingsWidget.h"
#include "ui_GlobalSettingsWidget.h"

#include "data/Storage.h"
#include "movies/MovieFilesOrganizer.h"
#include "settings/Settings.h"

#include <QFileDialog>
#include <QMessageBox>

// The directory table has four columns. We define the indices here
// to avoid more magic numbers
static constexpr int tableDirectoryTypeIndex = 0;
static constexpr int tableDirectoryPathIndex = 1;
static constexpr int tableDirectorySeparateFoldersIndex = 2;
static constexpr int tableDirectoryReloadIndex = 3;

GlobalSettingsWidget::GlobalSettingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::GlobalSettingsWidget)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    QFont smallFont = ui->labelGlobal->font();
    smallFont.setPointSize(smallFont.pointSize() - 1);
    ui->labelGlobal->setFont(smallFont);
#endif

    ui->dirs->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->dirs->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    // clang-format off
    connect(ui->buttonAddDir,           &QAbstractButton::clicked,         this, &GlobalSettingsWidget::chooseDirToAdd);
    connect(ui->buttonRemoveDir,        &QAbstractButton::clicked,         this, &GlobalSettingsWidget::removeDir);
    connect(ui->buttonMovieFilesToDirs, &QAbstractButton::clicked,         this, &GlobalSettingsWidget::organize);
    connect(ui->dirs,                   &QTableWidget::currentCellChanged, this, &GlobalSettingsWidget::dirListRowChanged);
    connect(ui->dirs,                   &QTableWidget::cellChanged,        this, &GlobalSettingsWidget::dirListEntryChanged);
    // clang-format on

    ui->comboStartupSection->addItem(tr("Movies"), "movies");
    ui->comboStartupSection->addItem(tr("TV Shows"), "tvshows");
    ui->comboStartupSection->addItem(tr("Concerts"), "concerts");
    ui->comboStartupSection->addItem(tr("Music"), "music");
    ui->comboStartupSection->addItem(tr("Import"), "import");
}

GlobalSettingsWidget::~GlobalSettingsWidget()
{
    delete ui;
}

void GlobalSettingsWidget::setSettings(Settings& settings)
{
    m_settings = &settings;
}

void GlobalSettingsWidget::chooseDirToAdd()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Choose a directory containing your movies, TV show or concerts"), QDir::homePath());
    if (dir.isEmpty()) {
        // User aborted file dialog.
        return;
    }
    QDir path(dir);
    if (path.isReadable()) {
        SettingsDir settingsDir;
        settingsDir.path = path;
        // A lot of users store their movies in separate folders.  Therefore, we set it per default.
        settingsDir.separateFolders = true;
        addDir(settingsDir);
    }
}

void GlobalSettingsWidget::loadSettings()
{
    // Stream Details
    ui->chkAutoLoadStreamDetails->setChecked(m_settings->autoLoadStreamDetails());

    ui->chkDownloadActorImages->setChecked(m_settings->downloadActorImages());
    ui->chkIgnoreArticlesWhenSorting->setChecked(m_settings->ignoreArticlesWhenSorting());
    ui->chkCheckForUpdates->setChecked(m_settings->checkForUpdates());

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
        addDir(movieDirectories.at(i), SettingsDirType::Movies);
    }
    QVector<SettingsDir> tvShowDirectories = m_settings->directorySettings().tvShowDirectories();
    for (int i = 0, n = tvShowDirectories.count(); i < n; ++i) {
        addDir(tvShowDirectories.at(i), SettingsDirType::TvShows);
    }
    QVector<SettingsDir> concertDirectories = m_settings->directorySettings().concertDirectories();
    for (int i = 0, n = concertDirectories.count(); i < n; ++i) {
        addDir(concertDirectories.at(i), SettingsDirType::Concerts);
    }
    QVector<SettingsDir> downloadDirectories = m_settings->directorySettings().downloadDirectories();
    for (int i = 0, n = downloadDirectories.count(); i < n; ++i) {
        SettingsDir dir;
        dir.path = downloadDirectories.at(i).path;
        addDir(downloadDirectories.at(i), SettingsDirType::Downloads);
    }
    QVector<SettingsDir> musicDirectories = m_settings->directorySettings().musicDirectories();
    for (int i = 0, n = musicDirectories.count(); i < n; ++i) {
        addDir(musicDirectories.at(i), SettingsDirType::Music);
    }
    dirListRowChanged(ui->dirs->currentRow());

    // Exclude words
    ui->excludeWordsText->setPlainText(m_settings->excludeWords().join(","));

    ui->useYoutubePluginUrls->setChecked(m_settings->useYoutubePluginUrls());
}

void GlobalSettingsWidget::saveSettings()
{
    m_settings->setUseYoutubePluginUrls(ui->useYoutubePluginUrls->isChecked());
    m_settings->setAutoLoadStreamDetails(ui->chkAutoLoadStreamDetails->isChecked());
    m_settings->setDownloadActorImages(ui->chkDownloadActorImages->isChecked());
    m_settings->setIgnoreArticlesWhenSorting(ui->chkIgnoreArticlesWhenSorting->isChecked());
    m_settings->setCheckForUpdates(ui->chkCheckForUpdates->isChecked());
    m_settings->setStartupSection(
        ui->comboStartupSection->itemData(ui->comboStartupSection->currentIndex()).toString());

    // save directories
    QVector<SettingsDir> movieDirectories;
    QVector<SettingsDir> tvShowDirectories;
    QVector<SettingsDir> concertDirectories;
    QVector<SettingsDir> downloadDirectories;
    QVector<SettingsDir> musicDirectories;
    for (int row = 0, n = ui->dirs->rowCount(); row < n; ++row) {
        SettingsDir dir;
        dir.path.setPath(ui->dirs->item(row, tableDirectoryPathIndex)->text());
        dir.separateFolders = ui->dirs->item(row, tableDirectorySeparateFoldersIndex)->checkState() == Qt::Checked;
        dir.autoReload = ui->dirs->item(row, tableDirectoryReloadIndex)->checkState() == Qt::Checked;

        const int index = dynamic_cast<QComboBox*>(ui->dirs->cellWidget(row, tableDirectoryTypeIndex))->currentIndex();
        if (dynamic_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() == 0) {
            movieDirectories.append(dir);
        } else if (index == 1) {
            tvShowDirectories.append(dir);
        } else if (index == 2) {
            concertDirectories.append(dir);
        } else if (index == 3) {
            downloadDirectories.append(dir);
        } else if (index == 4) {
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
}

void GlobalSettingsWidget::addDir(SettingsDir directory, SettingsDirType dirType)
{
    QString dir = QDir::toNativeSeparators(directory.path.path());
    if (!dir.isEmpty()) {
        bool exists = false;
        for (int i = 0, n = ui->dirs->rowCount(); i < n; ++i) {
            if (ui->dirs->item(i, tableDirectoryPathIndex)->text() == dir) {
                exists = true;
            }
        }

        if (!exists) {
            int row = ui->dirs->rowCount();
            ui->dirs->insertRow(row);
            auto* item = new QTableWidgetItem(dir);
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setToolTip(dir);
            auto* itemCheck = new QTableWidgetItem();
            itemCheck->setCheckState(directory.separateFolders ? Qt::Checked : Qt::Unchecked);

            auto* itemCheckReload = new QTableWidgetItem();
            itemCheckReload->setCheckState(directory.autoReload ? Qt::Checked : Qt::Unchecked);

            auto* box = new QComboBox();
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

            ui->dirs->setCellWidget(row, tableDirectoryTypeIndex, box);
            ui->dirs->setItem(row, tableDirectoryPathIndex, item);
            ui->dirs->setItem(row, tableDirectorySeparateFoldersIndex, itemCheck);
            ui->dirs->setItem(row, tableDirectoryReloadIndex, itemCheckReload);

            connect(box, elchOverload<int>(&QComboBox::currentIndexChanged), this, [this, box]() {
                onDirTypeChanged(box);
            });

            onDirTypeChanged(box);
        }
    }
}


void GlobalSettingsWidget::removeDir()
{
    int row = ui->dirs->currentRow();
    if (row < 0) {
        return;
    }
    ui->dirs->removeRow(row);
}

void GlobalSettingsWidget::organize()
{
    auto* organizer = new MovieFilesOrganizer(this);

    int row = ui->dirs->currentRow();
    if (dynamic_cast<QComboBox*>(ui->dirs->cellWidget(row, 0))->currentIndex() != 0
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
    case QMessageBox::Cancel:
    default: break;
    }
}

void GlobalSettingsWidget::onDirTypeChanged(QComboBox* box)
{
    if (box == nullptr) {
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


void GlobalSettingsWidget::dirListRowChanged(int currentRow)
{
    if (currentRow < 0 || currentRow >= ui->dirs->rowCount()) {
        // Somehow out of bounds?
        ui->buttonRemoveDir->setDisabled(true);
        ui->buttonMovieFilesToDirs->setDisabled(true);
        return;
    }
    ui->buttonRemoveDir->setDisabled(false);
    auto* typeWidget = ui->dirs->cellWidget(currentRow, 0);
    if (typeWidget != nullptr && dynamic_cast<QComboBox*>(typeWidget)->currentIndex() == 0
        && ui->dirs->item(currentRow, 2)->checkState() == Qt::Unchecked) {
        ui->buttonMovieFilesToDirs->setDisabled(false);
    } else {
        ui->buttonMovieFilesToDirs->setDisabled(true);
    }
}

void GlobalSettingsWidget::dirListEntryChanged(int row, int column)
{
    if (column != tableDirectoryPathIndex) {
        return;
    }
    QTableWidgetItem* dirCell = ui->dirs->item(row, tableDirectoryPathIndex);
    if (dirCell == nullptr) {
        return;
    }

    QColor defaultTextColor = QWidget::palette().color(QPalette::Text);
    QColor invalidColor(255, 0, 0);

    // if the directory is not readable, mark it red
    const QDir dir(dirCell->text());
    const QColor color = dir.isReadable() ? defaultTextColor : invalidColor;
    dirCell->setForeground(color);
}
