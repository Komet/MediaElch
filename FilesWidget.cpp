#include "FilesWidget.h"
#include "ui_FilesWidget.h"

#include <QLocale>
#include <QTableWidget>
#include "Manager.h"

FilesWidget::FilesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilesWidget)
{
    ui->setupUi(this);
    ui->files->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->filter->setType(MyLineEdit::TypeClear);
#ifdef Q_WS_MAC
    QFont font = ui->files->font();
    font.setPointSize(font.pointSize()-2);
    ui->files->setFont(font);
#endif

    m_movieProxyModel = new MovieProxyModel(this);
    m_movieProxyModel->setSourceModel(Manager::instance()->movieModel());
    m_movieProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_movieProxyModel->setDynamicSortFilter(true);
    ui->files->setModel(m_movieProxyModel);
    ui->files->sortByColumn(0);

    connect(ui->buttonRefresh, SIGNAL(clicked()), this, SLOT(startSearch()));
    connect(Manager::instance()->movieFileSearcher(), SIGNAL(finished()), this, SLOT(searchFinished()));
    connect(ui->filter, SIGNAL(textChanged(QString)), this, SLOT(filter(QString)));
    connect(ui->files->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(itemActivated(QModelIndex)));
    connect(ui->files, SIGNAL(resized(QSize)), this, SLOT(tableViewResized(QSize)));

    QString language = QLocale::system().name().left(2);
    QString textStartFiles = ":/img/text_start_files_" + language + ".png";
    QFileInfo fi(textStartFiles);
    if (!fi.exists())
        textStartFiles = ":/img/text_start_files_en.png";
    QPixmap firstTimePixmap(textStartFiles);
    m_firstTimeLabel = new QLabel(ui->files);
    m_firstTimeLabel->setStyleSheet("border-radius: 5px; background-color: rgba(0, 0, 0, 150); padding: 20px;");
    m_firstTimeLabel->setPixmap(firstTimePixmap);
    m_firstTimeLabel->resize(firstTimePixmap.width()+40, firstTimePixmap.height()+40);
    m_firstTimeLabel->hide();
}

FilesWidget::~FilesWidget()
{
    delete ui;
}

void FilesWidget::tableViewResized(QSize size)
{
    m_firstTimeLabel->move((size.width()-m_firstTimeLabel->width())/2, 50);
}

void FilesWidget::showFirstTime()
{
    m_firstTimeLabel->show();
}

void FilesWidget::hideFirstTime()
{
    m_firstTimeLabel->hide();
}

void FilesWidget::startSearch()
{
    ui->buttonRefresh->setEnabled(false);
    Manager::instance()->movieFileSearcher()->start();
}

void FilesWidget::searchFinished()
{
    ui->buttonRefresh->setEnabled(true);
}

void FilesWidget::itemActivated(QModelIndex index)
{
    if (!index.isValid()) {
        emit noMovieSelected();
        return;
    }
    int row = index.model()->data(index, Qt::UserRole).toInt();
    Movie *movie = Manager::instance()->movieModel()->movie(row);
    emit movieSelected(movie);
}

void FilesWidget::filter(QString filter)
{
    m_movieProxyModel->setFilterWildcard("*" + filter + "*");
}

void FilesWidget::enableRefresh()
{
    ui->buttonRefresh->setEnabled(true);
}

void FilesWidget::disableRefresh()
{
    ui->buttonRefresh->setEnabled(false);
}
