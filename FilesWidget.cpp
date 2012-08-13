#include "FilesWidget.h"
#include "ui_FilesWidget.h"

#include <QLocale>
#include <QTableWidget>
#include <QTimer>
#include "Manager.h"

FilesWidget *FilesWidget::m_instance;

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

FilesWidget::~FilesWidget()
{
    delete ui;
}

FilesWidget *FilesWidget::instance()
{
    return m_instance;
}

void FilesWidget::startSearch()
{
    Manager::instance()->movieFileSearcher()->start();
}

void FilesWidget::itemActivated(QModelIndex index, QModelIndex previous)
{
    if (!index.isValid()) {
        emit noMovieSelected();
        return;
    }
    m_lastModelIndex = previous;
    int row = index.model()->data(index, Qt::UserRole).toInt();
    m_lastMovie = Manager::instance()->movieModel()->movie(row);
    QTimer::singleShot(0, this, SLOT(movieSelectedEmitter()));
}

void FilesWidget::movieSelectedEmitter()
{
    emit movieSelected(m_lastMovie);
}

void FilesWidget::setFilter(QString filter)
{
    m_movieProxyModel->setFilterWildcard("*" + filter + "*");
}

void FilesWidget::restoreLastSelection()
{
    ui->files->setCurrentIndex(m_lastModelIndex);
}
