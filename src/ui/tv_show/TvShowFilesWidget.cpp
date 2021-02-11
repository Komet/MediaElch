#include "TvShowFilesWidget.h"
#include "ui_TvShowFilesWidget.h"

#include <QCheckBox>
#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>

#include "globals/Globals.h"
#include "globals/Manager.h"
#include "tv_shows/TvShowUpdater.h"
#include "tv_shows/model/EpisodeModelItem.h"
#include "tv_shows/model/SeasonModelItem.h"
#include "tv_shows/model/TvShowModelItem.h"
#include "ui/small_widgets/LoadingStreamDetails.h"
#include "ui/tv_show/TvShowMultiScrapeDialog.h"

TvShowFilesWidget* TvShowFilesWidget::m_instance;

TvShowFilesWidget::TvShowFilesWidget(QWidget* parent) :
    QWidget(parent), ui(new Ui::TvShowFilesWidget), m_tvShowProxyModel{new TvShowProxyModel(this)}
{
    m_instance = this;
    ui->setupUi(this);

    ui->statusLabel->setText(tr("%n TV shows", "", 0) + ", " + tr("%n episodes", "", 0));

#ifdef Q_OS_WIN
    ui->verticalLayout->setContentsMargins(0, 0, 0, 1);
#endif

    m_tvShowProxyModel->setSourceModel(Manager::instance()->tvShowModel());
    m_tvShowProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_tvShowProxyModel->setDynamicSortFilter(true);
    m_tvShowProxyModel->sort(0, Qt::AscendingOrder);

    ui->files->setModel(m_tvShowProxyModel);
    ui->files->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->files->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->files->setIconSize(QSize(12, 12));

#ifdef Q_OS_WIN
    ui->files->setAnimated(false);
#endif

    setupContextMenu();

    // clang-format off

    /// \todo Maybe select the first row after other rows were removed.
    // connect(m_tvShowProxyModel, &TvShowProxyModel::rowsAboutToBeRemoved, this,
    //  [](const QModelIndex &parent, int first, int last){ /*...*/  });

    connect(ui->files,            &TvShowTreeView::customContextMenuRequested, this, &TvShowFilesWidget::showContextMenu);
    connect(ui->files->selectionModel(), &QItemSelectionModel::currentChanged, this, &TvShowFilesWidget::onItemSelected, Qt::QueuedConnection);
    connect(ui->files,                   &TvShowTreeView::doubleClicked,       this, &TvShowFilesWidget::playEpisode);

    Manager::instance()->setTvShowFilesWidget(this);

    connect(m_tvShowProxyModel, &QAbstractItemModel::rowsInserted, this, &TvShowFilesWidget::updateStatusLabel);
    connect(m_tvShowProxyModel, &QAbstractItemModel::rowsRemoved,  this, &TvShowFilesWidget::updateStatusLabel);
    // clang-format on

    // FIXME:
    // For some reason, the proxy model emits "rowsRemoved" before "endRemoveRows()" is called in the source model.
    connect(Manager::instance()->tvShowFileSearcher(),
        &TvShowFileSearcher::tvShowsLoaded,
        this,
        &TvShowFilesWidget::updateStatusLabel);
}

TvShowFilesWidget::~TvShowFilesWidget()
{
    delete ui;
}

/// \brief Returns the current instance of the widget
/// \return Current instance of TvShowFilesWidget
TvShowFilesWidget& TvShowFilesWidget::instance()
{
    return *m_instance;
}

/// \brief Show the context menu for a selected show, season or episode
void TvShowFilesWidget::showContextMenu(QPoint point)
{
    const QModelIndexList rows = ui->files->selectionModel()->selectedRows(0);

    if (rows.count() != 1) {
        m_actionShowMissingEpisodes->setEnabled(false);

    } else {
        const QModelIndex index = m_tvShowProxyModel->mapToSource(rows.at(0));
        TvShowBaseModelItem& item = Manager::instance()->tvShowModel()->getItem(index);

        if (item.type() == TvShowType::TvShow) {
            m_actionShowMissingEpisodes->setEnabled(true);
            m_actionShowMissingEpisodes->setChecked(item.tvShow()->showMissingEpisodes());
            m_actionHideSpecialsInMissingEpisodes->setEnabled(item.tvShow()->showMissingEpisodes());
            m_actionHideSpecialsInMissingEpisodes->setChecked(item.tvShow()->hideSpecialsInMissingEpisodes());

        } else {
            // episode or season
            m_actionShowMissingEpisodes->setEnabled(false);
            m_actionHideSpecialsInMissingEpisodes->setEnabled(false);
        }
    }

    QPoint globalPoint = ui->files->mapToGlobal(point);
    m_contextMenu->exec(globalPoint);
}

void TvShowFilesWidget::scanForEpisodes()
{
    m_contextMenu->close();

    QModelIndex sourceIndex = m_tvShowProxyModel->mapToSource(ui->files->currentIndex());
    if (!sourceIndex.isValid()) {
        return;
    }

    auto& item = Manager::instance()->tvShowModel()->getItem(sourceIndex);
    if (item.tvShow() == nullptr) {
        // item type may be "TvShowType::None"
        return;
    }

    Manager::instance()->fileScannerDialog()->setScanDir(item.tvShow()->dir());
    Manager::instance()->fileScannerDialog()->setReloadType(FileScannerDialog::ReloadType::Episodes);

    auto* selectionModel = ui->files->selectionModel();
    selectionModel->blockSignals(true);
    Manager::instance()->fileScannerDialog()->exec();
    selectionModel->blockSignals(false);
    selectionModel->clearSelection();
    selectionModel->clearCurrentIndex();
    emit sigNothingSelected();

    QApplication::processEvents();

    if (m_lastItem == nullptr || m_lastItem->tvShow() == nullptr) {
        return;
    }

    // select the show again after re-scanning

    const QString dir = m_lastItem->tvShow()->dir().toString();
    const int rowCount = ui->files->model()->rowCount();

    for (int row = 0; row < rowCount; ++row) {
        QModelIndex proxyIndex = ui->files->model()->index(row, 0);

        if (ui->files->model()->data(proxyIndex, TvShowRoles::FilePath).toString() == dir) {
            selectionModel->setCurrentIndex(proxyIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            break;
        }
    }
}

void TvShowFilesWidget::markAsWatched()
{
    m_contextMenu->close();

    forEachSelectedItem([&](TvShowBaseModelItem& item) {
        switch (item.type()) {
        case TvShowType::None: break;
        case TvShowType::TvShow: {
            for (TvShowEpisode* episode : item.tvShow()->episodes()) {
                if (episode->isDummy()) {
                    continue;
                }
                if (episode->playCount() < 1) {
                    episode->setPlayCount(1);
                }
                if (!episode->lastPlayed().isValid()) {
                    episode->setLastPlayed(QDateTime::currentDateTime());
                }
            }
            break;
        }
        case TvShowType::Season: {
            auto* seasonModel = dynamic_cast<SeasonModelItem*>(&item);
            if (seasonModel == nullptr) {
                break;
            }
            for (TvShowEpisode* episode : seasonModel->tvShow()->episodes()) {
                if (episode->isDummy()) {
                    continue;
                }
                if (episode->seasonNumber() != seasonModel->seasonNumber()) {
                    continue;
                }
                if (episode->playCount() < 1) {
                    episode->setPlayCount(1);
                }
                if (!episode->lastPlayed().isValid()) {
                    episode->setLastPlayed(QDateTime::currentDateTime());
                }
            }
            break;
        }
        case TvShowType::Episode: {
            auto* episode = dynamic_cast<EpisodeModelItem*>(&item)->tvShowEpisode();
            if (episode == nullptr || episode->isDummy()) {
                break;
            }
            if (episode->playCount() < 1) {
                episode->setPlayCount(1);
            }
            if (!episode->lastPlayed().isValid()) {
                episode->setLastPlayed(QDateTime::currentDateTime());
            }
        } break;
        }
    });

    emitSelected(ui->files->currentIndex());
}

void TvShowFilesWidget::markAsUnwatched()
{
    m_contextMenu->close();

    forEachSelectedItem([&](TvShowBaseModelItem& item) {
        switch (item.type()) {
        case TvShowType::None: break;
        case TvShowType::TvShow: {
            auto* showModel = dynamic_cast<TvShowModelItem*>(&item);
            if (showModel == nullptr) {
                break;
            }
            for (TvShowEpisode* episode : showModel->tvShow()->episodes()) {
                if (!episode->isDummy() && episode->playCount() != 0) {
                    episode->setPlayCount(0);
                }
            }
            break;
        }
        case TvShowType::Season: {
            auto* seasonModel = dynamic_cast<SeasonModelItem*>(&item);
            if (seasonModel == nullptr) {
                break;
            }
            for (TvShowEpisode* episode : seasonModel->tvShow()->episodes()) {
                if (episode->isDummy()) {
                    continue;
                }
                if (episode->seasonNumber() != seasonModel->seasonNumber()) {
                    continue;
                }
                if (episode->playCount() != 0) {
                    episode->setPlayCount(0);
                }
            }
            break;
        }
        case TvShowType::Episode: {
            auto* episode = dynamic_cast<EpisodeModelItem*>(&item)->tvShowEpisode();
            if (episode != nullptr && !episode->isDummy() && episode->playCount() != 0) {
                episode->setPlayCount(0);
            }
            break;
        }
        }
    });

    emitSelected(ui->files->currentIndex());
}

void TvShowFilesWidget::loadStreamDetails()
{
    m_contextMenu->close();

    QVector<TvShowEpisode*> episodes;

    forEachSelectedItem([&](TvShowBaseModelItem& item) {
        if (item.type() == TvShowType::TvShow || item.type() == TvShowType::Season) {
            for (TvShowEpisode* episode : item.tvShow()->episodes()) {
                if (episode->isDummy()) {
                    continue;
                }
                if (item.type() == TvShowType::Season) {
                    auto* season = dynamic_cast<SeasonModelItem*>(&item);
                    if (season == nullptr || episode->seasonNumber() != season->seasonNumber()) {
                        continue;
                    }
                }
                episodes.append(episode);
            }

        } else if (item.type() == TvShowType::Episode) {
            auto* episode = dynamic_cast<EpisodeModelItem*>(&item);
            if (episode != nullptr && !episode->tvShowEpisode()->isDummy()) {
                episodes.append(episode->tvShowEpisode());
            }
        }
    });

    if (episodes.count() == 1) {
        episodes.at(0)->loadStreamDetailsFromFile();
        episodes.at(0)->setChanged(true);

    } else {
        LoadingStreamDetails loader;
        loader.loadTvShowEpisodes(episodes);
    }

    emitSelected(ui->files->currentIndex());
}

void TvShowFilesWidget::markForSyncBool(bool markForSync)
{
    m_contextMenu->close();

    for (const QModelIndex& mIndex : ui->files->selectionModel()->selectedRows(0)) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowBaseModelItem& item = Manager::instance()->tvShowModel()->getItem(index);

        if (item.type() == TvShowType::TvShow) {
            item.tvShow()->setSyncNeeded(markForSync);

        } else if (item.type() == TvShowType::Season) {
            auto* seasonModel = dynamic_cast<SeasonModelItem*>(&item);
            if (seasonModel == nullptr) {
                continue;
            }
            for (TvShowEpisode* episode : seasonModel->tvShow()->episodes()) {
                if (episode->isDummy()) {
                    continue;
                }
                if (episode->seasonNumber() != seasonModel->seasonNumber()) {
                    continue;
                }
                episode->setSyncNeeded(markForSync);
            }

            for (int i = 0, n = item.childCount(); i < n; ++i) {
                ui->files->update(
                    m_tvShowProxyModel->mapFromSource(Manager::instance()->tvShowModel()->index(i, 0, index)));
            }

        } else if (item.type() == TvShowType::Episode) {
            auto* episode = dynamic_cast<EpisodeModelItem*>(&item)->tvShowEpisode();
            if (episode != nullptr && !episode->isDummy()) {
                episode->setSyncNeeded(markForSync);
            }
        }
        ui->files->update(mIndex);
    }
}

void TvShowFilesWidget::markForSync()
{
    markForSyncBool(true);
}

void TvShowFilesWidget::unmarkForSync()
{
    markForSyncBool(false);
}

void TvShowFilesWidget::openFolder()
{
    m_contextMenu->close();
    if (!ui->files->currentIndex().isValid()) {
        return;
    }

    QString dir = [this]() {
        const QModelIndex index = m_tvShowProxyModel->mapToSource(ui->files->currentIndex());
        TvShowBaseModelItem& item = Manager::instance()->tvShowModel()->getItem(index);
        switch (item.type()) {
        case TvShowType::None: return QString{};
        case TvShowType::TvShow:
        case TvShowType::Season: return item.tvShow()->dir().toNativePathString();
        case TvShowType::Episode:
            auto* episode = dynamic_cast<EpisodeModelItem*>(&item)->tvShowEpisode();
            if (episode != nullptr && !episode->files().isEmpty() && !episode->isDummy()) {
                QFileInfo fi(episode->files().first().toString());
                return fi.absolutePath();
            }
        }
        qCritical() << "[TvShowFilesWidget] Unhandled case";
        return QString{};
    }();

    if (dir.isEmpty()) {
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void TvShowFilesWidget::openNfo()
{
    m_contextMenu->close();
    if (!ui->files->currentIndex().isValid()) {
        return;
    }

    const QModelIndex index = m_tvShowProxyModel->mapToSource(ui->files->currentIndex());
    TvShowBaseModelItem& item = Manager::instance()->tvShowModel()->getItem(index);

    QString file;
    if (item.type() == TvShowType::TvShow) {
        auto* showModel = dynamic_cast<TvShowModelItem*>(&item);
        if (showModel != nullptr) {
            file = Manager::instance()->mediaCenterInterface()->nfoFilePath(showModel->tvShow());
        }

    } else if (item.type() == TvShowType::Episode) {
        auto* episode = dynamic_cast<EpisodeModelItem*>(&item)->tvShowEpisode();
        if (episode != nullptr && !episode->files().isEmpty() && !episode->isDummy()) {
            file = Manager::instance()->mediaCenterInterface()->nfoFilePath(episode);
        }
    }

    if (file.isEmpty()) {
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(file));
}

void TvShowFilesWidget::showMissingEpisodes()
{
    m_contextMenu->close();

    forEachSelectedItem([&](TvShowBaseModelItem& item) {
        if (item.type() != TvShowType::TvShow) {
            return;
        }

        item.tvShow()->setShowMissingEpisodes(!item.tvShow()->showMissingEpisodes());
        m_actionShowMissingEpisodes->setChecked(item.tvShow()->showMissingEpisodes());

        if (!item.tvShow()->showMissingEpisodes()) {
            item.tvShow()->clearMissingEpisodes();
            return;
        }

        item.tvShow()->fillMissingEpisodes();

        if (item.tvShow()->tvdbId().isValid()) {
            TvShowUpdater::instance()->updateShow(item.tvShow());
            return;
        }

        if (Settings::instance()->showMissingEpisodesHint()) {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setWindowTitle(tr("Show update needed"));
            msgBox.setText(tr("You need to update the show once to show missing episodes.\n"
                              "Afterwards MediaElch will check automatically for new episodes on startup."));
            msgBox.setStandardButtons(QMessageBox::Ok);

            QCheckBox dontShowAgain(tr("Don't show this hint again"), &msgBox);
            dontShowAgain.blockSignals(true);

            msgBox.addButton(&dontShowAgain, QMessageBox::ActionRole);
            msgBox.exec();

            if (dontShowAgain.checkState() == Qt::Checked) {
                Settings::instance()->setShowMissingEpisodesHint(false);
            }
        }
    });
}

void TvShowFilesWidget::hideSpecialsInMissingEpisodes()
{
    m_contextMenu->close();
    forEachSelectedItem([&](TvShowBaseModelItem& item) {
        if (item.type() == TvShowType::TvShow) {
            item.tvShow()->setHideSpecialsInMissingEpisodes(!item.tvShow()->hideSpecialsInMissingEpisodes());
            m_actionHideSpecialsInMissingEpisodes->setChecked(item.tvShow()->hideSpecialsInMissingEpisodes());
            item.tvShow()->clearMissingEpisodes();
            item.tvShow()->fillMissingEpisodes();
        }
    });
}

/// \brief Sets the filters
/// \todo: respect filters and not only filter text
void TvShowFilesWidget::setFilter(const QVector<Filter*>& filters, QString text)
{
    QString filterText = filters.isEmpty() ? text : filters.first()->shortText();
    m_tvShowProxyModel->setFilterWildcard("*" + filterText + "*");
    m_tvShowProxyModel->setFilter(filters, text);
}

/// \brief Renews the model (necessary after searching for TV shows)
void TvShowFilesWidget::renewModel(bool force)
{
    qDebug() << "[TvShowFilesWidget] Renewing model | Forced:" << force;

    if (!force) {
        // When not forced, just update the view.
        updateStatusLabel();
        return;
    }

    /// \todo Check whether this code is really necessary after #883

    const int rowCount = ui->files->model()->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex showIndex = ui->files->model()->index(row, 0);

        for (int seasonRow = 0, x = ui->files->model()->rowCount(showIndex); seasonRow < x; ++seasonRow) {
            QModelIndex seasonIndex = ui->files->model()->index(seasonRow, 0, showIndex);
            ui->files->setFirstColumnSpanned(seasonRow, showIndex, true);

            for (int episodeRow = 0, y = ui->files->model()->rowCount(seasonIndex); episodeRow < y; ++episodeRow) {
                ui->files->setFirstColumnSpanned(episodeRow, seasonIndex, true);
            }
        }
    }

    updateStatusLabel();
}

/// \brief Emits sigTvShowSelected, sigSeasonSelected or sigEpisodeSelected based
///        on which model item is selected
void TvShowFilesWidget::onItemSelected(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);
    if (!current.isValid()) {
        // root item is selected
        emit sigNothingSelected();
        return;
    }

    qDebug() << "[TvShowFilesWidget] Selected item at row" << current.row() << "and column" << current.column();

    const QModelIndex sourceIndex = m_tvShowProxyModel->mapToSource(current);
    m_lastItem = &Manager::instance()->tvShowModel()->getItem(sourceIndex);

    emitLastSelection();
}

void TvShowFilesWidget::emitLastSelection()
{
    if (m_lastItem == nullptr) {
        qWarning() << "[TvShowFilesWidget] Can't emit last selection without selected item!";
        return;
    }
    const auto showType = m_lastItem->type();

    if (showType == TvShowType::TvShow && m_lastItem->tvShow() != nullptr) {
        emit sigTvShowSelected(m_lastItem->tvShow());

    } else if (showType == TvShowType::Episode) {
        auto* model = dynamic_cast<EpisodeModelItem*>(m_lastItem);
        if (model != nullptr && model->tvShowEpisode() != nullptr) {
            emit sigEpisodeSelected(model->tvShowEpisode());
        }

    } else if (showType == TvShowType::Season) {
        auto* model = dynamic_cast<SeasonModelItem*>(m_lastItem);
        if (model != nullptr && model->seasonNumber() != SeasonNumber::NoSeason) {
            emit sigSeasonSelected(model->tvShow(), model->seasonNumber());
        }
    }
}

QVector<TvShowEpisode*> TvShowFilesWidget::selectedEpisodes(bool includeFromSeasonOrShow)
{
    QVector<TvShowEpisode*> episodes;
    forEachSelectedItem([&](TvShowBaseModelItem& item) {
        if (item.type() == TvShowType::Episode) {
            auto* episode = dynamic_cast<EpisodeModelItem*>(&item)->tvShowEpisode();
            if (episode != nullptr && !episode->isDummy()) {
                episodes.append(episode);
            }

        } else if (item.type() == TvShowType::Season && includeFromSeasonOrShow) {
            auto* seasonModel = dynamic_cast<SeasonModelItem*>(&item);
            if (seasonModel == nullptr) {
                return;
            }
            for (TvShowEpisode* episode : seasonModel->tvShow()->episodes()) {
                if (episode->isDummy()) {
                    continue;
                }
                if (!episodes.contains(episode) && episode->seasonNumber() == seasonModel->seasonNumber()) {
                    episodes.append(episode);
                }
            }

        } else if (item.type() == TvShowType::TvShow && includeFromSeasonOrShow) {
            auto* showModel = dynamic_cast<TvShowModelItem*>(&item);
            if (showModel == nullptr) {
                return;
            }
            for (TvShowEpisode* episode : showModel->tvShow()->episodes()) {
                if (episode->isDummy()) {
                    continue;
                }
                if (!episodes.contains(episode)) {
                    episodes.append(episode);
                }
            }
        }
    });
    return episodes;
}

QVector<TvShow*> TvShowFilesWidget::selectedShows()
{
    QVector<TvShow*> shows;
    forEachSelectedItem([&](TvShowBaseModelItem& item) {
        if (item.type() == TvShowType::TvShow) {
            auto* show = dynamic_cast<TvShowModelItem*>(&item)->tvShow();
            if (show != nullptr) {
                shows.append(show);
            }
        }
    });
    return shows;
}


/// \brief Returns a vector of shows of which seasons are selected..
QVector<TvShow*> TvShowFilesWidget::selectedSeasons()
{
    QVector<TvShow*> shows;
    forEachSelectedItem([&](TvShowBaseModelItem& item) {
        if (item.type() == TvShowType::Season) {
            auto* show = dynamic_cast<SeasonModelItem*>(&item)->tvShow();
            if (show != nullptr && !shows.contains(show)) {
                shows.push_back(show);
            }
        }
    });
    return shows;
}

/// \brief Update the file-widget view. Updates the status label and invalidates the current
/// m_tvShowProxyModel. Called when rows are inserted or deleted or when the file searcher
/// has finished loading.
void TvShowFilesWidget::updateStatusLabel()
{
    const int rowCount = m_tvShowProxyModel->rowCount();

    if (m_tvShowProxyModel->filterRegExp().pattern().isEmpty()
        || m_tvShowProxyModel->filterRegExp().pattern() == "**") {
        int episodeCount = 0;
        for (const auto* show : Manager::instance()->tvShowModel()->tvShows()) {
            episodeCount += show->episodeCount();
        }
        ui->statusLabel->setText(tr("%n TV shows", "", rowCount) + ", " + tr("%n episodes", "", episodeCount));

    } else {
        const int showCount = Manager::instance()->tvShowModel()->tvShows().count();
        ui->statusLabel->setText(tr("%1 of %n TV shows", "", showCount).arg(rowCount));
    }
}

// void TvShowFilesWidget::updateProxy()
//{
//    m_tvShowProxyModel->invalidate();
//}

void TvShowFilesWidget::playEpisode(QModelIndex idx)
{
    if (!idx.isValid()) {
        return;
    }

    if (TvShowType(m_tvShowProxyModel->data(idx, TvShowRoles::Type).toInt()) != TvShowType::Episode) {
        return;
    }

    QString fileName = m_tvShowProxyModel->data(idx, TvShowRoles::FilePath).toString();
    if (fileName.isEmpty()) {
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

void TvShowFilesWidget::multiScrape()
{
    m_contextMenu->close();

    QVector<TvShow*> shows = selectedShows();
    QVector<TvShowEpisode*> episodes = selectedEpisodes(false);

    if (shows.isEmpty() && episodes.isEmpty()) {
        qDebug() << "[TvShowFilesWidget] Multi Scrape: Nothing selected";
        return;
    }

    qDebug() << "[TvShowFilesWidget] Multi Scrape: Selected" << shows.count() << "shows and" << episodes.count()
             << "episodes";
    if (shows.count() + episodes.count() == 1) {
        emit sigStartSearch();
        return;
    }

    auto* scrapeWidget = new TvShowMultiScrapeDialog(shows, episodes, this);
    const int result = scrapeWidget->exec();
    scrapeWidget->deleteLater();
    if (result == QDialog::Accepted) {
        emitLastSelection();
    }
}

void TvShowFilesWidget::emitSelected(QModelIndex proxyIndex)
{
    QModelIndex sourceIndex = m_tvShowProxyModel->mapToSource(proxyIndex);
    auto& item = Manager::instance()->tvShowModel()->getItem(sourceIndex);

    if (item.type() == TvShowType::TvShow) {
        emit sigTvShowSelected(item.tvShow());

    } else if (item.type() == TvShowType::Episode) {
        auto* episodeModel = dynamic_cast<EpisodeModelItem*>(&item);
        if (episodeModel != nullptr) {
            emit sigEpisodeSelected(episodeModel->tvShowEpisode());
        }
    }
}

void TvShowFilesWidget::forEachSelectedItem(std::function<void(TvShowBaseModelItem&)> callback)
{
    const QModelIndexList selectedRows = ui->files->selectionModel()->selectedRows(0);
    for (const QModelIndex& mIndex : selectedRows) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowBaseModelItem& item = Manager::instance()->tvShowModel()->getItem(index);
        callback(item);
    }
}

void TvShowFilesWidget::setupContextMenu()
{
    // clang-format off
    auto* actionMultiScrape       = new QAction(tr("Load Information"),                  this);
    auto* actionScanForEpisodes   = new QAction(tr("Search for new episodes"),           this);
    auto* actionMarkAsWatched     = new QAction(tr("Mark as watched"),                   this);
    auto* actionMarkAsUnwatched   = new QAction(tr("Mark as unwatched"),                 this);
    auto* actionLoadStreamDetails = new QAction(tr("Load Stream Details"),               this);
    auto* actionMarkForSync       = new QAction(tr("Add to Synchronization Queue"),      this);
    auto* actionUnmarkForSync     = new QAction(tr("Remove from Synchronization Queue"), this);
    auto* actionOpenFolder        = new QAction(tr("Open TV Show Folder"),               this);
    auto* actionOpenNfo           = new QAction(tr("Open NFO File"),                     this);

    connect(actionMultiScrape,       &QAction::triggered, this, &TvShowFilesWidget::multiScrape);
    connect(actionScanForEpisodes,   &QAction::triggered, this, &TvShowFilesWidget::scanForEpisodes);
    connect(actionMarkAsWatched,     &QAction::triggered, this, &TvShowFilesWidget::markAsWatched);
    connect(actionMarkAsUnwatched,   &QAction::triggered, this, &TvShowFilesWidget::markAsUnwatched);
    connect(actionLoadStreamDetails, &QAction::triggered, this, &TvShowFilesWidget::loadStreamDetails);
    connect(actionMarkForSync,       &QAction::triggered, this, &TvShowFilesWidget::markForSync);
    connect(actionUnmarkForSync,     &QAction::triggered, this, &TvShowFilesWidget::unmarkForSync);
    connect(actionOpenFolder,        &QAction::triggered, this, &TvShowFilesWidget::openFolder);
    connect(actionOpenNfo,           &QAction::triggered, this, &TvShowFilesWidget::openNfo);
    // clang-format on

    m_actionShowMissingEpisodes = new QAction(tr("Show missing episodes"), this);
    m_actionShowMissingEpisodes->setCheckable(true);

    m_actionHideSpecialsInMissingEpisodes = new QAction(tr("Hide specials in missing episodes"), this);
    m_actionHideSpecialsInMissingEpisodes->setCheckable(true);

    connect(m_actionShowMissingEpisodes, &QAction::triggered, this, &TvShowFilesWidget::showMissingEpisodes);
    connect(m_actionHideSpecialsInMissingEpisodes,
        &QAction::triggered,
        this,
        &TvShowFilesWidget::hideSpecialsInMissingEpisodes);

    m_contextMenu = new QMenu(ui->files);
    m_contextMenu->addAction(actionMultiScrape);
    m_contextMenu->addSeparator();
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
    m_contextMenu->addAction(actionOpenNfo);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_actionShowMissingEpisodes);
    m_contextMenu->addAction(m_actionHideSpecialsInMissingEpisodes);
}
