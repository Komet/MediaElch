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

    connect(ui->chkBackdrop, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkCertification, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkGenres, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkOverview, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkPoster, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkRating, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkReleased, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkRuntime, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkTagline, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkTitle, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkTrailer, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkExtraArts, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkUnCheckAll, SIGNAL(clicked(bool)), this, SLOT(chkAllToggled(bool)));

    ui->chkUnCheckAll->setChecked(true);
    chkAllToggled(true);
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
        QTableWidgetItem *item = new QTableWidgetItem(QString("%1 (%2)").arg(result.name).arg(result.released.toString("yyyy")));
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
    int numOfScraperSupports = 0;

    if (ui->chkBackdrop->isChecked())
        m_infosToLoad.append(ConcertScraperInfos::Backdrop);
    if (ui->chkCertification->isChecked())
        m_infosToLoad.append(ConcertScraperInfos::Certification);
    if (ui->chkGenres->isChecked())
        m_infosToLoad.append(ConcertScraperInfos::Genres);
    if (ui->chkOverview->isChecked())
        m_infosToLoad.append(ConcertScraperInfos::Overview);
    if (ui->chkPoster->isChecked())
        m_infosToLoad.append(ConcertScraperInfos::Poster);
    if (ui->chkRating->isChecked())
        m_infosToLoad.append(ConcertScraperInfos::Rating);
    if (ui->chkReleased->isChecked())
        m_infosToLoad.append(ConcertScraperInfos::Released);
    if (ui->chkRuntime->isChecked())
        m_infosToLoad.append(ConcertScraperInfos::Runtime);
    if (ui->chkTagline->isChecked())
        m_infosToLoad.append(ConcertScraperInfos::Tagline);
    if (ui->chkTitle->isChecked())
        m_infosToLoad.append(ConcertScraperInfos::Title);
    if (ui->chkTrailer->isChecked())
        m_infosToLoad.append(ConcertScraperInfos::Trailer);
    if (ui->chkExtraArts->isChecked())
        m_infosToLoad.append(ConcertScraperInfos::ExtraArts);

    if (ui->chkBackdrop->isEnabled())
        numOfScraperSupports++;
    if (ui->chkCertification->isEnabled())
        numOfScraperSupports++;
    if (ui->chkGenres->isEnabled())
        numOfScraperSupports++;
    if (ui->chkOverview->isEnabled())
        numOfScraperSupports++;
    if (ui->chkPoster->isEnabled())
        numOfScraperSupports++;
    if (ui->chkRating->isEnabled())
        numOfScraperSupports++;
    if (ui->chkReleased->isEnabled())
        numOfScraperSupports++;
    if (ui->chkRuntime->isEnabled())
        numOfScraperSupports++;
    if (ui->chkTagline->isEnabled())
        numOfScraperSupports++;
    if (ui->chkTitle->isEnabled())
        numOfScraperSupports++;
    if (ui->chkTrailer->isEnabled())
        numOfScraperSupports++;
    if (ui->chkExtraArts->isEnabled())
        numOfScraperSupports++;

    ui->chkUnCheckAll->setChecked(m_infosToLoad.size() == numOfScraperSupports);
}

/**
 * @brief Called when "check all" checkbox is clicked
 *        Toggles all other boxes
 * @param toggled Checkbox is toggled
 */
void ConcertSearch::chkAllToggled(bool toggled)
{
    bool isChecked = toggled;
    if (ui->chkBackdrop->isEnabled())
        ui->chkBackdrop->setChecked(isChecked);
    if (ui->chkCertification->isEnabled())
        ui->chkCertification->setChecked(isChecked);
    if (ui->chkGenres->isEnabled())
        ui->chkGenres->setChecked(isChecked);
    if (ui->chkOverview->isEnabled())
        ui->chkOverview->setChecked(isChecked);
    if (ui->chkPoster->isEnabled())
        ui->chkPoster->setChecked(isChecked);
    if (ui->chkRating->isEnabled())
        ui->chkRating->setChecked(isChecked);
    if (ui->chkReleased->isEnabled())
        ui->chkReleased->setChecked(isChecked);
    if (ui->chkRuntime->isEnabled())
        ui->chkRuntime->setChecked(isChecked);
    if (ui->chkTagline->isEnabled())
        ui->chkTagline->setChecked(isChecked);
    if (ui->chkTitle->isEnabled())
        ui->chkTitle->setChecked(isChecked);
    if (ui->chkTrailer->isEnabled())
        ui->chkTrailer->setChecked(isChecked);
    if (ui->chkExtraArts->isEnabled())
        ui->chkExtraArts->setChecked(isChecked);
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
    ui->chkBackdrop->setEnabled(scraperSupports.contains(ConcertScraperInfos::Backdrop));
    ui->chkCertification->setEnabled(scraperSupports.contains(ConcertScraperInfos::Certification));
    ui->chkGenres->setEnabled(scraperSupports.contains(ConcertScraperInfos::Genres));
    ui->chkOverview->setEnabled(scraperSupports.contains(ConcertScraperInfos::Overview));
    ui->chkPoster->setEnabled(scraperSupports.contains(ConcertScraperInfos::Poster));
    ui->chkRating->setEnabled(scraperSupports.contains(ConcertScraperInfos::Rating));
    ui->chkReleased->setEnabled(scraperSupports.contains(ConcertScraperInfos::Released));
    ui->chkRuntime->setEnabled(scraperSupports.contains(ConcertScraperInfos::Runtime));
    ui->chkTagline->setEnabled(scraperSupports.contains(ConcertScraperInfos::Tagline));
    ui->chkTitle->setEnabled(scraperSupports.contains(ConcertScraperInfos::Title));
    ui->chkTrailer->setEnabled(scraperSupports.contains(ConcertScraperInfos::Trailer));
    ui->chkExtraArts->setEnabled(scraperSupports.contains(ConcertScraperInfos::ExtraArts));

    if (!scraperSupports.contains(ConcertScraperInfos::Backdrop))
        ui->chkBackdrop->setChecked(false);
    if (!scraperSupports.contains(ConcertScraperInfos::Certification))
        ui->chkCertification->setChecked(false);
    if (!scraperSupports.contains(ConcertScraperInfos::Genres))
        ui->chkGenres->setChecked(false);
    if (!scraperSupports.contains(ConcertScraperInfos::Overview))
        ui->chkOverview->setChecked(false);
    if (!scraperSupports.contains(ConcertScraperInfos::Poster))
        ui->chkPoster->setChecked(false);
    if (!scraperSupports.contains(ConcertScraperInfos::Rating))
        ui->chkRating->setChecked(false);
    if (!scraperSupports.contains(ConcertScraperInfos::Released))
        ui->chkReleased->setChecked(false);
    if (!scraperSupports.contains(ConcertScraperInfos::Runtime))
        ui->chkRuntime->setChecked(false);
    if (!scraperSupports.contains(ConcertScraperInfos::Tagline))
        ui->chkTagline->setChecked(false);
    if (!scraperSupports.contains(ConcertScraperInfos::Title))
        ui->chkTitle->setChecked(false);
    if (!scraperSupports.contains(ConcertScraperInfos::Trailer))
        ui->chkTrailer->setChecked(false);
    if (!scraperSupports.contains(ConcertScraperInfos::ExtraArts))
        ui->chkExtraArts->setChecked(false);

    chkToggled();
}
