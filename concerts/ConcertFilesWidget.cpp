#include "ConcertFilesWidget.h"
#include "ui_ConcertFilesWidget.h"

#include <QLocale>
#include <QTableWidget>
#include <QTimer>
#include "globals/Globals.h"
#include "globals/Manager.h"

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
    ui->files->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#ifdef Q_WS_MAC
    QFont font = ui->files->font();
    font.setPointSize(font.pointSize()-2);
    ui->files->setFont(font);
#endif
#ifdef Q_WS_WIN
    ui->verticalLayout->setContentsMargins(0, 0, 0, 1);
#endif
    m_concertDelegate = new ConcertDelegate(this);
    m_concertProxyModel = new ConcertProxyModel(this);
    m_concertProxyModel->setSourceModel(Manager::instance()->concertModel());
    m_concertProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_concertProxyModel->setDynamicSortFilter(true);
    ui->files->setModel(m_concertProxyModel);
    ui->files->setItemDelegate(m_concertDelegate);
    ui->files->sortByColumn(0);

    connect(ui->files->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(itemActivated(QModelIndex, QModelIndex)));
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
    emit concertSelected(m_lastConcert);
}

/**
 * @brief Sets the filter
 * @param filter Filter text
 */
void ConcertFilesWidget::setFilter(QString filter)
{
    qDebug() << "Entered, filter=" << filter;
    m_concertProxyModel->setFilterWildcard("*" + filter + "*");
}

/**
 * @brief Restores the last selected item
 */
void ConcertFilesWidget::restoreLastSelection()
{
    qDebug() << "Entered";
    ui->files->setCurrentIndex(m_lastModelIndex);
}
