#include "MovieListDialog.h"
#include "ui_MovieListDialog.h"

#include "Manager.h"

MovieListDialog::MovieListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MovieListDialog)
{
    ui->setupUi(this);
    ui->movies->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#ifdef Q_WS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #MovieListDialog { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif
    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->movies, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onMovieSelected(QTableWidgetItem*)));
}

MovieListDialog::~MovieListDialog()
{
    delete ui;
}

MovieListDialog* MovieListDialog::instance(QWidget *parent)
{
    static MovieListDialog *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new MovieListDialog(parent);
    }
    return m_instance;
}

int MovieListDialog::exec()
{
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height()-200);
    newSize.setWidth(qMin(1000, parentWidget()->size().width()-400));
    resize(newSize);

    int xMove = (parentWidget()->size().width()-size().width())/2;
    QPoint globalPos = parentWidget()->mapToGlobal(parentWidget()->pos());
    move(globalPos.x()+xMove, globalPos.y());

    ui->movies->clearContents();
    ui->movies->setRowCount(0);
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        int row = ui->movies->rowCount();
        ui->movies->insertRow(row);
        QString title = (movie->released().isValid()) ? QString("%1 (%2)").arg(movie->name()).arg(movie->released().toString("yyyy")) : movie->name();
        ui->movies->setItem(row, 0, new QTableWidgetItem(title));
        ui->movies->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(movie));
    }
    return QDialog::exec();
}

void MovieListDialog::onMovieSelected(QTableWidgetItem *item)
{
    m_selectedMovie = item->data(Qt::UserRole).value<Movie*>();
    accept();
}

Movie *MovieListDialog::selectedMovie()
{
    return m_selectedMovie;
}
