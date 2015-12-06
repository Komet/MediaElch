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
#include "smallWidgets/LoadingStreamDetails.h"

ConcertFilesWidget *ConcertFilesWidget::m_instance;

/**
 * @brief ConcertFilesWidget::ConcertFilesWidget
 * @param parent
 */
ConcertFilesWidget::ConcertFilesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConcertFilesWidget)
{
    m_instance = this;
    ui->setupUi(this);
    ui->statusLabel->setText(tr("%n concerts", "", 0));
#ifdef Q_OS_MAC
    QFont font = ui->files->font();
    font.setPointSize(font.pointSize()-2);
    ui->files->setFont(font);
#endif
#ifdef Q_OS_WIN32
    ui->verticalLayout->setContentsMargins(0, 0, 0, 1);
#endif
    m_mouseIsIn = false;
    m_alphaList = new AlphabeticalList(this);
    m_lastConcert = 0;
    m_concertProxyModel = new ConcertProxyModel(this);
    m_concertProxyModel->setSourceModel(Manager::instance()->concertModel());
    m_concertProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_concertProxyModel->setDynamicSortFilter(true);
    ui->files->setModel(m_concertProxyModel);
    ui->files->sortByColumn(0);
#ifdef Q_OS_WIN
    ui->files->setIconSize(QSize(12, 12));
#else
    ui->files->setIconSize(QSize(16, 16));
#endif

    QAction *actionMarkAsWatched = new QAction(tr("Mark as watched"), this);
    QAction *actionMarkAsUnwatched = new QAction(tr("Mark as unwatched"), this);
    QAction *actionLoadStreamDetails = new QAction(tr("Load Stream Details"), this);
    QAction *actionMarkForSync = new QAction(tr("Add to Synchronization Queue"), this);
    QAction *actionUnmarkForSync = new QAction(tr("Remove from Synchronization Queue"), this);
    QAction *actionOpenFolder = new QAction(tr("Open Concert Folder"), this);
    QAction *actionOpenNfo = new QAction(tr("Open NFO File"), this);
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
    connect(actionMarkAsWatched, SIGNAL(triggered()), this, SLOT(markAsWatched()));
    connect(actionMarkAsUnwatched, SIGNAL(triggered()), this, SLOT(markAsUnwatched()));
    connect(actionLoadStreamDetails, SIGNAL(triggered()), this, SLOT(loadStreamDetails()));
    connect(actionMarkForSync, SIGNAL(triggered()), this, SLOT(markForSync()));
    connect(actionUnmarkForSync, SIGNAL(triggered()), this, SLOT(unmarkForSync()));
    connect(actionOpenFolder, SIGNAL(triggered()), this, SLOT(openFolder()));
    connect(actionOpenNfo, SIGNAL(triggered()), this, SLOT(openNfo()));
    connect(ui->files, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    connect(ui->files->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(itemActivated(QModelIndex, QModelIndex)));
    connect(ui->files->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(setAlphaListData()));
    connect(ui->files, SIGNAL(sigLeftEdge(bool)), this, SLOT(onLeftEdge(bool)));
    connect(ui->files, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(playConcert(QModelIndex)));

    connect(m_alphaList, SIGNAL(sigAlphaClicked(QString)), this, SLOT(scrollToAlpha(QString)));

    connect(m_concertProxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(onViewUpdated()));
    connect(m_concertProxyModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(onViewUpdated()));
}

/**
 * @brief ConcertFilesWidget::~ConcertFilesWidget
 */
ConcertFilesWidget::~ConcertFilesWidget()
{
    delete ui;
}

/**
 * @brief Returns the current instance
 * @return Instance of ConcertFilesWidget
 */
ConcertFilesWidget *ConcertFilesWidget::instance()
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
    foreach (const QModelIndex &index, ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        Concert *concert = Manager::instance()->concertModel()->concert(row);
        concert->setWatched(true);
        if (concert->playcount() < 1)
            concert->setPlayCount(1);
        if (!concert->lastPlayed().isValid())
            concert->setLastPlayed(QDateTime::currentDateTime());
    }
    if (ui->files->selectionModel()->selectedRows(0).count() > 0)
        concertSelectedEmitter();
}

void ConcertFilesWidget::markAsUnwatched()
{
    m_contextMenu->close();
    foreach (const QModelIndex &index, ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        Concert *concert = Manager::instance()->concertModel()->concert(row);
        if (concert->watched())
            concert->setWatched(false);
        if (concert->playcount() != 0)
            concert->setPlayCount(0);
    }
    if (ui->files->selectionModel()->selectedRows(0).count() > 0)
        concertSelectedEmitter();
}

void ConcertFilesWidget::loadStreamDetails()
{
    m_contextMenu->close();
    QList<Concert*> concerts;
    foreach (const QModelIndex &index, ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        Concert *concert = Manager::instance()->concertModel()->concert(row);
        concerts.append(concert);
    }
    if (concerts.count() == 1) {
        concerts.at(0)->controller()->loadStreamDetailsFromFile();
        concerts.at(0)->setChanged(true);
    } else {
        LoadingStreamDetails *loader = new LoadingStreamDetails(this);
        loader->loadConcerts(concerts);
        delete loader;
    }
    concertSelectedEmitter();
}

void ConcertFilesWidget::markForSync()
{
    m_contextMenu->close();
    foreach (const QModelIndex &index, ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        Concert *concert = Manager::instance()->concertModel()->concert(row);
        concert->setSyncNeeded(true);
        ui->files->update(index);
    }
}

void ConcertFilesWidget::unmarkForSync()
{
    m_contextMenu->close();
    foreach (const QModelIndex &index, ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        Concert *concert = Manager::instance()->concertModel()->concert(row);
        concert->setSyncNeeded(false);
        ui->files->update(index);
    }
}

void ConcertFilesWidget::openFolder()
{
    m_contextMenu->close();
    if (!ui->files->currentIndex().isValid())
        return;
    int row = ui->files->currentIndex().data(Qt::UserRole).toInt();
    Concert *concert = Manager::instance()->concertModel()->concert(row);
    if (!concert || concert->files().isEmpty())
        return;
    QFileInfo fi(concert->files().at(0));
    QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absolutePath()));
}

void ConcertFilesWidget::openNfo()
{
    m_contextMenu->close();
    if (!ui->files->currentIndex().isValid())
        return;
    int row = ui->files->currentIndex().data(Qt::UserRole).toInt();
    Concert *concert = Manager::instance()->concertModel()->concert(row);
    if (!concert || concert->files().isEmpty())
        return;
    QFileInfo fi(Manager::instance()->mediaCenterInterface()->nfoFilePath(concert));
    QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
}

/**
 * @brief Called when an item has selected
 * @param index
 * @param previous
 */
void ConcertFilesWidget::itemActivated(QModelIndex index, QModelIndex previous)
{
    qDebug() << "Entered";
    if (!index.isValid()) {
        qDebug() << "Index is invalid";
        m_lastConcert = 0;
        emit noConcertSelected();
        return;
    }
    m_lastModelIndex = previous;
    int row = index.model()->data(index, Qt::UserRole).toInt();
    m_lastConcert = Manager::instance()->concertModel()->concert(row);
    QTimer::singleShot(0, this, SLOT(concertSelectedEmitter()));
}

/**
 * @brief Just emits concertSelected
 */
void ConcertFilesWidget::concertSelectedEmitter()
{
    qDebug() << "Entered";
    if (m_lastConcert)
        emit concertSelected(m_lastConcert);
}

/**
 * @brief Sets the filters
 * @param filters List of filters
 * @param text Filter text
 */
void ConcertFilesWidget::setFilter(QList<Filter*> filters, QString text)
{
    m_concertProxyModel->setFilter(filters, text);
    m_concertProxyModel->setFilterWildcard("*" + text + "*");
    setAlphaListData();
}

/**
 * @brief Restores the last selected item
 */
void ConcertFilesWidget::restoreLastSelection()
{
    qDebug() << "Entered";
    ui->files->setCurrentIndex(m_lastModelIndex);
}

QList<Concert*> ConcertFilesWidget::selectedConcerts()
{
    QList<Concert*> concerts;
    foreach (const QModelIndex &index, ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        concerts.append(Manager::instance()->concertModel()->concert(row));
    }
    if (concerts.isEmpty())
        concerts << m_lastConcert;
    return concerts;
}

void ConcertFilesWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    int scrollBarWidth = 0;
    if (ui->files->verticalScrollBar()->isVisible())
        scrollBarWidth = ui->files->verticalScrollBar()->width();
    m_alphaList->setBottomSpace(10);
    m_alphaList->setRightSpace(scrollBarWidth+5);
    m_alphaList->adjustSize();
}

void ConcertFilesWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_mouseIsIn = true;
}

void ConcertFilesWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_mouseIsIn = false;
    m_alphaList->hide();
}

void ConcertFilesWidget::setAlphaListData()
{
    QStringList alphas;
    for (int i=0, n=ui->files->model()->rowCount() ; i<n ; ++i) {
        QString title = ui->files->model()->data(ui->files->model()->index(i, 0)).toString();
        QString first = title.left(1).toUpper();
        if (!alphas.contains(first))
            alphas.append(first);
    }
    qSort(alphas.begin(), alphas.end(), LocaleStringCompare());
    int scrollBarWidth = 0;
    if (ui->files->verticalScrollBar()->isVisible())
        scrollBarWidth = ui->files->verticalScrollBar()->width();
    m_alphaList->setRightSpace(scrollBarWidth+5);
    m_alphaList->setAlphas(alphas);
}

void ConcertFilesWidget::scrollToAlpha(QString alpha)
{
    for (int i=0, n=ui->files->model()->rowCount() ; i<n ; ++i) {
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
    if (isEdge && m_mouseIsIn)
        m_alphaList->show();
    else
        m_alphaList->hide();
}


void ConcertFilesWidget::onViewUpdated()
{
    int concertCount = Manager::instance()->concertModel()->rowCount();
    int visibleCount = m_concertProxyModel->rowCount();
    if (concertCount == visibleCount)
        ui->statusLabel->setText(tr("%n concerts", "", concertCount));
    else
        ui->statusLabel->setText(tr("%1 of %n concerts", "", concertCount).arg(visibleCount));
}

void ConcertFilesWidget::playConcert(QModelIndex idx)
{
    if (!idx.isValid())
        return;
    QString fileName = m_concertProxyModel->data(idx, Qt::UserRole+4).toString();
    if (fileName.isEmpty())
        return;
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}
