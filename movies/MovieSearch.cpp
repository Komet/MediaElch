#include "MovieSearch.h"
#include "ui_MovieSearch.h"

#include <QDebug>
#include "globals/Manager.h"

/**
 * @brief MovieSearch::MovieSearch
 * @param parent
 */
MovieSearch::MovieSearch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MovieSearch)
{
    ui->setupUi(this);
    ui->results->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

#ifdef Q_WS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #MovieSearch { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        ui->comboScraper->addItem(scraper->name(), Manager::instance()->scrapers().indexOf(scraper));
        connect(scraper, SIGNAL(searchDone(QList<ScraperSearchResult>)), this, SLOT(showResults(QList<ScraperSearchResult>)));
    }

    connect(ui->comboScraper, SIGNAL(currentIndexChanged(int)), this, SLOT(search()));
    connect(ui->searchString, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(ui->results, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(resultClicked(QTableWidgetItem*)));
    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(reject()));

    connect(ui->chkActors, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkBackdrop, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkCertification, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkCountries, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkGenres, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkOverview, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkPoster, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkRating, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkReleased, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkRuntime, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkStudios, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkTagline, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkTitle, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkTrailer, SIGNAL(clicked()), this, SLOT(chkToggled()));
    connect(ui->chkUnCheckAll, SIGNAL(clicked(bool)), this, SLOT(chkAllToggled(bool)));

    ui->chkUnCheckAll->setChecked(true);
    chkAllToggled(true);
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
    m_scraperNo = ui->comboScraper->itemData(index, Qt::UserRole).toInt();
    setChkBoxesEnabled(Manager::instance()->scrapers().at(m_scraperNo)->scraperSupports());
    clear();
    ui->comboScraper->setEnabled(false);
    ui->searchString->setLoading(true);
    Manager::instance()->scrapers().at(m_scraperNo)->search(ui->searchString->text());
}

/**
 * @brief Fills the result table with results
 * @param results List of results
 */
void MovieSearch::showResults(QList<ScraperSearchResult> results)
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
void MovieSearch::resultClicked(QTableWidgetItem *item)
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
void MovieSearch::chkToggled()
{
    m_infosToLoad.clear();
    int numOfScraperSupports = 0;

    if (ui->chkActors->isChecked())
        m_infosToLoad.append(MovieScraperInfos::Actors);
    if (ui->chkBackdrop->isChecked())
        m_infosToLoad.append(MovieScraperInfos::Backdrop);
    if (ui->chkCertification->isChecked())
        m_infosToLoad.append(MovieScraperInfos::Certification);
    if (ui->chkCountries->isChecked())
        m_infosToLoad.append(MovieScraperInfos::Countries);
    if (ui->chkGenres->isChecked())
        m_infosToLoad.append(MovieScraperInfos::Genres);
    if (ui->chkOverview->isChecked())
        m_infosToLoad.append(MovieScraperInfos::Overview);
    if (ui->chkPoster->isChecked())
        m_infosToLoad.append(MovieScraperInfos::Poster);
    if (ui->chkRating->isChecked())
        m_infosToLoad.append(MovieScraperInfos::Rating);
    if (ui->chkReleased->isChecked())
        m_infosToLoad.append(MovieScraperInfos::Released);
    if (ui->chkRuntime->isChecked())
        m_infosToLoad.append(MovieScraperInfos::Runtime);
    if (ui->chkStudios->isChecked())
        m_infosToLoad.append(MovieScraperInfos::Studios);
    if (ui->chkTagline->isChecked())
        m_infosToLoad.append(MovieScraperInfos::Tagline);
    if (ui->chkTitle->isChecked())
        m_infosToLoad.append(MovieScraperInfos::Title);
    if (ui->chkTrailer->isChecked())
        m_infosToLoad.append(MovieScraperInfos::Trailer);

    if (ui->chkActors->isEnabled())
        numOfScraperSupports++;
    if (ui->chkBackdrop->isEnabled())
        numOfScraperSupports++;
    if (ui->chkCertification->isEnabled())
        numOfScraperSupports++;
    if (ui->chkCountries->isEnabled())
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
    if (ui->chkStudios->isEnabled())
        numOfScraperSupports++;
    if (ui->chkTagline->isEnabled())
        numOfScraperSupports++;
    if (ui->chkTitle->isEnabled())
        numOfScraperSupports++;
    if (ui->chkTrailer->isEnabled())
        numOfScraperSupports++;

    ui->chkUnCheckAll->setChecked(m_infosToLoad.size() == numOfScraperSupports);
}

/**
 * @brief Called when "check all" checkbox is clicked
 *        Toggles all other boxes
 * @param toggled Checkbox is toggled
 */
void MovieSearch::chkAllToggled(bool toggled)
{
    bool isChecked = toggled;
    if (ui->chkActors->isEnabled())
        ui->chkActors->setChecked(isChecked);
    if (ui->chkBackdrop->isEnabled())
        ui->chkBackdrop->setChecked(isChecked);
    if (ui->chkCertification->isEnabled())
        ui->chkCertification->setChecked(isChecked);
    if (ui->chkCountries->isEnabled())
        ui->chkCountries->setChecked(isChecked);
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
    if (ui->chkStudios->isEnabled())
        ui->chkStudios->setChecked(isChecked);
    if (ui->chkTagline->isEnabled())
        ui->chkTagline->setChecked(isChecked);
    if (ui->chkTitle->isEnabled())
        ui->chkTitle->setChecked(isChecked);
    if (ui->chkTrailer->isEnabled())
        ui->chkTrailer->setChecked(isChecked);
    chkToggled();
}

/*** GETTER ***/

/**
 * @brief MovieSearch::scraperNo
 * @return Current scraper number
 */
int MovieSearch::scraperNo()
{
    qDebug() << "Entered, m_scraperNo=" << m_scraperNo;
    return m_scraperNo;
}

/**
 * @brief MovieSearch::scraperId
 * @return Scraper id of the movie last clicked in result table
 */
QString MovieSearch::scraperId()
{
    qDebug() << "Entered, m_scraperId=" << m_scraperId;
    return m_scraperId;
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
    ui->chkActors->setEnabled(scraperSupports.contains(MovieScraperInfos::Actors));
    ui->chkBackdrop->setEnabled(scraperSupports.contains(MovieScraperInfos::Backdrop));
    ui->chkCertification->setEnabled(scraperSupports.contains(MovieScraperInfos::Certification));
    ui->chkCountries->setEnabled(scraperSupports.contains(MovieScraperInfos::Countries));
    ui->chkGenres->setEnabled(scraperSupports.contains(MovieScraperInfos::Genres));
    ui->chkOverview->setEnabled(scraperSupports.contains(MovieScraperInfos::Overview));
    ui->chkPoster->setEnabled(scraperSupports.contains(MovieScraperInfos::Poster));
    ui->chkRating->setEnabled(scraperSupports.contains(MovieScraperInfos::Rating));
    ui->chkReleased->setEnabled(scraperSupports.contains(MovieScraperInfos::Released));
    ui->chkRuntime->setEnabled(scraperSupports.contains(MovieScraperInfos::Runtime));
    ui->chkStudios->setEnabled(scraperSupports.contains(MovieScraperInfos::Studios));
    ui->chkTagline->setEnabled(scraperSupports.contains(MovieScraperInfos::Tagline));
    ui->chkTitle->setEnabled(scraperSupports.contains(MovieScraperInfos::Title));
    ui->chkTrailer->setEnabled(scraperSupports.contains(MovieScraperInfos::Trailer));

    if (!scraperSupports.contains(MovieScraperInfos::Actors))
        ui->chkActors->setChecked(false);
    if (!scraperSupports.contains(MovieScraperInfos::Backdrop))
        ui->chkBackdrop->setChecked(false);
    if (!scraperSupports.contains(MovieScraperInfos::Certification))
        ui->chkCertification->setChecked(false);
    if (!scraperSupports.contains(MovieScraperInfos::Countries))
        ui->chkCountries->setChecked(false);
    if (!scraperSupports.contains(MovieScraperInfos::Genres))
        ui->chkGenres->setChecked(false);
    if (!scraperSupports.contains(MovieScraperInfos::Overview))
        ui->chkOverview->setChecked(false);
    if (!scraperSupports.contains(MovieScraperInfos::Poster))
        ui->chkPoster->setChecked(false);
    if (!scraperSupports.contains(MovieScraperInfos::Rating))
        ui->chkRating->setChecked(false);
    if (!scraperSupports.contains(MovieScraperInfos::Released))
        ui->chkReleased->setChecked(false);
    if (!scraperSupports.contains(MovieScraperInfos::Runtime))
        ui->chkRuntime->setChecked(false);
    if (!scraperSupports.contains(MovieScraperInfos::Studios))
        ui->chkStudios->setChecked(false);
    if (!scraperSupports.contains(MovieScraperInfos::Tagline))
        ui->chkTagline->setChecked(false);
    if (!scraperSupports.contains(MovieScraperInfos::Title))
        ui->chkTitle->setChecked(false);
    if (!scraperSupports.contains(MovieScraperInfos::Trailer))
        ui->chkTrailer->setChecked(false);

    chkToggled();
}
