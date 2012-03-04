#include "MovieSearch.h"
#include "ui_MovieSearch.h"

#include <QDebug>
#include "Manager.h"

MovieSearch::MovieSearch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MovieSearch)
{
    ui->setupUi(this);
    ui->results->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

    Qt::WindowFlags flags = windowFlags();
#ifdef Q_WS_WIN
    flags |= Qt::Dialog;
#else
    flags |= Qt::SplashScreen;
    setStyleSheet(styleSheet() + " #MovieSearch { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#endif
    setWindowFlags(flags);

    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        ui->comboScraper->addItem(scraper->name(), Manager::instance()->scrapers().indexOf(scraper));
        connect(scraper, SIGNAL(searchDone(QList<ScraperSearchResult>)), this, SLOT(showResults(QList<ScraperSearchResult>)));
    }

    connect(ui->comboScraper, SIGNAL(currentIndexChanged(int)), this, SLOT(search()));
    connect(ui->searchString, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(ui->results, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(resultClicked(QTableWidgetItem*)));
    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(reject()));
}

MovieSearch::~MovieSearch()
{
    delete ui;
}

MovieSearch* MovieSearch::instance(QWidget *parent)
{
    static MovieSearch *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new MovieSearch(parent);
    }
    return m_instance;
}

int MovieSearch::exec(QString searchString)
{
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height()-200);
    newSize.setWidth(qMin(600, parentWidget()->size().width()-400));
    resize(newSize);

    int xMove = (parentWidget()->size().width()-size().width())/2;
    QPoint globalPos = parentWidget()->mapToGlobal(parentWidget()->pos());
    move(globalPos.x()+xMove, globalPos.y());

    ui->searchString->setText(searchString);
    this->search();
    return QDialog::exec();
}

void MovieSearch::clear()
{
    ui->results->clearContents();
    ui->results->setRowCount(0);
}

void MovieSearch::search()
{
    int index = ui->comboScraper->currentIndex();
    if (index < 0 || index >= Manager::instance()->scrapers().size()) {
        return;
    }
    m_scraperNo = ui->comboScraper->itemData(index, Qt::UserRole).toInt();
    this->clear();
    ui->comboScraper->setEnabled(false);
    ui->searchString->setLoading(true);
    Manager::instance()->scrapers().at(m_scraperNo)->search(ui->searchString->text());
}

void MovieSearch::showResults(QList<ScraperSearchResult> results)
{
    ui->comboScraper->setEnabled(true);
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

void MovieSearch::resultClicked(QTableWidgetItem *item)
{
    m_scraperId = item->data(Qt::UserRole).toString();
    this->accept();
}

/*** GETTER ***/

int MovieSearch::scraperNo()
{
    return m_scraperNo;
}

QString MovieSearch::scraperId()
{
    return m_scraperId;
}
