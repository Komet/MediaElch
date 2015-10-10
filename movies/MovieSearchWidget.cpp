#include "MovieSearchWidget.h"
#include "ui_MovieSearchWidget.h"

#include <QDebug>
#include "globals/Manager.h"
#include "scrapers/CustomMovieScraper.h"


QDebug operator<<(QDebug lhs, const ScraperSearchResult &rhs)
{
    lhs << QString("(\"%1\", \"%2\", %3)").arg(rhs.id)
                                          .arg(rhs.name)
                                          .arg(rhs.released.toString("yyyy"));
    return lhs;
}


MovieSearchWidget::MovieSearchWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MovieSearchWidget)
{
    ui->setupUi(this);
    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

    foreach (ScraperInterface *scraper, Manager::instance()->scrapers())
        connect(scraper, SIGNAL(searchDone(QList<ScraperSearchResult>)), this, SLOT(showResults(QList<ScraperSearchResult>)));
    setupScrapers();

    connect(ui->comboScraper, SIGNAL(currentIndexChanged(int)), this, SLOT(onUpdateSearchString()), Qt::QueuedConnection);
    connect(ui->comboScraper, SIGNAL(currentIndexChanged(int)), this, SLOT(search()), Qt::QueuedConnection);
    connect(ui->searchString, SIGNAL(textEdited(QString)), this, SLOT(onStoreSearchString(QString)));
    connect(ui->searchString, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(ui->results, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(resultClicked(QTableWidgetItem*)));

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
    ui->chkTags->setMyData(MovieScraperInfos::Tags);

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0)
            connect(box, SIGNAL(clicked()), this, SLOT(chkToggled()));
    }
    connect(ui->chkUnCheckAll, SIGNAL(clicked(bool)), this, SLOT(chkAllToggled(bool)));
}

MovieSearchWidget::~MovieSearchWidget()
{
    delete ui;
}

void MovieSearchWidget::clear()
{
    qDebug() << "Entered";
    ui->results->clearContents();
    ui->results->setRowCount(0);
}

void MovieSearchWidget::setupScrapers()
{
    ui->comboScraper->blockSignals(true);
    ui->comboScraper->clear();
    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (!Settings::instance()->showAdultScrapers() && scraper->isAdult())
            continue;
        if (scraper->isAdult())
            ui->comboScraper->addItem(QIcon(":/img/heart_red_open.png"), scraper->name(), scraper->identifier());
        else
            ui->comboScraper->addItem(scraper->name(), scraper->identifier());
    }
    ui->comboScraper->setCurrentIndex((Settings::instance()->currentMovieScraper() < ui->comboScraper->count()) ? Settings::instance()->currentMovieScraper() : 0);
    ui->comboScraper->blockSignals(false);
}

void MovieSearchWidget::search(QString searchString, QString id, QString tmdbId)
{
    setupScrapers();
    m_searchString = searchString.replace(".", " ");

    m_id = id;
    m_tmdbId = tmdbId;
    ui->comboScraper->setEnabled(true);
    ui->groupBox->setEnabled(true);
    m_currentCustomScraper = 0;
    m_customScraperIds.clear();

    int index = ui->comboScraper->currentIndex();
    if (index < 0 || index >= Manager::instance()->scrapers().size()) {
        return;
    }
    m_scraperId = ui->comboScraper->itemData(index, Qt::UserRole).toString();
    ScraperInterface *scraper = Manager::instance()->scraper(m_scraperId);
    if (!scraper)
        return;

    if (scraper->identifier() == "imdb" && !m_id.isEmpty())
        ui->searchString->setText(m_id);
    else if (scraper->identifier() == "tmdb" && !m_tmdbId.isEmpty() && !m_tmdbId.startsWith("tt"))
        ui->searchString->setText("id" + m_tmdbId);
    else if (scraper->identifier() == "tmdb" && !m_tmdbId.isEmpty())
        ui->searchString->setText(m_tmdbId);
    else if (scraper->identifier() == "tmdb" && !m_id.isEmpty())
        ui->searchString->setText(m_id);
    else
        ui->searchString->setText(m_searchString);

    search();
}

void MovieSearchWidget::search()
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
    Settings::instance()->setCurrentMovieScraper(ui->comboScraper->currentIndex());
}

void MovieSearchWidget::showResults(QList<ScraperSearchResult> results)
{
    qDebug() << "results.count(): " << results.count() << ", results:" << results;

    ui->comboScraper->setEnabled(m_customScraperIds.isEmpty());
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();
    foreach (const ScraperSearchResult &result, results) {
        QTableWidgetItem *item;
        if (!result.released.isNull())
            item = new QTableWidgetItem(QString("%1 (%2)").arg(result.name).arg(result.released.toString("yyyy")));
        else
            item = new QTableWidgetItem(QString("%1").arg(result.name));
        item->setData(Qt::UserRole, result.id);
        int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
    }
}

void MovieSearchWidget::resultClicked(QTableWidgetItem *item)
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
            emit sigResultClicked();
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
        emit sigResultClicked();
    }
}

void MovieSearchWidget::chkToggled()
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

void MovieSearchWidget::chkAllToggled(bool toggled)
{
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0 && box->isEnabled())
            box->setChecked(toggled);
    }
    chkToggled();
}

QString MovieSearchWidget::scraperId()
{
    return m_scraperId;
}

QString MovieSearchWidget::scraperMovieId()
{
    return m_scraperMovieId;
}

QList<int> MovieSearchWidget::infosToLoad()
{
    return m_infosToLoad;
}

void MovieSearchWidget::setChkBoxesEnabled(QList<int> scraperSupports)
{
    QString scraperId = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toString();
    QList<int> infos = Settings::instance()->scraperInfos(WidgetMovies, scraperId);

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        box->setEnabled(scraperSupports.contains(box->myData().toInt()));
        box->setChecked((infos.contains(box->myData().toInt()) || infos.isEmpty()) && scraperSupports.contains(box->myData().toInt()));
    }
    chkToggled();
}

QMap<ScraperInterface*, QString> MovieSearchWidget::customScraperIds()
{
    return m_customScraperIds;
}

void MovieSearchWidget::onUpdateSearchString()
{
    int index = ui->comboScraper->currentIndex();
    if (index < 0 || index >= Manager::instance()->scrapers().size()) {
        return;
    }
    m_scraperId = ui->comboScraper->itemData(index, Qt::UserRole).toString();
    ScraperInterface *scraper = Manager::instance()->scraper(m_scraperId);
    if (!scraper)
        return;

    if (scraper->identifier() == "imdb" && !m_id.isEmpty())
        ui->searchString->setText(m_id);
    else if (scraper->identifier() == "tmdb" && !m_tmdbId.isEmpty())
        ui->searchString->setText("id" + m_tmdbId);
    else if (scraper->identifier() == "tmdb" && !m_id.isEmpty())
        ui->searchString->setText(m_id);
    else
        ui->searchString->setText(m_searchString);
}

void MovieSearchWidget::onStoreSearchString(QString searchString)
{
    m_searchString = searchString;
}
