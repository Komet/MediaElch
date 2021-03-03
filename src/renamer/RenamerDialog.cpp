#include "renamer/RenamerDialog.h"
#include "ui_RenamerDialog.h"

#include "globals/Helper.h"
#include "globals/Manager.h"
#include "renamer/ConcertRenamer.h"
#include "renamer/EpisodeRenamer.h"
#include "renamer/MovieRenamer.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTimer>

RenamerDialog::RenamerDialog(QWidget* parent) : QDialog(parent), ui(new Ui::RenamerDialog)
{
    ui->setupUi(this);

    ui->resultsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->resultsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
#ifdef Q_OS_MAC
    QFont font = ui->resultsTable->font();
    font.setPointSize(font.pointSize() - 2);
    ui->resultsTable->setFont(font);
#endif

    connect(ui->chkDirectoryNaming, &QCheckBox::stateChanged, this, &RenamerDialog::onChkRenameDirectories);
    connect(ui->chkFileNaming, &QCheckBox::stateChanged, this, &RenamerDialog::onChkRenameFiles);
    connect(ui->chkSeasonDirectories, &QCheckBox::stateChanged, this, &RenamerDialog::onChkUseSeasonDirectories);
    connect(ui->btnDryRun, &QAbstractButton::clicked, this, &RenamerDialog::onDryRun);
    connect(ui->btnRename, &QAbstractButton::clicked, this, &RenamerDialog::onRename);

    onChkRenameDirectories();
    onChkRenameFiles();

    m_extraFiles = Settings::instance()->advanced()->subtitleFilters();
    ui->helpLabel->setText(tr("Please see %1 for help and examples on how to use the renamer.")
                               .arg("<a "
                                    "href=\"https://mediaelch.github.io/mediaelch-doc/renaming.html\">"
                                    "Renaming Files</a>"));
}

RenamerDialog::~RenamerDialog()
{
    delete ui;
}

int RenamerDialog::exec()
{
    m_filesRenamed = false;
    m_renameErrorOccured = false;

    const QString infoLabel = [&]() {
        switch (m_renameType) {
        case Renamer::RenameType::All: qWarning() << "Unknown Rename Type All"; return QString("");
        case Renamer::RenameType::Concerts: return tr("%n concerts will be renamed", "", m_concerts.count());
        case Renamer::RenameType::Movies: return tr("%n movies will be renamed", "", m_movies.count());
        case Renamer::RenameType::TvShows:
            return tr("%n TV shows and %1", "", m_shows.count())
                .arg(tr("%n episodes will be renamed", "", m_episodes.count()));
        }
        qCritical() << "[RenamerDialog] RenamerType: Missing case.";
        return QString("");
    }();
    ui->infoLabel->setText(infoLabel);

    QString fileName;
    QString fileNameMulti;
    QString directoryName;
    QString seasonName;
    bool renameFiles = false;
    bool renameFolders = false;
    bool useSeasonDirectories = false;
    Settings::instance()->renamePatterns(m_renameType, fileName, fileNameMulti, directoryName, seasonName);
    Settings::instance()->renamings(m_renameType, renameFiles, renameFolders, useSeasonDirectories);
    ui->fileNaming->setText(fileName);
    ui->fileNamingMulti->setText(fileNameMulti);
    ui->directoryNaming->setText(directoryName);
    ui->seasonNaming->setText(seasonName);
    ui->chkFileNaming->setChecked(renameFiles);
    ui->chkDirectoryNaming->setChecked(renameFolders);
    ui->chkSeasonDirectories->setChecked(useSeasonDirectories);

    ui->chkSeasonDirectories->setVisible(m_renameType == Renamer::RenameType::TvShows);
    ui->seasonNaming->setVisible(m_renameType == Renamer::RenameType::TvShows);
    ui->labelSeasonDirectory->setVisible(m_renameType == Renamer::RenameType::TvShows);

    ui->placeholders->setType(m_renameType);

    ui->results->clear();
    ui->resultsTable->setRowCount(0);
    ui->btnDryRun->setEnabled(true);
    ui->btnRename->setEnabled(true);

    ui->tabWidget->setCurrentIndex(0);

    return QDialog::exec();
}

void RenamerDialog::reject()
{
    m_movies.clear();
    m_concerts.clear();
    m_shows.clear();
    m_episodes.clear();

    Settings::instance()->setRenamePatterns(m_renameType,
        ui->fileNaming->text(),
        ui->fileNamingMulti->text(),
        ui->directoryNaming->text(),
        ui->seasonNaming->text());
    Settings::instance()->setRenamings(m_renameType,
        ui->chkFileNaming->isChecked(),
        ui->chkDirectoryNaming->isChecked(),
        ui->chkSeasonDirectories->isChecked());

    QDialog::reject();
    if (m_filesRenamed) {
        QTimer::singleShot(0, this, &RenamerDialog::onRenamed);
    }
}

void RenamerDialog::onRenamed()
{
    emit sigFilesRenamed(m_renameType);
}

bool RenamerDialog::renameErrorOccured() const
{
    return m_renameErrorOccured;
}

void RenamerDialog::setMovies(QVector<Movie*> movies)
{
    m_movies = movies;
}

void RenamerDialog::setConcerts(QVector<Concert*> concerts)
{
    m_concerts = concerts;
}

void RenamerDialog::setShows(QVector<TvShow*> shows)
{
    m_shows = shows;
}

void RenamerDialog::setEpisodes(QVector<TvShowEpisode*> episodes)
{
    m_episodes = episodes;
}

void RenamerDialog::setRenameType(Renamer::RenameType type)
{
    m_renameType = type;
}

void RenamerDialog::onChkRenameDirectories()
{
    ui->directoryNaming->setEnabled(ui->chkDirectoryNaming->isChecked());
}

void RenamerDialog::onChkRenameFiles()
{
    ui->fileNaming->setEnabled(ui->chkFileNaming->isChecked());
    ui->fileNamingMulti->setEnabled(ui->chkFileNaming->isChecked());
}

void RenamerDialog::onChkUseSeasonDirectories()
{
    ui->seasonNaming->setEnabled(ui->chkSeasonDirectories->isChecked());
}

void RenamerDialog::onRename()
{
    ui->btnRename->setEnabled(false);
    ui->btnDryRun->setEnabled(false);

    renameType(false);
}

void RenamerDialog::onDryRun()
{
    renameType(true);
}

void RenamerDialog::renameType(const bool isDryRun)
{
    ui->tabWidget->setCurrentIndex(1);
    ui->results->clear();
    ui->resultsTable->setRowCount(0);

    RenamerConfig config;
    config.dryRun = isDryRun;
    config.filePattern = ui->fileNaming->text();
    config.filePatternMulti = ui->fileNamingMulti->text();
    config.renameFiles = ui->chkFileNaming->isChecked();

    if (m_renameType == Renamer::RenameType::Movies) {
        config.directoryPattern = ui->directoryNaming->text();
        config.renameDirectories = ui->chkDirectoryNaming->isChecked();
        renameMovies(m_movies, config);

    } else if (m_renameType == Renamer::RenameType::Concerts) {
        config.directoryPattern = ui->directoryNaming->text();
        config.renameDirectories = ui->chkDirectoryNaming->isChecked();
        renameConcerts(m_concerts, config);

    } else if (m_renameType == Renamer::RenameType::TvShows) {
        config.directoryPattern = ui->seasonNaming->text();
        config.renameDirectories = ui->chkSeasonDirectories->isChecked();
        renameEpisodes(m_episodes, config);
        renameShows(m_shows, ui->directoryNaming->text(), ui->chkDirectoryNaming->isChecked(), isDryRun);
    }
    if (isDryRun) {
        m_filesRenamed = true;
    }
    ui->results->append("<span style=\"color:#01a800;\"><b>" + tr("Finished") + "</b></span>");
}

void RenamerDialog::renameMovies(QVector<Movie*> movies, const RenamerConfig& config)
{
    if ((config.renameFiles && config.filePattern.isEmpty())
        || (config.renameDirectories && config.directoryPattern.isEmpty())) {
        return;
    }

    MovieRenamer renamer(config, this);
    for (Movie* movie : movies) {
        if (movie->files().isEmpty() || (movie->files().count() > 1 && config.filePatternMulti.isEmpty())) {
            continue;
        }
        if (movie->hasChanged()) {
            ui->results->append(tr("<b>Movie</b> \"%1\" has been edited but is not saved").arg(movie->name()));
            continue;
        }

        QApplication::processEvents();

        Renamer::RenameError err = renamer.renameMovie(*movie);
        if (err != Renamer::RenameError::None) {
            m_renameErrorOccured = true;
        }
    }
}

void RenamerDialog::renameEpisodes(QVector<TvShowEpisode*> episodes, const RenamerConfig& config)
{
    if (config.renameFiles && config.filePattern.isEmpty()) {
        return;
    }

    EpisodeRenamer renamer(config, this);
    QVector<TvShowEpisode*> episodesRenamed;

    for (TvShowEpisode* episode : episodes) {
        if (episode->files().isEmpty() || (episode->files().count() > 1 && config.filePatternMulti.isEmpty())
            || episodesRenamed.contains(episode)) {
            continue;
        }
        if (episode->hasChanged()) {
            ui->results->append(tr("<b>Episode</b> \"%1\" has been edited but is not saved").arg(episode->title()));
            continue;
        }

        QApplication::processEvents();

        Renamer::RenameError err = renamer.renameEpisode(*episode, episodesRenamed);
        if (err != Renamer::RenameError::None) {
            m_renameErrorOccured = true;
        }
    }
}

void RenamerDialog::renameShows(QVector<TvShow*> shows,
    const QString& directoryPattern,
    const bool& renameDirectories,
    const bool& dryRun)
{
    if ((renameDirectories && directoryPattern.isEmpty()) || !renameDirectories) {
        return;
    }

    for (TvShow* show : shows) {
        if (show->hasChanged()) {
            ui->results->append(tr("<b>TV Show</b> \"%1\" has been edited but is not saved").arg(show->title()));
            continue;
        }

        QDir dir(show->dir().toString());
        QString newFolderName = directoryPattern;
        Renamer::replace(newFolderName, "title", show->title());
        Renamer::replace(newFolderName, "showTitle", show->title());
        Renamer::replace(newFolderName, "year", show->firstAired().toString("yyyy"));
        helper::sanitizeFolderName(newFolderName);
        if (newFolderName != dir.dirName()) {
            const int row = addResultToTable(dir.dirName(), newFolderName, Renamer::RenameOperation::Rename);
            QDir parentDir(dir.path());
            parentDir.cdUp();
            if (dryRun) {
                continue;
            }
            if (!Renamer::rename(dir, parentDir.absolutePath() + "/" + newFolderName)) {
                setResultStatus(row, Renamer::RenameResult::Failed);
                m_renameErrorOccured = true;
                continue;
            }
            const QString newShowDir = parentDir.absolutePath() + "/" + newFolderName;
            const QString oldShowDir = show->dir().toString();
            show->setDir(newShowDir);
            Manager::instance()->database()->update(show);
            for (TvShowEpisode* episode : show->episodes()) {
                QStringList files;
                for (const mediaelch::FilePath& file : episode->files()) {
                    files << newShowDir + file.toString().mid(oldShowDir.length());
                }
                episode->setFiles(files);
                Manager::instance()->database()->update(episode);
            }
        }
    }
}

void RenamerDialog::renameConcerts(QVector<Concert*> concerts, const RenamerConfig& config)
{
    if ((config.renameFiles && config.filePattern.isEmpty())
        || (config.renameDirectories && config.directoryPattern.isEmpty())) {
        return;
    }

    ConcertRenamer renamer(config, this);

    for (Concert* concert : concerts) {
        if (concert->files().isEmpty() || (concert->files().count() > 1 && config.filePatternMulti.isEmpty())) {
            continue;
        }
        if (concert->hasChanged()) {
            ui->results->append(tr("<b>Concert</b> \"%1\" has been edited but is not saved").arg(concert->title()));
            continue;
        }

        QApplication::processEvents();

        Renamer::RenameError err = renamer.renameConcert(*concert);
        if (err != Renamer::RenameError::None) {
            m_renameErrorOccured = true;
        }
    }
}

int RenamerDialog::addResultToTable(const QString& oldFileName,
    const QString& newFileName,
    Renamer::RenameOperation operation)
{
    const QString opString = [operation]() -> QString {
        switch (operation) {
        case Renamer::RenameOperation::CreateDir: return tr("Create dir");
        case Renamer::RenameOperation::Move: return tr("Move");
        case Renamer::RenameOperation::Rename: return tr("Rename");
        }
        qCritical() << "[RenamerDialog] RenameOperation: Missing case.";
        return QString("");
    }();

    QFont font = ui->resultsTable->font();
    font.setBold(true);

    int row = ui->resultsTable->rowCount();
    ui->resultsTable->insertRow(row);
    ui->resultsTable->setItem(row, 0, new QTableWidgetItem(opString));
    ui->resultsTable->setItem(row, 1, new QTableWidgetItem(oldFileName));
    ui->resultsTable->setItem(row, 2, new QTableWidgetItem(newFileName));
    ui->resultsTable->item(row, 0)->setFont(font);

    return row;
}

void RenamerDialog::setResultStatus(int row, Renamer::RenameResult result)
{
    for (int col = 0, n = ui->resultsTable->columnCount(); col < n; ++col) {
        if (result == Renamer::RenameResult::Failed) {
            ui->resultsTable->item(row, col)->setBackground(QColor(242, 222, 222));
            ui->resultsTable->item(row, col)->setForeground(QColor(0, 0, 0));
        }
    }
}

void RenamerDialog::appendResultText(QString str)
{
    ui->results->append(str);
}
