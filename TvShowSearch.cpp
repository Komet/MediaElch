#include "TvShowSearch.h"
#include "ui_TvShowSearch.h"

#include "Manager.h"

TvShowSearch::TvShowSearch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TvShowSearch)
{
    ui->setupUi(this);
    ui->results->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

#ifdef Q_WS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #TvShowSearch { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    connect(Manager::instance()->tvScrapers().at(0), SIGNAL(sigSearchDone(QList<ScraperSearchResult>)), this, SLOT(onShowResults(QList<ScraperSearchResult>)));
    connect(ui->searchString, SIGNAL(returnPressed()), this, SLOT(onSearch()));
    connect(ui->results, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onResultClicked(QTableWidgetItem*)));
    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(reject()));
}

TvShowSearch::~TvShowSearch()
{
    delete ui;
}

TvShowSearch* TvShowSearch::instance(QWidget *parent)
{
    static TvShowSearch *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new TvShowSearch(parent);
    }
    return m_instance;
}

int TvShowSearch::exec(QString searchString)
{
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height()-200);
    newSize.setWidth(qMin(600, parentWidget()->size().width()-400));
    resize(newSize);

    ui->searchString->setText(searchString);
    onSearch();
    return QDialog::exec();
}

void TvShowSearch::clear()
{
    ui->results->clearContents();
    ui->results->setRowCount(0);
}

void TvShowSearch::onSearch()
{
    clear();
    ui->searchString->setLoading(true);
    Manager::instance()->tvScrapers().at(0)->search(ui->searchString->text());
}

void TvShowSearch::onShowResults(QList<ScraperSearchResult> results)
{
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();
    foreach (const ScraperSearchResult &result, results) {
        QTableWidgetItem *item = new QTableWidgetItem(QString("%1 (%2)").arg(result.name).arg(result.released.toString("yyyy")));
        item->setData(Qt::UserRole, result.id);
        int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
    }
}

void TvShowSearch::onResultClicked(QTableWidgetItem *item)
{
    m_scraperId = item->data(Qt::UserRole).toString();
    this->accept();
}

void TvShowSearch::setChkUpdateAllVisible(bool visible)
{
    ui->chkUpdateAllEpisodes->setVisible(visible);
}

/*** GETTER ***/

QString TvShowSearch::scraperId()
{
    return m_scraperId;
}

bool TvShowSearch::updateAll()
{
    return ui->chkUpdateAllEpisodes->isChecked();
}
