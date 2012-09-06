#include "TvShowSearch.h"
#include "ui_TvShowSearch.h"

#include "Manager.h"

/**
 * @brief TvShowSearch::TvShowSearch
 * @param parent
 */
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

/**
 * @brief TvShowSearch::~TvShowSearch
 */
TvShowSearch::~TvShowSearch()
{
    delete ui;
}

/**
 * @brief Returns the instance of the dialog
 * @param parent Parent widget (used only the first time for constructing)
 * @return Instance of the dialog
 */
TvShowSearch* TvShowSearch::instance(QWidget *parent)
{
    static TvShowSearch *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new TvShowSearch(parent);
    }
    return m_instance;
}

/**
 * @brief Adjusts size and executes the dialog
 * @param searchString String to search for
 * @return Result of QDialog::exec
 */
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

/**
 * @brief Clears the widgets contents
 */
void TvShowSearch::clear()
{
    ui->results->clearContents();
    ui->results->setRowCount(0);
}

/**
 * @brief Tells the current scraper to search
 */
void TvShowSearch::onSearch()
{
    clear();
    ui->searchString->setLoading(true);
    Manager::instance()->tvScrapers().at(0)->search(ui->searchString->text());
}

/**
 * @brief Displays the results from the scraper
 * @param results List of results
 */
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

/**
 * @brief Stores the clicked id and accepts the dialog
 * @param item Item which was clicked
 */
void TvShowSearch::onResultClicked(QTableWidgetItem *item)
{
    m_scraperId = item->data(Qt::UserRole).toString();
    this->accept();
}

/**
 * @brief Toggles the visibility of the "Update all episodes" checkbox
 * @param visible Visibility
 */
void TvShowSearch::setChkUpdateAllVisible(bool visible)
{
    ui->chkUpdateAllEpisodes->setVisible(visible);
}

/*** GETTER ***/

/**
 * @brief Returns the id of the current scraper
 * @return Id of the current scraper
 */
QString TvShowSearch::scraperId()
{
    return m_scraperId;
}

/**
 * @brief Returns the state of the "Update all episodes" checkbox
 * @return Is update all episodes checked
 */
bool TvShowSearch::updateAll()
{
    return ui->chkUpdateAllEpisodes->isChecked();
}
