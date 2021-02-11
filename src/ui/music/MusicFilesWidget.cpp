#include "MusicFilesWidget.h"
#include "ui_MusicFilesWidget.h"

#include <QDebug>
#include <QDesktopServices>

#include "MusicMultiScrapeDialog.h"
#include "globals/Manager.h"

MusicFilesWidget* MusicFilesWidget::m_instance;

MusicFilesWidget::MusicFilesWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MusicFilesWidget)
{
    m_instance = this;
    Manager::instance()->setMusicFilesWidget(this);
    ui->setupUi(this);

    m_proxyModel = new MusicProxyModel(this);
    m_proxyModel->setSourceModel(Manager::instance()->musicModel());
    ui->music->setModel(m_proxyModel);
    ui->music->sortByColumn(0, Qt::AscendingOrder);
    ui->music->setAttribute(Qt::WA_MacShowFocusRect, false);

    ui->statusLabel->setText(tr("%n artists", "", 0) + ", " + tr("%n albums", "", 0));

#ifdef Q_OS_WIN
    ui->music->setAnimated(false);
#endif

    auto* actionOpenFolder = new QAction(tr("Open Folder"), this);
    auto* actionOpenNfo = new QAction(tr("Open NFO File"), this);
    m_contextMenu = new QMenu(ui->music);
    m_contextMenu->addAction(actionOpenFolder);
    m_contextMenu->addAction(actionOpenNfo);
    connect(actionOpenFolder, &QAction::triggered, this, &MusicFilesWidget::onOpenFolder);
    connect(actionOpenNfo, &QAction::triggered, this, &MusicFilesWidget::onOpenNfo);
    connect(ui->music, &QWidget::customContextMenuRequested, this, &MusicFilesWidget::showContextMenu);

    connect(ui->music->selectionModel(),
        &QItemSelectionModel::currentChanged,
        this,
        &MusicFilesWidget::onItemSelected,
        Qt::QueuedConnection);
    connect(m_proxyModel, &QAbstractItemModel::rowsInserted, this, &MusicFilesWidget::updateStatusLabel);
    connect(m_proxyModel, &QAbstractItemModel::rowsRemoved, this, &MusicFilesWidget::updateStatusLabel);

    // FIXME:
    // For some reason, the proxy model emits "rowsRemoved" before "endRemoveRows()" is called in the source model.
    connect(Manager::instance()->musicFileSearcher(),
        &MusicFileSearcher::musicLoaded,
        this,
        &MusicFilesWidget::updateStatusLabel);
}

MusicFilesWidget::~MusicFilesWidget()
{
    delete ui;
}

void MusicFilesWidget::showContextMenu(QPoint point)
{
    m_contextMenu->exec(ui->music->mapToGlobal(point));
}

void MusicFilesWidget::onOpenFolder()
{
    m_contextMenu->close();
    if (!ui->music->currentIndex().isValid()) {
        return;
    }
    QModelIndex index = m_proxyModel->mapToSource(ui->music->currentIndex());
    MusicModelItem* item = Manager::instance()->musicModel()->getItem(index);
    if (item == nullptr) {
        return;
    }
    mediaelch::DirectoryPath dir;
    if (item->type() == MusicType::Artist) {
        dir = item->artist()->path();
    } else if (item->type() == MusicType::Album) {
        dir = item->album()->path();
    }

    if (!dir.isValid()) {
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(dir.toNativePathString()));
}

void MusicFilesWidget::onOpenNfo()
{
    m_contextMenu->close();
    if (!ui->music->currentIndex().isValid()) {
        return;
    }
    QModelIndex index = m_proxyModel->mapToSource(ui->music->currentIndex());
    MusicModelItem* item = Manager::instance()->musicModel()->getItem(index);
    if (item == nullptr) {
        return;
    }
    QString file;
    if (item->type() == MusicType::Artist) {
        file = Manager::instance()->mediaCenterInterface()->nfoFilePath(item->artist());
    } else if (item->type() == MusicType::Album) {
        file = Manager::instance()->mediaCenterInterface()->nfoFilePath(item->album());
    }

    if (file.isEmpty()) {
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(file));
}

MusicFilesWidget* MusicFilesWidget::instance()
{
    return m_instance;
}

void MusicFilesWidget::setFilter(QVector<Filter*> filters, QString text)
{
    if (!filters.isEmpty()) {
        m_proxyModel->setFilterWildcard("*" + filters.first()->shortText() + "*");
    } else {
        m_proxyModel->setFilterWildcard("*" + text + "*");
    }
    m_proxyModel->setFilter(filters, text);
}

void MusicFilesWidget::onItemSelected(QModelIndex index)
{
    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    if (Manager::instance()->musicModel()->getItem(sourceIndex)->type() == MusicType::Artist) {
        emit sigArtistSelected(Manager::instance()->musicModel()->getItem(sourceIndex)->artist());
    } else if (Manager::instance()->musicModel()->getItem(sourceIndex)->type() == MusicType::Album) {
        emit sigAlbumSelected(Manager::instance()->musicModel()->getItem(sourceIndex)->album());
    }
}

void MusicFilesWidget::updateStatusLabel()
{
    if (m_proxyModel->filterRegExp().pattern().isEmpty() || m_proxyModel->filterRegExp().pattern() == "**") {
        int albumCount = 0;
        for (Artist* artist : Manager::instance()->musicModel()->artists()) {
            albumCount += artist->albums().count();
        }
        ui->statusLabel->setText(
            tr("%n artists", "", m_proxyModel->rowCount()) + ", " + tr("%n albums", "", albumCount));
    } else {
        ui->statusLabel->setText(tr("%1 of %n artists", "", Manager::instance()->musicModel()->artists().count())
                                     .arg(m_proxyModel->rowCount()));
    }
}

QVector<Artist*> MusicFilesWidget::selectedArtists()
{
    QVector<Artist*> artists;
    for (const QModelIndex& index : ui->music->selectionModel()->selectedIndexes()) {
        MusicModelItem* item = Manager::instance()->musicModel()->getItem(m_proxyModel->mapToSource(index));
        if (item->type() == MusicType::Artist) {
            artists.append(item->artist());
        }
    }
    return artists;
}

QVector<Album*> MusicFilesWidget::selectedAlbums()
{
    QVector<Album*> albums;
    for (const QModelIndex& index : ui->music->selectionModel()->selectedIndexes()) {
        MusicModelItem* item = Manager::instance()->musicModel()->getItem(m_proxyModel->mapToSource(index));
        if (item->type() == MusicType::Album) {
            albums.append(item->album());
        }
    }
    return albums;
}

void MusicFilesWidget::multiScrape()
{
    auto* scrapeWidget = new MusicMultiScrapeDialog(this);
    scrapeWidget->setItems(selectedArtists(), selectedAlbums());
    scrapeWidget->exec();
    scrapeWidget->deleteLater();
}
