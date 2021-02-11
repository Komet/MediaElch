#include "ConcertFilesWidget.h"
#include "ui_ConcertFilesWidget.h"

#include <QDesktopServices>
#include <QLocale>
#include <QScrollBar>
#include <QTableWidget>
#include <QTimer>

#include "globals/Globals.h"
#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"
#include "ui/small_widgets/LoadingStreamDetails.h"

ConcertFilesWidget* ConcertFilesWidget::m_instance;

ConcertFilesWidget::ConcertFilesWidget(QWidget* parent) : QWidget(parent), ui(new Ui::ConcertFilesWidget)
{
    m_instance = this;
    ui->setupUi(this);
    ui->statusLabel->setText(tr("%n concerts", "", 0));

#ifdef Q_OS_WIN
    ui->verticalLayout->setContentsMargins(0, 0, 0, 1);
#endif
    m_mouseIsIn = false;
    m_alphaList = new AlphabeticalList(this);
    m_lastConcert = nullptr;
    m_concertProxyModel = new ConcertProxyModel(this);
    m_concertProxyModel->setSourceModel(Manager::instance()->concertModel());
    m_concertProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_concertProxyModel->setDynamicSortFilter(true);
    ui->files->setModel(m_concertProxyModel);
    ui->files->sortByColumn(0, Qt::AscendingOrder);
#ifdef Q_OS_WIN
    ui->files->setIconSize(QSize(12, 12));
#else
    ui->files->setIconSize(QSize(16, 16));
#endif

    auto* actionMarkAsWatched = new QAction(tr("Mark as watched"), this);
    auto* actionMarkAsUnwatched = new QAction(tr("Mark as unwatched"), this);
    auto* actionLoadStreamDetails = new QAction(tr("Load Stream Details"), this);
    auto* actionMarkForSync = new QAction(tr("Add to Synchronization Queue"), this);
    auto* actionUnmarkForSync = new QAction(tr("Remove from Synchronization Queue"), this);
    auto* actionOpenFolder = new QAction(tr("Open Concert Folder"), this);
    auto* actionOpenNfo = new QAction(tr("Open NFO File"), this);
    m_contextMenu = new QMenu(ui->files);
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

    // clang-format off
    connect(actionMarkAsWatched,     &QAction::triggered, this, &ConcertFilesWidget::markAsWatched);
    connect(actionMarkAsUnwatched,   &QAction::triggered, this, &ConcertFilesWidget::markAsUnwatched);
    connect(actionLoadStreamDetails, &QAction::triggered, this, &ConcertFilesWidget::loadStreamDetails);
    connect(actionMarkForSync,       &QAction::triggered, this, &ConcertFilesWidget::markForSync);
    connect(actionUnmarkForSync,     &QAction::triggered, this, &ConcertFilesWidget::unmarkForSync);
    connect(actionOpenFolder,        &QAction::triggered, this, &ConcertFilesWidget::openFolder);
    connect(actionOpenNfo,           &QAction::triggered, this, &ConcertFilesWidget::openNfo);

    connect(ui->files, &QWidget::customContextMenuRequested, this, &ConcertFilesWidget::showContextMenu);

    connect(ui->files->selectionModel(), &QItemSelectionModel::currentChanged, this, &ConcertFilesWidget::itemActivated);
    connect(ui->files->model(),          &QAbstractItemModel::dataChanged,     this, &ConcertFilesWidget::setAlphaListData);
    connect(ui->files,                   &MyTableView::sigLeftEdge,            this, &ConcertFilesWidget::onLeftEdge);
    connect(ui->files,                   &QAbstractItemView::doubleClicked,    this, &ConcertFilesWidget::playConcert);

    connect(m_alphaList, &AlphabeticalList::sigAlphaClicked, this, &ConcertFilesWidget::scrollToAlpha);

    connect(m_concertProxyModel, &QAbstractItemModel::rowsInserted, this, &ConcertFilesWidget::updateStatusLabel);
    connect(m_concertProxyModel, &QAbstractItemModel::rowsRemoved,  this, &ConcertFilesWidget::updateStatusLabel);
    // clang-format on

    // FIXME:
    // For some reason, the proxy model emits "rowsRemoved" before "endRemoveRows()" is called in the source model.
    connect(Manager::instance()->concertFileSearcher(),
        &ConcertFileSearcher::concertsLoaded,
        this,
        &ConcertFilesWidget::updateStatusLabel);
}

/**
 * \brief ConcertFilesWidget::~ConcertFilesWidget
 */
ConcertFilesWidget::~ConcertFilesWidget()
{
    delete ui;
}

/**
 * \brief Returns the current instance
 * \return Instance of ConcertFilesWidget
 */
ConcertFilesWidget* ConcertFilesWidget::instance()
{
    return m_instance;
}

void ConcertFilesWidget::showContextMenu(QPoint point)
{
    m_contextMenu->exec(ui->files->mapToGlobal(point));
}

void ConcertFilesWidget::markAsWatched()
{
    m_contextMenu->close();
    for (const QModelIndex& index : ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        Concert* concert = Manager::instance()->concertModel()->concert(row);
        concert->setPlayCount(std::max(1, concert->playcount()));

        if (!concert->lastPlayed().isValid()) {
            concert->setLastPlayed(QDateTime::currentDateTime());
        }
    }
    if (ui->files->selectionModel()->selectedRows(0).count() > 0) {
        concertSelectedEmitter();
    }
}

void ConcertFilesWidget::markAsUnwatched()
{
    m_contextMenu->close();
    for (const QModelIndex& index : ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        Concert* concert = Manager::instance()->concertModel()->concert(row);
        concert->setPlayCount(0);
        concert->setLastPlayed(QDateTime{});
    }
    if (ui->files->selectionModel()->selectedRows(0).count() > 0) {
        concertSelectedEmitter();
    }
}

void ConcertFilesWidget::loadStreamDetails()
{
    m_contextMenu->close();
    QVector<Concert*> concerts;
    for (const QModelIndex& index : ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        Concert* concert = Manager::instance()->concertModel()->concert(row);
        concerts.append(concert);
    }
    if (concerts.count() == 1) {
        concerts.at(0)->controller()->loadStreamDetailsFromFile();
        concerts.at(0)->setChanged(true);
    } else {
        auto* loader = new LoadingStreamDetails(this);
        loader->loadConcerts(concerts);
        delete loader;
    }
    concertSelectedEmitter();
}

void ConcertFilesWidget::markForSync()
{
    m_contextMenu->close();
    for (const QModelIndex& index : ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        Concert* concert = Manager::instance()->concertModel()->concert(row);
        concert->setSyncNeeded(true);
        ui->files->update(index);
    }
}

void ConcertFilesWidget::unmarkForSync()
{
    m_contextMenu->close();
    for (const QModelIndex& index : ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        Concert* concert = Manager::instance()->concertModel()->concert(row);
        concert->setSyncNeeded(false);
        ui->files->update(index);
    }
}

void ConcertFilesWidget::openFolder()
{
    m_contextMenu->close();
    if (!ui->files->currentIndex().isValid()) {
        return;
    }
    int row = ui->files->currentIndex().data(Qt::UserRole).toInt();
    Concert* concert = Manager::instance()->concertModel()->concert(row);
    if ((concert == nullptr) || concert->files().isEmpty()) {
        return;
    }
    QFileInfo fi(concert->files().at(0).toString());
    QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absolutePath()));
}

void ConcertFilesWidget::openNfo()
{
    m_contextMenu->close();
    if (!ui->files->currentIndex().isValid()) {
        return;
    }
    int row = ui->files->currentIndex().data(Qt::UserRole).toInt();
    Concert* concert = Manager::instance()->concertModel()->concert(row);
    if ((concert == nullptr) || concert->files().isEmpty()) {
        return;
    }
    QFileInfo fi(Manager::instance()->mediaCenterInterface()->nfoFilePath(concert));
    QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
}

/**
 * \brief Called when an item has selected
 */
void ConcertFilesWidget::itemActivated(QModelIndex index, QModelIndex previous)
{
    if (!index.isValid()) {
        qDebug() << "Index is invalid";
        m_lastConcert = nullptr;
        emit noConcertSelected();
        return;
    }
    m_lastModelIndex = previous;
    int row = index.model()->data(index, Qt::UserRole).toInt();
    m_lastConcert = Manager::instance()->concertModel()->concert(row);
    QTimer::singleShot(0, this, &ConcertFilesWidget::concertSelectedEmitter);
}

/**
 * \brief Just emits concertSelected
 */
void ConcertFilesWidget::concertSelectedEmitter()
{
    if (m_lastConcert != nullptr) {
        emit concertSelected(m_lastConcert);
    }
}

/**
 * \brief Sets the filters
 * \param filters List of filters
 * \param text Filter text
 */
void ConcertFilesWidget::setFilter(QVector<Filter*> filters, QString text)
{
    m_concertProxyModel->setFilter(filters, text);
    m_concertProxyModel->setFilterWildcard("*" + text + "*");
    setAlphaListData();
}

/**
 * \brief Restores the last selected item
 */
void ConcertFilesWidget::restoreLastSelection()
{
    ui->files->setCurrentIndex(m_lastModelIndex);
}

QVector<Concert*> ConcertFilesWidget::selectedConcerts()
{
    QVector<Concert*> concerts;
    for (const QModelIndex& index : ui->files->selectionModel()->selectedRows(0)) {
        const int row = index.model()->data(index, Qt::UserRole).toInt();
        concerts.append(Manager::instance()->concertModel()->concert(row));
    }
    if (concerts.isEmpty()) {
        concerts << m_lastConcert;
    }
    return concerts;
}

void ConcertFilesWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)
    int scrollBarWidth = 0;
    if (ui->files->verticalScrollBar()->isVisible()) {
        scrollBarWidth = ui->files->verticalScrollBar()->width();
    }
    m_alphaList->setBottomSpace(10);
    m_alphaList->setRightSpace(scrollBarWidth + 5);
    m_alphaList->adjustSize();
}

void ConcertFilesWidget::enterEvent(QEvent* event)
{
    Q_UNUSED(event)
    m_mouseIsIn = true;
}

void ConcertFilesWidget::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
    m_mouseIsIn = false;
    m_alphaList->hide();
}

void ConcertFilesWidget::setAlphaListData()
{
    QStringList alphas;
    for (int i = 0, n = ui->files->model()->rowCount(); i < n; ++i) {
        QString title = ui->files->model()->data(ui->files->model()->index(i, 0)).toString();
        QString first = title.left(1).toUpper();
        if (!alphas.contains(first)) {
            alphas.append(first);
        }
    }
    std::sort(alphas.begin(), alphas.end(), LocaleStringCompare());
    int scrollBarWidth = 0;
    if (ui->files->verticalScrollBar()->isVisible()) {
        scrollBarWidth = ui->files->verticalScrollBar()->width();
    }
    m_alphaList->setRightSpace(scrollBarWidth + 5);
    m_alphaList->setAlphas(alphas);
}

void ConcertFilesWidget::scrollToAlpha(QString alpha)
{
    for (int i = 0, n = ui->files->model()->rowCount(); i < n; ++i) {
        QModelIndex index = ui->files->model()->index(i, 0);
        QString title = ui->files->model()->data(index).toString();
        QString first = title.left(1).toUpper();
        if (first == alpha) {
            ui->files->scrollTo(index, QAbstractItemView::PositionAtTop);
            return;
        }
    }
}

void ConcertFilesWidget::renewModel()
{
    m_concertProxyModel->setSourceModel(Manager::instance()->concertModel());
}

void ConcertFilesWidget::onLeftEdge(bool isEdge)
{
    if (isEdge && m_mouseIsIn) {
        m_alphaList->show();
    } else {
        m_alphaList->hide();
    }
}


void ConcertFilesWidget::updateStatusLabel()
{
    int concertCount = m_concertProxyModel->sourceModel()->rowCount();
    int visibleCount = m_concertProxyModel->rowCount();
    if (concertCount == visibleCount) {
        ui->statusLabel->setText(tr("%n concerts", "", concertCount));
    } else {
        ui->statusLabel->setText(tr("%1 of %n concerts", "", concertCount).arg(visibleCount));
    }
}

void ConcertFilesWidget::playConcert(QModelIndex idx)
{
    if (!idx.isValid()) {
        return;
    }
    QString fileName = m_concertProxyModel->data(idx, Qt::UserRole + 4).toString();
    if (fileName.isEmpty()) {
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}
