#include "TvShowFilesWidget.h"
#include "ui_TvShowFilesWidget.h"

#include <QDebug>
#include <QDesktopServices>
#include "globals/Manager.h"
#include "data/TvShowModelItem.h"
#include "smallWidgets/LoadingStreamDetails.h"

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

    connect(actionScanForEpisodes, SIGNAL(triggered()), this, SLOT(scanForEpisodes()));
    connect(actionMarkAsWatched, SIGNAL(triggered()), this, SLOT(markAsWatched()));
    connect(actionMarkAsUnwatched, SIGNAL(triggered()), this, SLOT(markAsUnwatched()));
    connect(actionLoadStreamDetails, SIGNAL(triggered()), this, SLOT(loadStreamDetails()));
    connect(actionMarkForSync, SIGNAL(triggered()), this, SLOT(markForSync()));
    connect(actionUnmarkForSync, SIGNAL(triggered()), this, SLOT(unmarkForSync()));
    connect(actionOpenFolder, SIGNAL(triggered()), this, SLOT(openFolder()));
    connect(ui->files, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(ui->files, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
    connect(ui->files->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemActivated(QModelIndex,QModelIndex)));
    Manager::instance()->setTvShowFilesWidget(this);
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
    m_contextMenu->exec(ui->files->mapToGlobal(point));
}

void TvShowFilesWidget::scanForEpisodes()
{
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
    foreach (const QModelIndex &mIndex, ui->files->selectionModel()->selectedRows(0)) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
        if (item->type() == TypeTvShow || item->type() == TypeSeason) {
            foreach (TvShowEpisode *episode, item->tvShow()->episodes()) {
                if (item->type() == TypeSeason && episode->season() != item->season().toInt())
                    continue;
                if (episode->playCount() < 1)
                    episode->setPlayCount(1);
                if (!episode->lastPlayed().isValid())
                    episode->setLastPlayed(QDateTime::currentDateTime());
            }
        } else if (item->type() == TypeEpisode) {
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
    foreach (const QModelIndex &mIndex, ui->files->selectionModel()->selectedRows(0)) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
        if (item->type() == TypeTvShow || item->type() == TypeSeason) {
            foreach (TvShowEpisode *episode, item->tvShow()->episodes()) {
                if (item->type() == TypeSeason && episode->season() != item->season().toInt())
                    continue;
                if (episode->playCount() != 0)
                    episode->setPlayCount(0);
            }
        } else if (item->type() == TypeEpisode) {
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
    QList<TvShowEpisode*> episodes;

    foreach (const QModelIndex &mIndex, ui->files->selectionModel()->selectedRows(0)) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
        if (item->type() == TypeTvShow || item->type() == TypeSeason) {
            foreach (TvShowEpisode *episode, item->tvShow()->episodes()) {
                if (item->type() == TypeSeason && episode->season() != item->season().toInt())
                    continue;
                episodes.append(episode);
            }
        } else if (item->type() == TypeEpisode) {
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
    foreach (const QModelIndex &mIndex, ui->files->selectionModel()->selectedRows(0)) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
        if (item->type() == TypeTvShow) {
            item->tvShow()->setSyncNeeded(true);
        } else if (item->type() == TypeSeason) {
            foreach (TvShowEpisode *episode, item->tvShow()->episodes()) {
                if (episode->season() != item->season().toInt())
                    continue;
                episode->setSyncNeeded(true);
            }

            for (int i=0, n=item->childCount() ; i<n ; ++i)
                ui->files->update(m_tvShowProxyModel->mapFromSource(Manager::instance()->tvShowModel()->index(i, 0, index)));
        } else if (item->type() == TypeEpisode) {
            item->tvShowEpisode()->setSyncNeeded(true);
        }
        ui->files->update(mIndex);
    }
}

void TvShowFilesWidget::unmarkForSync()
{
    foreach (const QModelIndex &mIndex, ui->files->selectionModel()->selectedRows(0)) {
        QModelIndex index = m_tvShowProxyModel->mapToSource(mIndex);
        TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
        if (item->type() == TypeTvShow) {
            item->tvShow()->setSyncNeeded(false);
        } else if (item->type() == TypeSeason) {
            foreach (TvShowEpisode *episode, item->tvShow()->episodes()) {
                if (episode->season() != item->season().toInt())
                    continue;
                episode->setSyncNeeded(false);
            }
            for (int i=0, n=item->childCount() ; i<n ; ++i)
                ui->files->update(m_tvShowProxyModel->mapFromSource(Manager::instance()->tvShowModel()->index(i, 0, index)));
        } else if (item->type() == TypeEpisode) {
            item->tvShowEpisode()->setSyncNeeded(false);
        }
        ui->files->update(mIndex);
    }
}

void TvShowFilesWidget::openFolder()
{
    QModelIndex index = m_tvShowProxyModel->mapToSource(ui->files->currentIndex());
    TvShowModelItem *item = Manager::instance()->tvShowModel()->getItem(index);
    QString dir;
    if (item->type() == TypeTvShow) {
        dir = item->tvShow()->dir();
    } else if (item->type() == TypeSeason) {
        dir = item->tvShow()->dir();
    } else if (item->type() == TypeEpisode && !item->tvShowEpisode()->files().isEmpty()) {
        QFileInfo fi(item->tvShowEpisode()->files().at(0));
        dir = fi.absolutePath();
    }

    if (dir.isEmpty())
        return;
    QDesktopServices::openUrl("file://" + QDir::toNativeSeparators(dir));
}

/**
 * @brief Sets the filters
 * @param filters List of filters
 * @param text Filter text
 */
void TvShowFilesWidget::setFilter(QList<Filter *> filters, QString text)
{
    m_tvShowProxyModel->setFilter(filters, text);
    m_tvShowProxyModel->setFilterWildcard("*" + text + "*");
}

/**
 * @brief Renews the model (necessary after searching for tv shows)
 */
void TvShowFilesWidget::renewModel()
{
    qDebug() << "Entered";
    m_tvShowProxyModel->setSourceModel(0);
    m_tvShowProxyModel->setSourceModel(Manager::instance()->tvShowModel());
}

/**
 * @brief Collapses or expands items
 * @param index
 */
void TvShowFilesWidget::onItemClicked(QModelIndex index)
{
    qDebug() << "Entered";
    QModelIndex sourceIndex = m_tvShowProxyModel->mapToSource(index);
    if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeTvShow) {
        bool wasExpanded = ui->files->isExpanded(index);
        ui->files->collapseAll();
        if (!wasExpanded)
            ui->files->expand(index);
    } else if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeSeason) {
        bool wasExpanded = ui->files->isExpanded(index);
        if (wasExpanded)
            ui->files->collapse(index);
        else
            ui->files->expand(index);
    }
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
    QModelIndex sourceIndex = m_tvShowProxyModel->mapToSource(index);
    if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeTvShow) {
        m_lastTvShow = Manager::instance()->tvShowModel()->getItem(sourceIndex)->tvShow();
        emit sigTvShowSelected(m_lastTvShow);
    } else if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeEpisode) {
        m_lastEpisode = Manager::instance()->tvShowModel()->getItem(sourceIndex)->tvShowEpisode();
        emit sigEpisodeSelected(m_lastEpisode);
    }
}

void TvShowFilesWidget::emitLastSelection()
{
    if (m_lastTvShow)
        emit sigTvShowSelected(m_lastTvShow);
    else if (m_lastEpisode)
        emit sigEpisodeSelected(m_lastEpisode);
}
