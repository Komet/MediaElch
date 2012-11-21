#include "TvShowFilesWidget.h"
#include "ui_TvShowFilesWidget.h"

#include <QDebug>
#include "globals/Manager.h"
#include "data/TvShowModelItem.h"

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

#ifdef Q_WS_MAC
    QFont font = ui->files->font();
    font.setPointSize(font.pointSize()-2);
    ui->files->setFont(font);
#endif
#ifdef Q_WS_WIN
    ui->verticalLayout->setContentsMargins(0, 0, 0, 1);
#endif

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
    m_contextMenu = new QMenu(ui->files);
    m_contextMenu->addAction(actionScanForEpisodes);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(actionMarkAsWatched);
    m_contextMenu->addAction(actionMarkAsUnwatched);
    connect(actionScanForEpisodes, SIGNAL(triggered()), this, SLOT(scanForEpisodes()));
    connect(actionMarkAsWatched, SIGNAL(triggered()), this, SLOT(markAsWatched()));
    connect(actionMarkAsUnwatched, SIGNAL(triggered()), this, SLOT(markAsUnwatched()));
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

    QModelIndex sourceIndex = m_tvShowProxyModel->mapToSource(index);
    if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeTvShow) {
        emit sigTvShowSelected(Manager::instance()->tvShowModel()->getItem(sourceIndex)->tvShow());
    } else if (Manager::instance()->tvShowModel()->getItem(sourceIndex)->type() == TypeEpisode) {
        emit sigEpisodeSelected(Manager::instance()->tvShowModel()->getItem(sourceIndex)->tvShowEpisode());
    }
}
