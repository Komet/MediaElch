#include "FilesWidget.h"
#include "ui_FilesWidget.h"

#include <QLocale>
#include <QTableWidget>
#include <QTimer>
#include "globals/Globals.h"
#include "globals/Manager.h"

FilesWidget *FilesWidget::m_instance;

/**
 * @brief FilesWidget::FilesWidget
 * @param parent
 */
FilesWidget::FilesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilesWidget)
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
    m_movieDelegate = new MovieDelegate(this);
    m_movieProxyModel = new MovieProxyModel(this);
    m_movieProxyModel->setSourceModel(Manager::instance()->movieModel());
    m_movieProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_movieProxyModel->setDynamicSortFilter(true);
    ui->files->setModel(m_movieProxyModel);
    ui->files->setItemDelegate(m_movieDelegate);
    ui->files->sortByColumn(0);

    connect(ui->files->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(itemActivated(QModelIndex, QModelIndex)));
}

/**
 * @brief FilesWidget::~FilesWidget
 */
FilesWidget::~FilesWidget()
{
    delete ui;
}

/**
 * @brief Returns the current instance
 * @return Instance of FilesWidget
 */
FilesWidget *FilesWidget::instance()
{
    return m_instance;
}

/**
 * @brief Called when an item has selected
 * @param index
 * @param previous
 */
void FilesWidget::itemActivated(QModelIndex index, QModelIndex previous)
{
    qDebug() << "Entered";
    if (!index.isValid()) {
        qDebug() << "Index is invalid";
        emit noMovieSelected();
        return;
    }
    m_lastModelIndex = previous;
    int row = index.model()->data(index, Qt::UserRole).toInt();
    m_lastMovie = Manager::instance()->movieModel()->movie(row);
    QTimer::singleShot(0, this, SLOT(movieSelectedEmitter()));
}

/**
 * @brief Just emits movieSelected
 */
void FilesWidget::movieSelectedEmitter()
{
    qDebug() << "Entered";
    emit movieSelected(m_lastMovie);
}

/**
 * @brief Sets the filter
 * @param filter Filter text
 */
void FilesWidget::setFilter(QString filter)
{
    qDebug() << "Entered, filter=" << filter;
    m_movieProxyModel->setFilterWildcard("*" + filter + "*");
}

/**
 * @brief Restores the last selected item
 */
void FilesWidget::restoreLastSelection()
{
    qDebug() << "Entered";
    ui->files->setCurrentIndex(m_lastModelIndex);
}
