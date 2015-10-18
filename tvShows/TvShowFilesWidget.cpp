#include "TvShowFilesWidget.h"
#include "ui_TvShowFilesWidget.h"

#include <QCheckBox>
#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>
#include "globals/Manager.h"
#include "data/TvShowModelItem.h"
#include "smallWidgets/LoadingStreamDetails.h"
#include "tvShows/TvShowUpdater.h"

TvShowFilesWidget *TvShowFilesWidget::m_instance;

/**
 * @brief TvShowFilesWidget::TvShowFilesWidget
 * @param parent
 */
TvShowFilesWidget::TvShowFilesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TvShowFilesWidget)
{
    m_instance = this;
    ui->setupUi(this);

    ui->statusLabel->setText(tr("%n tv shows", "", 0) + ", " + tr("%n episodes", "", 0));

#ifdef Q_OS_MAC
    QFont font = ui->files->font();
    font.setPointSize(font.pointSize()-2);
    ui->files->setFont(font);
#endif
#ifdef Q_OS_WIN32
    ui->verticalLayout->setContentsMargins(0, 0, 0, 1);
#endif

    m_lastTvShow = 0;
    m_lastEpisode = 0;
    m_lastSeason = -1;
    m_tvShowDelegate = new TvShowDelegate(this);
    m_tvShowProxyModel = Manager::instance()->tvShowProxyModel();
    m_tvShowProxyModel->setSourceModel(Manager::instance()->tvShowModel());
    m_tvShowProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_tvShowProxyModel->setDynamicSortFilter(true);
    ui->files->setModel(m_tvShowProxyModel);
    ui->files->setItemDelegate(m_tvShowDelegate);
    ui->files->sortByColumn(0);
    ui->files->setAttribute(Qt::WA_MacShowFocusRect, false);

    QAction *actionScanForEpisodes = new QAction(tr("Search for new episodes"), this);
    QAction *actionMarkAsWatched = new QAction(tr("Mark as watched"), this);
    QAction *actionMarkAsUnwatched = new QAction(tr("Mark as unwatched"), this);
    QAction *actionLoadStreamDetails = new QAction(tr("Load Stream Details"), this);
    QAction *actionMarkForSync = new QAction(tr("Add to Synchronization Queue"), this);
    QAction *actionUnmarkForSync = new QAction(tr("Remove from Synchronization Queue"), this);
    QAction *actionOpenFolder = new QAction(tr("Open TV Show Folder"), this);
    m_actionShowMissingEpisodes = new QAction(tr("Show missing episodes"), this);
    m_actionShowMissingEpisodes->setCheckable(true);
    m_actionHideSpecialsInMissingEpisodes = new QAction(tr("Hide specials in missing episodes"), this);
    m_actionHideSpecialsInMissingEpisodes->setCheckable(true);
    m_contextMenu = new QMenu(ui->files);
    m_contextMenu->addAction(actionScanForEpisodes);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(actionMarkAsWatched);
    m_contextMenu->addAction(actionMarkAsUnwatched);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(actionLoadStreamDetails);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(actionMarkForSync);
    m_contextMenu->addAction(actionUnmarkForSync);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(actionOpenFolder);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_actionShowMissingEpisodes);
    m_contextMenu->addAction(m_actionHideSpecialsInMissingEpisodes);

    connect(actionScanForEpisodes, SIGNAL(triggered()), this, SLOT(scanForEpisodes()));
    connect(actionMarkAsWatched, SIGNAL(triggered()), this, SLOT(markAsWatched()));
    connect(actionMarkAsUnwatched, SIGNAL(triggered()), this, SLOT(markAsUnwatched()));
    connect(actionLoadStreamDetails, SIGNAL(triggered()), this, SLOT(loadStreamDetails()));
    connect(actionMarkForSync, SIGNAL(triggered()), this, SLOT(markForSync()));
    connect(actionUnmarkForSync, SIGNAL(triggered()), this, SLOT(unmarkForSync()));
    connect(actionOpenFolder, SIGNAL(triggered()), this, SLOT(openFolder()));
    connect(m_actionShowMissingEpisodes, SIGNAL(triggered()), this, SLOT(showMissingEpisodes()));
    connect(m_actionHideSpecialsInMissingEpisodes, SIGNAL(triggered()), this, SLOT(hideSpecialsInMissingEpisodes()));
    connect(ui->files, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(ui->files, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)), Qt::QueuedConnection);
    connect(ui->files->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemActivated(QModelIndex,QModelIndex)), Qt::QueuedConnection);
    connect(ui->files, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(playEpisode(QModelIndex)));
    Manager::instance()->setTvShowFilesWidget(this);

    connect(m_tvShowProxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(onViewUpdated()));
    connect(m_tvShowProxyModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(onViewUpdated()));
    connect(Manager::instance()->tvShowFileSearcher(), SIGNAL(tvShowsLoaded(int)), this, SLOT(onViewUpdated()));
}

/**
 * @brief TvShowFilesWidget::~TvShowFilesWidget
 */
TvShowFilesWidget::~TvShowFilesWidget()
{
    delete ui;
}

/**
 * @brief Returns the current instance of the widget
 * @return Current instance of TvShowFilesWidget
 */
TvShowFilesWidget *TvShowFilesWidget::instance()
{
    return m_instance;
}

/**
 * @brief Shows the contextmenu
 * @param point
 */
void TvShowFilesWidget::showContextMenu(QPoint point)
{
    if (ui->files->selectionModel()->selectedRows(0).count() != 1) {
        m_actionShowMissingEpisodes->setEnabled(false);
    } else {
        QModelIndex index = m_tvShowProxyModel->mapToSource(ui->files->selectionModel()->selectedRows(0).at(0));
        TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
        if (item->type() != TypeTvShow) {
            m_actionShowMissingEpisodes->setEnabled(false);
            m_actionHideSpecialsInMissingEpisodes->setEnabled(false);
        } else {
            m_actionShowMissingEpisodes->setEnabled(true);
            m_actionShowMissingEpisodes->setChecked(item->tvShow()->showMissingEpisodes());
            m_actionHideSpecialsInMissingEpisodes->setEnabled(item->tvShow()->showMissingEpisodes());
            m_actionHideSpecialsInMissingEpisodes->setChecked(item->tvShow()->hideSpecialsInMissingEpisodes());
        }
    }

    QPoint globalPoint = ui->files->mapToGlobal(point);
    m_contextMenu->exec(globalPoint);
}

void TvShowFilesWidget::scanForEpisodes()
{
    m_contextMenu->close();
    QModelIndex sourceIndex = m_tvShowProxyModel->mapToSource(ui->files->currentIndex());
    if (!sourceIndex.isValid())
        return;

    TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(sourceIndex);
    if (item->type() == TypeTvShow)
        Manager::instance()->fileScannerDialog()->setScanDir(item->tvShow()->dir());
    else if (item->type() == TypeSeason)
        Manager::instance()->fileScannerDialog()->setScanDir(item->tvShow()->dir());
    else if (item->type() == TypeEpisode)
        Manager::instance()->fileScannerDialog()->setScanDir(item->tvShowEpisode()->tvShow()->dir());
    else
        return;
    Manager::instance()->fileScannerDialog()->setReloadType(FileScannerDialog::TypeEpisodes);
    Manager::instance()->fileScannerDialog()->exec();
}

void TvShowFilesWidget::markAsWatched()
{
    m_contextMenu->close();
    foreach (const QModelIndex &mIndex, ui->files->selectionModel()->selectedRows(0)) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
        if (item->type() == TypeTvShow || item->type() == TypeSeason) {
            foreach (TvShowEpisode *episode, item->tvShow()->episodes()) {
                if (episode->isDummy())
                    continue;
                if (item->type() == TypeSeason && episode->season() != item->season().toInt())
                    continue;
                if (episode->playCount() < 1)
                    episode->setPlayCount(1);
                if (!episode->lastPlayed().isValid())
                    episode->setLastPlayed(QDateTime::currentDateTime());
            }
        } else if (item->type() == TypeEpisode && !item->tvShowEpisode()->isDummy()) {
            if (item->tvShowEpisode()->playCount() < 1)
                item->tvShowEpisode()->setPlayCount(1);
            if (!item->tvShowEpisode()->lastPlayed().isValid())
                item->tvShowEpisode()->setLastPlayed(QDateTime::currentDateTime());
        }
    }

    QModelIndex sourceIndex = m_tvShowProxyModel->mapToSource(ui->files->currentIndex());
    if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeTvShow) {
        emit sigTvShowSelected(Manager::instance()->tvShowModel()->getItem(sourceIndex)->tvShow());
    } else if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeEpisode) {
        emit sigEpisodeSelected(Manager::instance()->tvShowModel()->getItem(sourceIndex)->tvShowEpisode());
    }
}

void TvShowFilesWidget::markAsUnwatched()
{
    m_contextMenu->close();
    foreach (const QModelIndex &mIndex, ui->files->selectionModel()->selectedRows(0)) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
        if (item->type() == TypeTvShow || item->type() == TypeSeason) {
            foreach (TvShowEpisode *episode, item->tvShow()->episodes()) {
                if (episode->isDummy())
                    continue;
                if (item->type() == TypeSeason && episode->season() != item->season().toInt())
                    continue;
                if (episode->playCount() != 0)
                    episode->setPlayCount(0);
            }
        } else if (item->type() == TypeEpisode && !item->tvShowEpisode()->isDummy()) {
            if (item->tvShowEpisode()->playCount() != 0)
                item->tvShowEpisode()->setPlayCount(0);
        }
    }

    QModelIndex sourceIndex = m_tvShowProxyModel->mapToSource(ui->files->currentIndex());
    if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeTvShow) {
        emit sigTvShowSelected(Manager::instance()->tvShowModel()->getItem(sourceIndex)->tvShow());
    } else if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeEpisode) {
        emit sigEpisodeSelected(Manager::instance()->tvShowModel()->getItem(sourceIndex)->tvShowEpisode());
    }
}

void TvShowFilesWidget::loadStreamDetails()
{
    m_contextMenu->close();
    QList<TvShowEpisode*> episodes;

    foreach (const QModelIndex &mIndex, ui->files->selectionModel()->selectedRows(0)) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
        if (item->type() == TypeTvShow || item->type() == TypeSeason) {
            foreach (TvShowEpisode *episode, item->tvShow()->episodes()) {
                if (episode->isDummy())
                    continue;
                if (item->type() == TypeSeason && episode->season() != item->season().toInt())
                    continue;
                episodes.append(episode);
            }
        } else if (item->type() == TypeEpisode && !item->tvShowEpisode()->isDummy()) {
            episodes.append(item->tvShowEpisode());
        }
    }

    if (episodes.count() == 1) {
        episodes.at(0)->loadStreamDetailsFromFile();
        episodes.at(0)->setChanged(true);
    } else {
        LoadingStreamDetails *loader = new LoadingStreamDetails(this);
        loader->loadTvShowEpisodes(episodes);
        delete loader;
    }
    QModelIndex sourceIndex = m_tvShowProxyModel->mapToSource(ui->files->currentIndex());
    if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeEpisode) {
        emit sigEpisodeSelected(Manager::instance()->tvShowModel()->getItem(sourceIndex)->tvShowEpisode());
    }
}

void TvShowFilesWidget::markForSync()
{
    m_contextMenu->close();
    foreach (const QModelIndex &mIndex, ui->files->selectionModel()->selectedRows(0)) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
        if (item->type() == TypeTvShow) {
            item->tvShow()->setSyncNeeded(true);
        } else if (item->type() == TypeSeason) {
            foreach (TvShowEpisode *episode, item->tvShow()->episodes()) {
                if (episode->isDummy())
                    continue;
                if (episode->season() != item->season().toInt())
                    continue;
                episode->setSyncNeeded(true);
            }

            for (int i=0, n=item->childCount() ; i<n ; ++i)
                ui->files->update(m_tvShowProxyModel->mapFromSource(Manager::instance()->tvShowModel()->index(i, 0, index)));
        } else if (item->type() == TypeEpisode && !item->tvShowEpisode()->isDummy()) {
            item->tvShowEpisode()->setSyncNeeded(true);
        }
        ui->files->update(mIndex);
    }
}

void TvShowFilesWidget::unmarkForSync()
{
    m_contextMenu->close();
    foreach (const QModelIndex &mIndex, ui->files->selectionModel()->selectedRows(0)) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
        if (item->type() == TypeTvShow) {
            item->tvShow()->setSyncNeeded(false);
        } else if (item->type() == TypeSeason) {
            foreach (TvShowEpisode *episode, item->tvShow()->episodes()) {
                if (episode->isDummy())
                    continue;
                if (episode->season() != item->season().toInt())
                    continue;
                episode->setSyncNeeded(false);
            }
            for (int i=0, n=item->childCount() ; i<n ; ++i)
                ui->files->update(m_tvShowProxyModel->mapFromSource(Manager::instance()->tvShowModel()->index(i, 0, index)));
        } else if (item->type() == TypeEpisode && !item->tvShowEpisode()->isDummy()) {
            item->tvShowEpisode()->setSyncNeeded(false);
        }
        ui->files->update(mIndex);
    }
}

void TvShowFilesWidget::openFolder()
{
    m_contextMenu->close();
    if (!ui->files->currentIndex().isValid())
        return;
    QModelIndex index = m_tvShowProxyModel->mapToSource(ui->files->currentIndex());
    TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
    if (!item)
        return;
    QString dir;
    if (item->type() == TypeTvShow) {
        dir = item->tvShow()->dir();
    } else if (item->type() == TypeSeason) {
        dir = item->tvShow()->dir();
    } else if (item->type() == TypeEpisode && !item->tvShowEpisode()->files().isEmpty() && !item->tvShowEpisode()->isDummy()) {
        QFileInfo fi(item->tvShowEpisode()->files().at(0));
        dir = fi.absolutePath();
    }

    if (dir.isEmpty())
        return;

    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void TvShowFilesWidget::showMissingEpisodes()
{
    m_contextMenu->close();
    foreach (const QModelIndex &mIndex, ui->files->selectionModel()->selectedRows(0)) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
        if (item->type() == TypeTvShow) {
            item->tvShow()->setShowMissingEpisodes(!item->tvShow()->showMissingEpisodes());
            m_actionShowMissingEpisodes->setChecked(item->tvShow()->showMissingEpisodes());
            if (item->tvShow()->showMissingEpisodes()) {
                item->tvShow()->fillMissingEpisodes();
                if (item->tvShow()->id().isEmpty() && item->tvShow()->tvdbId().isEmpty() &&
                        item->tvShow()->episodeGuideUrl().isEmpty()) {
                    if (Settings::instance()->showMissingEpisodesHint()) {
                        QMessageBox msgBox;
                        msgBox.setIcon(QMessageBox::Information);
                        msgBox.setWindowTitle(tr("Show update needed"));
                        msgBox.setText(tr("You need to update the show once to show missing episodes.\n"
                                          "Afterwards MediaElch will check automatically for new episodes on startup."));
                        msgBox.setStandardButtons(QMessageBox::Ok);
                        QCheckBox dontShowAgain(QObject::tr("Don't show this hint again"), &msgBox);
                        dontShowAgain.blockSignals(true);
                        msgBox.addButton(&dontShowAgain, QMessageBox::ActionRole);
                        msgBox.exec();
                        if (dontShowAgain.checkState() == Qt::Checked)
                            Settings::instance()->setShowMissingEpisodesHint(false);
                    }
                } else {
                    TvShowUpdater::instance()->updateShow(item->tvShow());
                }

            } else {
                item->tvShow()->clearMissingEpisodes();
            }
        }
    }
}

void TvShowFilesWidget::hideSpecialsInMissingEpisodes()
{
    m_contextMenu->close();
    foreach (const QModelIndex &mIndex, ui->files->selectionModel()->selectedRows(0)) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
        if (item->type() == TypeTvShow) {
            item->tvShow()->setHideSpecialsInMissingEpisodes(!item->tvShow()->hideSpecialsInMissingEpisodes());
            m_actionHideSpecialsInMissingEpisodes->setChecked(item->tvShow()->hideSpecialsInMissingEpisodes());
            item->tvShow()->clearMissingEpisodes();
            item->tvShow()->fillMissingEpisodes();
        }
    }
}

/**
 * @brief Sets the filters
 * @param filters List of filters
 * @param text Filter text
 * @todo: respect filters and not only filter text
 */
void TvShowFilesWidget::setFilter(QList<Filter *> filters, QString text)
{
    if (!filters.isEmpty())
        m_tvShowProxyModel->setFilterWildcard("*" + filters.first()->shortText() + "*");
    else
        m_tvShowProxyModel->setFilterWildcard("*" + text + "*");
    m_tvShowProxyModel->setFilter(filters, text);
}

/**
 * @brief Renews the model (necessary after searching for tv shows)
 */
void TvShowFilesWidget::renewModel(bool force)
{
    qDebug() << "Entered";
    if (force) {
        disconnect(ui->files->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemActivated(QModelIndex,QModelIndex)));
        m_tvShowProxyModel->setSourceModel(0);
        m_tvShowProxyModel->setSourceModel(Manager::instance()->tvShowModel());
        ui->files->setModel(0);
        ui->files->setModel(m_tvShowProxyModel);
        connect(ui->files->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemActivated(QModelIndex,QModelIndex)), Qt::QueuedConnection);
    }
    onViewUpdated();
}

/**
 * @brief Collapses or expands items
 * @param index
 */
void TvShowFilesWidget::onItemClicked(QModelIndex index)
{
    if (ui->files->isExpanded(index))
        ui->files->collapse(index);
    else
        ui->files->expand(index);
}

/**
 * @brief Emits sigTvShowSelected or sigEpisodeSelected
 * @param index
 * @param previous
 */
void TvShowFilesWidget::onItemActivated(QModelIndex index, QModelIndex previous)
{
    qDebug() << "Entered";
    Q_UNUSED(previous);

    if (!index.isValid()) {
        qDebug() << "Invalid index";
        emit sigNothingSelected();
        return;
    }

    m_lastEpisode = 0;
    m_lastTvShow = 0;
    m_lastSeason = -1;
    QModelIndex sourceIndex = m_tvShowProxyModel->mapToSource(index);
    if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeTvShow) {
        m_lastTvShow = Manager::instance()->tvShowModel()->getItem(sourceIndex)->tvShow();
        emit sigTvShowSelected(m_lastTvShow);
    } else if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeEpisode) {
        m_lastEpisode = Manager::instance()->tvShowModel()->getItem(sourceIndex)->tvShowEpisode();
        emit sigEpisodeSelected(m_lastEpisode);
    } else if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeSeason) {
        m_lastTvShow = Manager::instance()->tvShowModel()->getItem(sourceIndex)->tvShow();
        m_lastSeason = Manager::instance()->tvShowModel()->getItem(sourceIndex)->season().toInt();
        emit sigSeasonSelected(m_lastTvShow, m_lastSeason);
    }
}

void TvShowFilesWidget::emitLastSelection()
{
    if (m_lastTvShow && m_lastSeason != -1)
        emit sigSeasonSelected(m_lastTvShow, m_lastSeason);
    else if (m_lastTvShow)
        emit sigTvShowSelected(m_lastTvShow);
    else if (m_lastEpisode)
        emit sigEpisodeSelected(m_lastEpisode);
}

QList<TvShowEpisode*> TvShowFilesWidget::selectedEpisodes()
{
    QList<TvShowEpisode*> episodes;
    foreach (const QModelIndex &mIndex, ui->files->selectionModel()->selectedRows(0)) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
        if (item->type() == TypeEpisode && !item->tvShowEpisode()->isDummy()) {
            episodes.append(item->tvShowEpisode());
        } else if (item->type() == TypeSeason) {
            foreach (TvShowEpisode *episode, item->tvShow()->episodes()) {
                if (episode->isDummy())
                    continue;
                if (!episodes.contains(episode) && episode->season() == item->season().toInt())
                    episodes.append(episode);
            }
        } else if (item->type() == TypeTvShow) {
            foreach (TvShowEpisode *episode, item->tvShow()->episodes()) {
                if (episode->isDummy())
                    continue;
                if (!episodes.contains(episode))
                    episodes.append(episode);
            }
        }
    }
    return episodes;
}

QList<TvShow*> TvShowFilesWidget::selectedShows()
{
    QList<TvShow*> shows;
    foreach (const QModelIndex &mIndex, ui->files->selectionModel()->selectedRows(0)) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
        if (item->type() == TypeTvShow)
            shows.append(item->tvShow());
    }
    return shows;
}

void TvShowFilesWidget::onViewUpdated()
{
    if (m_tvShowProxyModel->filterRegExp().pattern().isEmpty() || m_tvShowProxyModel->filterRegExp().pattern() == "**") {
        int episodeCount = 0;
        foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows())
            episodeCount += show->episodeCount();
        ui->statusLabel->setText(tr("%n tv shows", "", m_tvShowProxyModel->rowCount()) + ", " + tr("%n episodes", "", episodeCount));
    } else {
        ui->statusLabel->setText(tr("%1 of %n tv shows", "", Manager::instance()->tvShowModel()->tvShows().count()).arg(m_tvShowProxyModel->rowCount()));
    }
}

void TvShowFilesWidget::playEpisode(QModelIndex idx)
{
    if (!idx.isValid())
        return;

    if (m_tvShowProxyModel->data(idx, TvShowRoles::Type).toInt() != TypeEpisode)
        return;

    QString fileName = m_tvShowProxyModel->data(idx, TvShowRoles::FilePath).toString();
    if (fileName.isEmpty())
        return;
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}
