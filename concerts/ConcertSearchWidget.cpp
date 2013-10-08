#include "ConcertSearchWidget.h"
#include "ui_ConcertSearchWidget.h"

#include <QDebug>
#include "globals/Manager.h"

ConcertSearchWidget::ConcertSearchWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConcertSearchWidget)
{
    ui->setupUi(this);

#if QT_VERSION >= 0x050000
    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    ui->results->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    ui->searchString->setType(MyLineEdit::TypeLoading);

    foreach (ConcertScraperInterface *scraper, Manager::instance()->concertScrapers()) {
        ui->comboScraper->addItem(scraper->name(), Manager::instance()->concertScrapers().indexOf(scraper));
        connect(scraper, SIGNAL(searchDone(QList<ScraperSearchResult>)), this, SLOT(showResults(QList<ScraperSearchResult>)));
    }

    connect(ui->comboScraper, SIGNAL(currentIndexChanged(int)), this, SLOT(search()));
    connect(ui->searchString, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(ui->results, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(resultClicked(QTableWidgetItem*)));

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

ConcertSearchWidget::~ConcertSearchWidget()
{
    delete ui;
}

void ConcertSearchWidget::search(QString searchString)
{
    ui->searchString->setText(searchString);
    search();
}

void ConcertSearchWidget::clear()
{
    qDebug() << "Entered";
    ui->results->clearContents();
    ui->results->setRowCount(0);
}

void ConcertSearchWidget::search()
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

void ConcertSearchWidget::showResults(QList<ScraperSearchResult> results)
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

void ConcertSearchWidget::resultClicked(QTableWidgetItem *item)
{
    qDebug() << "Entered";
    m_scraperId = item->data(Qt::UserRole).toString();
    emit sigResultClicked();
}

void ConcertSearchWidget::chkToggled()
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
    Settings::instance()->setScraperInfos(WidgetConcerts, QString::number(scraperNo), m_infosToLoad);
}

void ConcertSearchWidget::chkAllToggled(bool toggled)
{
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0 && box->isEnabled())
            box->setChecked(toggled);
    }
    chkToggled();
}

int ConcertSearchWidget::scraperNo()
{
    qDebug() << "Entered, m_scraperNo=" << m_scraperNo;
    return m_scraperNo;
}

QString ConcertSearchWidget::scraperId()
{
    qDebug() << "Entered, m_scraperId=" << m_scraperId;
    return m_scraperId;
}

QList<int> ConcertSearchWidget::infosToLoad()
{
    return m_infosToLoad;
}

void ConcertSearchWidget::setChkBoxesEnabled(QList<int> scraperSupports)
{
    int scraperNo = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toInt();
    QList<int> infos = Settings::instance()->scraperInfos(WidgetConcerts, QString::number(scraperNo));

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        box->setEnabled(scraperSupports.contains(box->myData().toInt()));
        box->setChecked((infos.contains(box->myData().toInt()) || infos.isEmpty()) && scraperSupports.contains(box->myData().toInt()));
    }
    chkToggled();
}
