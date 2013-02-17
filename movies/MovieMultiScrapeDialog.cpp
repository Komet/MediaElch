#include "MovieMultiScrapeDialog.h"
#include "ui_MovieMultiScrapeDialog.h"

#include "globals/Manager.h"
#include "smallWidgets/MyCheckBox.h"

MovieMultiScrapeDialog::MovieMultiScrapeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MovieMultiScrapeDialog)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    QFont font = ui->movieCounter->font();
#ifdef Q_OS_WIN32
    font.setPointSize(font.pointSize()-1);
#else
    font.setPointSize(font.pointSize()-2);
#endif
    ui->movieCounter->setFont(font);

    m_executed = false;
    m_currentMovie = 0;

    ui->chkActors->setMyData(MovieScraperInfos::Actors);
    ui->chkBackdrop->setMyData(MovieScraperInfos::Backdrop);
    ui->chkCertification->setMyData(MovieScraperInfos::Certification);
    ui->chkCountries->setMyData(MovieScraperInfos::Countries);
    ui->chkDirector->setMyData(MovieScraperInfos::Director);
    ui->chkExtraArts->setMyData(MovieScraperInfos::ExtraArts);
    ui->chkGenres->setMyData(MovieScraperInfos::Genres);
    ui->chkOverview->setMyData(MovieScraperInfos::Overview);
    ui->chkPoster->setMyData(MovieScraperInfos::Poster);
    ui->chkRating->setMyData(MovieScraperInfos::Rating);
    ui->chkReleased->setMyData(MovieScraperInfos::Released);
    ui->chkRuntime->setMyData(MovieScraperInfos::Runtime);
    ui->chkStudios->setMyData(MovieScraperInfos::Studios);
    ui->chkTagline->setMyData(MovieScraperInfos::Tagline);
    ui->chkTitle->setMyData(MovieScraperInfos::Title);
    ui->chkTrailer->setMyData(MovieScraperInfos::Trailer);
    ui->chkWriter->setMyData(MovieScraperInfos::Writer);

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0)
            connect(box, SIGNAL(clicked()), this, SLOT(onChkToggled()));
    }
    for (int i=0, n=Manager::instance()->scrapers().count() ; i<n ; ++i)
        ui->comboScraper->addItem(Manager::instance()->scrapers().at(i)->name(), i);

    connect(ui->chkUnCheckAll, SIGNAL(clicked()), this, SLOT(onChkAllToggled()));
    connect(ui->btnStartScraping, SIGNAL(clicked()), this, SLOT(onStartScraping()));
    connect(ui->comboScraper, SIGNAL(currentIndexChanged(int)), this, SLOT(setChkBoxesEnabled()));
}

MovieMultiScrapeDialog::~MovieMultiScrapeDialog()
{
    delete ui;
}

MovieMultiScrapeDialog *MovieMultiScrapeDialog::instance(QWidget *parent)
{
    static MovieMultiScrapeDialog *m_instance = 0;
    if (m_instance == 0)
        m_instance = new MovieMultiScrapeDialog(parent);
    return m_instance;
}

int MovieMultiScrapeDialog::exec()
{
    m_queue.clear();
    ui->movieCounter->setVisible(false);
    ui->comboScraper->setEnabled(true);
    ui->btnCancel->setVisible(true);
    ui->btnClose->setVisible(false);
    ui->btnStartScraping->setVisible(true);
    ui->btnStartScraping->setEnabled(true);
    ui->chkAutoSave->setEnabled(true);
    ui->progressAll->setValue(0);
    ui->progressMovie->setValue(0);
    ui->groupBox->setEnabled(true);
    ui->movie->clear();
    m_currentMovie = 0;
    m_executed = true;
    setChkBoxesEnabled();
    adjustSize();
    return QDialog::exec();
}

void MovieMultiScrapeDialog::accept()
{
    m_executed = false;
    QDialog::accept();
}

void MovieMultiScrapeDialog::reject()
{
    m_executed = false;
    if (m_currentMovie) {
        m_queue.clear();
        m_currentMovie->controller()->abortDownloads();
    }
    QDialog::reject();
}

void MovieMultiScrapeDialog::setMovies(QList<Movie *> movies)
{
    m_movies = movies;
}

void MovieMultiScrapeDialog::onStartScraping()
{
    ui->groupBox->setEnabled(false);
    ui->comboScraper->setEnabled(false);
    ui->btnStartScraping->setEnabled(false);
    ui->chkAutoSave->setEnabled(false);

    m_scraperInterface = Manager::instance()->scrapers().at(ui->comboScraper->itemData(ui->comboScraper->currentIndex()).toInt());
    m_isTmdb = m_scraperInterface->name() == "The Movie DB";
    m_isImdb = m_scraperInterface->name() == "IMDB";

    connect(m_scraperInterface, SIGNAL(searchDone(QList<ScraperSearchResult>)), this, SLOT(onSearchFinished(QList<ScraperSearchResult>)), Qt::UniqueConnection);

    m_queue.append(m_movies);

    ui->movieCounter->setText(QString("0/%1").arg(m_queue.count()));
    ui->movieCounter->setVisible(true);
    ui->progressAll->setMaximum(m_movies.count());
    scrapeNext();
}

void MovieMultiScrapeDialog::onScrapingFinished()
{
    ui->movieCounter->setVisible(false);
    ui->movie->setText(tr("Scraping of %1 movies has finished.").arg(m_movies.count()));
    ui->progressAll->setValue(ui->progressAll->maximum());
    ui->btnCancel->setVisible(false);
    ui->btnClose->setVisible(true);
    ui->btnStartScraping->setVisible(false);
}

void MovieMultiScrapeDialog::scrapeNext()
{
    if (!isExecuted())
        return;

    if (m_queue.isEmpty()) {
        onScrapingFinished();
        return;
    }

    if (m_currentMovie && ui->chkAutoSave->isChecked()) {
        m_currentMovie->controller()->saveData(Manager::instance()->mediaCenterInterface());
    }

    m_currentMovie = m_queue.dequeue();
    ui->movie->setText(m_currentMovie->name());
    ui->movieCounter->setText(QString("%1/%2").arg(m_movies.count()-m_queue.count()).arg(m_movies.count()));

    ui->progressAll->setValue(ui->progressAll->maximum()-m_queue.size()-1);
    ui->progressMovie->setValue(0);

    connect(m_currentMovie->controller(), SIGNAL(sigLoadDone(Movie*)), this, SLOT(scrapeNext()), Qt::UniqueConnection);
    connect(m_currentMovie->controller(), SIGNAL(sigDownloadProgress(Movie*,int,int)), this, SLOT(onProgress(Movie*,int,int)), Qt::UniqueConnection);

    if (m_isImdb && !m_currentMovie->id().isEmpty())
        loadMovieData(m_currentMovie, m_currentMovie->id());
    else if (m_isTmdb && !m_currentMovie->tmdbId().isEmpty())
        loadMovieData(m_currentMovie, m_currentMovie->tmdbId());
    else if (m_isTmdb && !m_currentMovie->id().isEmpty())
        loadMovieData(m_currentMovie, m_currentMovie->id());
    else
        m_scraperInterface->search(m_currentMovie->name());
}

void MovieMultiScrapeDialog::loadMovieData(Movie *movie, QString id)
{
    movie->controller()->loadData(id, m_scraperInterface, m_infosToLoad);
}

void MovieMultiScrapeDialog::onSearchFinished(QList<ScraperSearchResult> results)
{
    if (!isExecuted())
        return;
    if (results.isEmpty()) {
        scrapeNext();
        return;
    }
    m_currentMovie->controller()->loadData(results.first().id, m_scraperInterface, m_infosToLoad);
}

void MovieMultiScrapeDialog::onProgress(Movie *movie, int current, int maximum)
{
    Q_UNUSED(movie);

    if (!isExecuted())
        return;
    ui->progressMovie->setValue(maximum-current);
    ui->progressMovie->setMaximum(maximum);
}

bool MovieMultiScrapeDialog::isExecuted()
{
    return m_executed;
}

void MovieMultiScrapeDialog::onChkToggled()
{
    m_infosToLoad.clear();
    bool allToggled = true;
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isChecked() && box->myData().toInt() > 0)
            m_infosToLoad.append(box->myData().toInt());
        if (!box->isChecked() && box->myData().toInt() > 0)
            allToggled = false;
    }

    ui->chkUnCheckAll->setChecked(allToggled);

    int scraperNo = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toInt();
    Settings::instance()->setScraperInfos(WidgetMovies, scraperNo, m_infosToLoad);

    ui->btnStartScraping->setEnabled(!m_infosToLoad.isEmpty());
}

void MovieMultiScrapeDialog::onChkAllToggled()
{
    bool checked = ui->chkUnCheckAll->isChecked();
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0)
            box->setChecked(checked);
    }
    onChkToggled();
}

void MovieMultiScrapeDialog::setChkBoxesEnabled()
{
    int scraperNo = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toInt();
    QList<int> scraperSupports = Manager::instance()->scrapers().at(scraperNo)->scraperSupports();
    QList<int> infos = Settings::instance()->scraperInfos(WidgetMovies, scraperNo);

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        box->setEnabled(scraperSupports.contains(box->myData().toInt()));
        box->setChecked((infos.contains(box->myData().toInt()) || infos.isEmpty()) && scraperSupports.contains(box->myData().toInt()));
    }
    onChkToggled();
}
