#include "MovieSearch.h"
#include "ui_MovieSearch.h"

#include <QDebug>
#include "globals/Manager.h"
#include "scrapers/CustomMovieScraper.h"

/**
 * @brief MovieSearch::MovieSearch
 * @param parent
 */
MovieSearch::MovieSearch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MovieSearch)
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
    setStyleSheet(styleSheet() + " #MovieSearch { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        ui->comboScraper->addItem(scraper->name(), scraper->identifier());
        connect(scraper, SIGNAL(searchDone(QList<ScraperSearchResult>)), this, SLOT(showResults(QList<ScraperSearchResult>)));
    }

    connect(ui->comboScraper, SIGNAL(currentIndexChanged(int)), this, SLOT(search()));
    connect(ui->searchString, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(ui->results, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(resultClicked(QTableWidgetItem*)));
    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(reject()));

    ui->chkActors->setMyData(MovieScraperInfos::Actors);
    ui->chkBackdrop->setMyData(MovieScraperInfos::Backdrop);
    ui->chkCertification->setMyData(MovieScraperInfos::Certification);
    ui->chkCountries->setMyData(MovieScraperInfos::Countries);
    ui->chkDirector->setMyData(MovieScraperInfos::Director);
    ui->chkGenres->setMyData(MovieScraperInfos::Genres);
    ui->chkOverview->setMyData(MovieScraperInfos::Overview);
    ui->chkPoster->setMyData(MovieScraperInfos::Poster);
    ui->chkRating->setMyData(MovieScraperInfos::Rating);
    ui->chkReleased->setMyData(MovieScraperInfos::Released);
    ui->chkRuntime->setMyData(MovieScraperInfos::Runtime);
    ui->chkSet->setMyData(MovieScraperInfos::Set);
    ui->chkStudios->setMyData(MovieScraperInfos::Studios);
    ui->chkTagline->setMyData(MovieScraperInfos::Tagline);
    ui->chkTitle->setMyData(MovieScraperInfos::Title);
    ui->chkTrailer->setMyData(MovieScraperInfos::Trailer);
    ui->chkWriter->setMyData(MovieScraperInfos::Writer);
    ui->chkLogo->setMyData(MovieScraperInfos::Logo);
    ui->chkClearArt->setMyData(MovieScraperInfos::ClearArt);
    ui->chkCdArt->setMyData(MovieScraperInfos::CdArt);
    ui->chkBanner->setMyData(MovieScraperInfos::Banner);
    ui->chkThumb->setMyData(MovieScraperInfos::Thumb);

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0)
            connect(box, SIGNAL(clicked()), this, SLOT(chkToggled()));
    }
    connect(ui->chkUnCheckAll, SIGNAL(clicked(bool)), this, SLOT(chkAllToggled(bool)));
}

/**
 * @brief MovieSearch::~MovieSearch
 */
MovieSearch::~MovieSearch()
{
    delete ui;
}

/**
 * @brief Returns an instance of the class
 * @param parent Parent widget
 * @return Instance of MovieSearch
 */
MovieSearch* MovieSearch::instance(QWidget *parent)
{
    static MovieSearch *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new MovieSearch(parent);
    }
    return m_instance;
}

/**
 * @brief Executes the search dialog
 * @param searchString Movie name/search string
 * @return Result of QDialog::exec
 * @see MovieSearch::search
 */
int MovieSearch::exec(QString searchString)
{
    qDebug() << "Entered";
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height()-200);
    newSize.setWidth(qMin(600, parentWidget()->size().width()-400));
    resize(newSize);

    ui->comboScraper->setEnabled(true);
    ui->groupBox->setEnabled(true);
    m_currentCustomScraper = 0;
    m_customScraperIds.clear();

    ui->searchString->setText(searchString);
    search();
    return QDialog::exec();
}

/**
 * @brief Clears the result table
 */
void MovieSearch::clear()
{
    qDebug() << "Entered";
    ui->results->clearContents();
    ui->results->setRowCount(0);
}

/**
 * @brief Performs a search with the selected scraper
 */
void MovieSearch::search()
{
    qDebug() << "Entered";
    int index = ui->comboScraper->currentIndex();
    if (index < 0 || index >= Manager::instance()->scrapers().size()) {
        return;
    }
    m_scraperId = ui->comboScraper->itemData(index, Qt::UserRole).toString();
    ScraperInterface *scraper = Manager::instance()->scraper(m_scraperId);
    if (!scraper)
        return;

    if (m_scraperId == "custom-movie")
        m_currentCustomScraper = CustomMovieScraper::instance()->titleScraper();

    setChkBoxesEnabled(Manager::instance()->scraper(m_scraperId)->scraperSupports());
    clear();
    ui->comboScraper->setEnabled(false);
    ui->searchString->setLoading(true);
    scraper->search(ui->searchString->text());
}

/**
 * @brief Fills the result table with results
 * @param results List of results
 */
void MovieSearch::showResults(QList<ScraperSearchResult> results)
{
    qDebug() << "Entered, size of results=" << results.count();
    ui->comboScraper->setEnabled(m_customScraperIds.isEmpty());
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
void MovieSearch::resultClicked(QTableWidgetItem *item)
{
    qDebug() << "Entered";

    if (m_scraperId == "custom-movie" || !m_customScraperIds.isEmpty()) {
        ui->comboScraper->setEnabled(false);
        ui->groupBox->setEnabled(false);

        if (m_currentCustomScraper == CustomMovieScraper::instance()->titleScraper())
            m_customScraperIds.clear();

        m_customScraperIds.insert(m_currentCustomScraper, item->data(Qt::UserRole).toString());
        QList<ScraperInterface*> scrapers = CustomMovieScraper::instance()->scrapersNeedSearch(infosToLoad(), m_customScraperIds);
        if (scrapers.isEmpty()) {
            m_scraperId = "custom-movie";
            accept();
        } else {
            m_currentCustomScraper = scrapers.first();
            for (int i=0, n=ui->comboScraper->count() ; i<n ; ++i) {
                if (ui->comboScraper->itemData(i, Qt::UserRole).toString() == m_currentCustomScraper->identifier()) {
                    ui->comboScraper->setCurrentIndex(i);
                    break;
                }
            }
        }
    } else {
        m_scraperMovieId = item->data(Qt::UserRole).toString();
        m_customScraperIds.clear();
        accept();
    }
}

/**
 * @brief Called when one of the checkboxes are clicked.
 *        Clears m_infosToLoad and fills it with the items which are checked.
 *        If all enabled items are checked, "check all" checkbox is checked too
 */
void MovieSearch::chkToggled()
{
    m_infosToLoad.clear();
    bool allToggled = true;
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isChecked() && box->myData().toInt() > 0)
            m_infosToLoad.append(box->myData().toInt());
        if (!box->isChecked() && box->myData().toInt() > 0 && box->isEnabled())
            allToggled = false;
    }
    ui->chkUnCheckAll->setChecked(allToggled);

    QString scraperId = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toString();
    Settings::instance()->setScraperInfos(WidgetMovies, scraperId, m_infosToLoad);
}

/**
 * @brief Called when "check all" checkbox is clicked
 *        Toggles all other boxes
 * @param toggled Checkbox is toggled
 */
void MovieSearch::chkAllToggled(bool toggled)
{
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0 && box->isEnabled())
            box->setChecked(toggled);
    }
    chkToggled();
}

/*** GETTER ***/

/**
 * @brief MovieSearch::scraperId
 * @return Current scraper Id
 */
QString MovieSearch::scraperId()
{
    return m_scraperId;
}

/**
 * @brief MovieSearch::scraperId
 * @return Scraper id of the movie last clicked in result table
 */
QString MovieSearch::scraperMovieId()
{
    return m_scraperMovieId;
}

/**
 * @brief MovieSearch::infosToLoad
 * @return List of infos to load from the scraper
 */
QList<int> MovieSearch::infosToLoad()
{
    return m_infosToLoad;
}

/**
 * @brief Sets status of checkboxes based on the support of the scraper
 * @param scraperSupports List of infos supported by the scraper
 */
void MovieSearch::setChkBoxesEnabled(QList<int> scraperSupports)
{
    QString scraperId = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toString();
    QList<int> infos = Settings::instance()->scraperInfos(WidgetMovies, scraperId);

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        box->setEnabled(scraperSupports.contains(box->myData().toInt()));
        box->setChecked((infos.contains(box->myData().toInt()) || infos.isEmpty()) && scraperSupports.contains(box->myData().toInt()));
    }
    chkToggled();
}

QMap<ScraperInterface*, QString> MovieSearch::customScraperIds()
{
    return m_customScraperIds;
}
