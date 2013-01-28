#include "ConcertSearch.h"
#include "ui_ConcertSearch.h"

#include <QDebug>
#include "globals/Manager.h"

/**
 * @brief ConcertSearch::ConcertSearch
 * @param parent
 */
ConcertSearch::ConcertSearch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConcertSearch)
{
    ui->setupUi(this);
#if QT_VERSION >= 0x050000
    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    ui->results->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    ui->searchString->setType(MyLineEdit::TypeLoading);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #ConcertSearch { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    foreach (ConcertScraperInterface *scraper, Manager::instance()->concertScrapers()) {
        ui->comboScraper->addItem(scraper->name(), Manager::instance()->concertScrapers().indexOf(scraper));
        connect(scraper, SIGNAL(searchDone(QList<ScraperSearchResult>)), this, SLOT(showResults(QList<ScraperSearchResult>)));
    }

    connect(ui->comboScraper, SIGNAL(currentIndexChanged(int)), this, SLOT(search()));
    connect(ui->searchString, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(ui->results, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(resultClicked(QTableWidgetItem*)));
    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(reject()));

    ui->chkBackdrop->setMyData(ConcertScraperInfos::Backdrop);
    ui->chkCertification->setMyData(ConcertScraperInfos::Certification);
    ui->chkExtraArts->setMyData(ConcertScraperInfos::ExtraArts);
    ui->chkGenres->setMyData(ConcertScraperInfos::Genres);
    ui->chkOverview->setMyData(ConcertScraperInfos::Overview);
    ui->chkPoster->setMyData(ConcertScraperInfos::Poster);
    ui->chkRating->setMyData(ConcertScraperInfos::Rating);
    ui->chkReleased->setMyData(ConcertScraperInfos::Released);
    ui->chkRuntime->setMyData(ConcertScraperInfos::Runtime);
    ui->chkTagline->setMyData(ConcertScraperInfos::Tagline);
    ui->chkTitle->setMyData(ConcertScraperInfos::Title);
    ui->chkTrailer->setMyData(ConcertScraperInfos::Trailer);

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0)
            connect(box, SIGNAL(clicked()), this, SLOT(chkToggled()));
    }
    connect(ui->chkUnCheckAll, SIGNAL(clicked(bool)), this, SLOT(chkAllToggled(bool)));
}

/**
 * @brief ConcertSearch::~ConcertSearch
 */
ConcertSearch::~ConcertSearch()
{
    delete ui;
}

/**
 * @brief Returns an instance of the class
 * @param parent Parent widget
 * @return Instance of ConcertSearch
 */
ConcertSearch* ConcertSearch::instance(QWidget *parent)
{
    static ConcertSearch *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new ConcertSearch(parent);
    }
    return m_instance;
}

/**
 * @brief Executes the search dialog
 * @param searchString Concert name/search string
 * @return Result of QDialog::exec
 * @see ConcertSearch::search
 */
int ConcertSearch::exec(QString searchString)
{
    qDebug() << "Entered";
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height()-200);
    newSize.setWidth(qMin(600, parentWidget()->size().width()-400));
    resize(newSize);

    ui->searchString->setText(searchString);
    search();
    return QDialog::exec();
}

/**
 * @brief Clears the result table
 */
void ConcertSearch::clear()
{
    qDebug() << "Entered";
    ui->results->clearContents();
    ui->results->setRowCount(0);
}

/**
 * @brief Performs a search with the selected scraper
 */
void ConcertSearch::search()
{
    qDebug() << "Entered";
    int index = ui->comboScraper->currentIndex();
    if (index < 0 || index >= Manager::instance()->concertScrapers().size()) {
        return;
    }
    m_scraperNo = ui->comboScraper->itemData(index, Qt::UserRole).toInt();
    setChkBoxesEnabled(Manager::instance()->concertScrapers().at(m_scraperNo)->scraperSupports());
    clear();
    ui->comboScraper->setEnabled(false);
    ui->searchString->setLoading(true);
    Manager::instance()->concertScrapers().at(m_scraperNo)->search(ui->searchString->text());
}

/**
 * @brief Fills the result table with results
 * @param results List of results
 */
void ConcertSearch::showResults(QList<ScraperSearchResult> results)
{
    qDebug() << "Entered, size of results=" << results.count();
    ui->comboScraper->setEnabled(true);
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();
    foreach (const ScraperSearchResult &result, results) {
        QString name = result.name;
        if (result.released.isValid())
            name.append(QString(" (%1)").arg(result.released.toString("yyyy")));
        QTableWidgetItem *item = new QTableWidgetItem(name);
        item->setData(Qt::UserRole, result.id);
        int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
    }
}

/**
 * @brief Called when an item in the result table was clicked
 *        Accepts and closes the dialog
 * @param item Clicked table item
 */
void ConcertSearch::resultClicked(QTableWidgetItem *item)
{
    qDebug() << "Entered";
    m_scraperId = item->data(Qt::UserRole).toString();
    accept();
}

/**
 * @brief Called when one of the checkboxes are clicked.
 *        Clears m_infosToLoad and fills it with the items which are checked.
 *        If all enabled items are checked, "check all" checkbox is checked too
 */
void ConcertSearch::chkToggled()
{
    m_infosToLoad.clear();
    bool allToggled = true;
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isChecked() && box->myData().toInt() > 0 && box->isEnabled())
            m_infosToLoad.append(box->myData().toInt());
        if (!box->isChecked() && box->myData().toInt() > 0 && box->isEnabled())
            allToggled = false;
    }
    ui->chkUnCheckAll->setChecked(allToggled);

    int scraperNo = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toInt();
    Settings::instance()->setScraperInfos(WidgetConcerts, scraperNo, m_infosToLoad);
}

/**
 * @brief Called when "check all" checkbox is clicked
 *        Toggles all other boxes
 * @param toggled Checkbox is toggled
 */
void ConcertSearch::chkAllToggled(bool toggled)
{
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0 && box->isEnabled())
            box->setChecked(toggled);
    }
    chkToggled();
}

/*** GETTER ***/

/**
 * @brief ConcertSearch::scraperNo
 * @return Current scraper number
 */
int ConcertSearch::scraperNo()
{
    qDebug() << "Entered, m_scraperNo=" << m_scraperNo;
    return m_scraperNo;
}

/**
 * @brief ConcertSearch::scraperId
 * @return Scraper id of the concert last clicked in result table
 */
QString ConcertSearch::scraperId()
{
    qDebug() << "Entered, m_scraperId=" << m_scraperId;
    return m_scraperId;
}

/**
 * @brief ConcertSearch::infosToLoad
 * @return List of infos to load from the scraper
 */
QList<int> ConcertSearch::infosToLoad()
{
    return m_infosToLoad;
}

/**
 * @brief Sets status of checkboxes based on the support of the scraper
 * @param scraperSupports List of infos supported by the scraper
 */
void ConcertSearch::setChkBoxesEnabled(QList<int> scraperSupports)
{
    int scraperNo = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toInt();
    QList<int> infos = Settings::instance()->scraperInfos(WidgetConcerts, scraperNo);

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        box->setEnabled(scraperSupports.contains(box->myData().toInt()));
        box->setChecked((infos.contains(box->myData().toInt()) || infos.isEmpty()) && scraperSupports.contains(box->myData().toInt()));
    }
    chkToggled();
}
