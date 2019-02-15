#include "ConcertSearchWidget.h"
#include "ui_ConcertSearchWidget.h"

#include <QDebug>

#include "globals/Manager.h"

ConcertSearchWidget::ConcertSearchWidget(QWidget* parent) : QWidget(parent), ui(new Ui::ConcertSearchWidget)
{
    ui->setupUi(this);

    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

    for (ConcertScraperInterface* scraper : Manager::instance()->concertScrapers()) {
        ui->comboScraper->addItem(scraper->name(), Manager::instance()->concertScrapers().indexOf(scraper));
        connect(scraper,
            SIGNAL(searchDone(QVector<ScraperSearchResult>)),
            this,
            SLOT(showResults(QVector<ScraperSearchResult>)));
    }

    connect(ui->comboScraper, SIGNAL(currentIndexChanged(int)), this, SLOT(search()));
    connect(ui->searchString, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(ui->results, &QTableWidget::itemClicked, this, &ConcertSearchWidget::resultClicked);

    ui->chkBackdrop->setMyData(static_cast<int>(ConcertScraperInfos::Backdrop));
    ui->chkCertification->setMyData(static_cast<int>(ConcertScraperInfos::Certification));
    ui->chkExtraArts->setMyData(static_cast<int>(ConcertScraperInfos::ExtraArts));
    ui->chkGenres->setMyData(static_cast<int>(ConcertScraperInfos::Genres));
    ui->chkOverview->setMyData(static_cast<int>(ConcertScraperInfos::Overview));
    ui->chkPoster->setMyData(static_cast<int>(ConcertScraperInfos::Poster));
    ui->chkRating->setMyData(static_cast<int>(ConcertScraperInfos::Rating));
    ui->chkReleased->setMyData(static_cast<int>(ConcertScraperInfos::Released));
    ui->chkRuntime->setMyData(static_cast<int>(ConcertScraperInfos::Runtime));
    ui->chkTagline->setMyData(static_cast<int>(ConcertScraperInfos::Tagline));
    ui->chkTitle->setMyData(static_cast<int>(ConcertScraperInfos::Title));
    ui->chkTrailer->setMyData(static_cast<int>(ConcertScraperInfos::Trailer));

    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            connect(box, &QAbstractButton::clicked, this, &ConcertSearchWidget::chkToggled);
        }
    }
    connect(ui->chkUnCheckAll, &QAbstractButton::clicked, this, &ConcertSearchWidget::chkAllToggled);
}

ConcertSearchWidget::~ConcertSearchWidget()
{
    delete ui;
}

void ConcertSearchWidget::search(QString searchString)
{
    ui->searchString->setText(searchString.replace(".", " ").trimmed());
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
    setCheckBoxesEnabled(Manager::instance()->concertScrapers().at(m_scraperNo)->scraperSupports());
    clear();
    ui->comboScraper->setEnabled(false);
    ui->searchString->setLoading(true);
    Manager::instance()->concertScrapers().at(m_scraperNo)->search(ui->searchString->text().trimmed());
}

void ConcertSearchWidget::showResults(QVector<ScraperSearchResult> results)
{
    qDebug() << "Entered, size of results=" << results.count();
    ui->comboScraper->setEnabled(true);
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();
    for (const ScraperSearchResult& result : results) {
        QString name = result.name;
        if (result.released.isValid()) {
            name.append(QString(" (%1)").arg(result.released.toString("yyyy")));
        }
        auto item = new QTableWidgetItem(name);
        item->setData(Qt::UserRole, result.id);
        int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
    }
}

void ConcertSearchWidget::resultClicked(QTableWidgetItem* item)
{
    qDebug() << "Entered";
    m_scraperId = TmdbId(item->data(Qt::UserRole).toString());
    emit sigResultClicked();
}

void ConcertSearchWidget::chkToggled()
{
    m_infosToLoad.clear();
    bool allToggled = true;
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isChecked() && box->myData().toInt() > 0 && box->isEnabled()) {
            m_infosToLoad.append(ConcertScraperInfos(box->myData().toInt()));
        }
        if (!box->isChecked() && box->myData().toInt() > 0 && box->isEnabled()) {
            allToggled = false;
        }
    }
    ui->chkUnCheckAll->setChecked(allToggled);

    int scraperNo = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toInt();
    Settings::instance()->setScraperInfos(MainWidgets::Concerts, QString::number(scraperNo), m_infosToLoad);
}

void ConcertSearchWidget::chkAllToggled(bool toggled)
{
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0 && box->isEnabled()) {
            box->setChecked(toggled);
        }
    }
    chkToggled();
}

int ConcertSearchWidget::scraperNo()
{
    qDebug() << "Entered, m_scraperNo=" << m_scraperNo;
    return m_scraperNo;
}

TmdbId ConcertSearchWidget::scraperId()
{
    qDebug() << "Entered, m_scraperId=" << m_scraperId;
    return m_scraperId;
}

QVector<ConcertScraperInfos> ConcertSearchWidget::infosToLoad()
{
    return m_infosToLoad;
}

void ConcertSearchWidget::setCheckBoxesEnabled(QVector<ConcertScraperInfos> scraperSupports)
{
    int scraperNo = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toInt();
    QVector<ConcertScraperInfos> infos =
        Settings::instance()->scraperInfos<ConcertScraperInfos>(QString::number(scraperNo));

    for (auto box : ui->groupBox->findChildren<MyCheckBox*>()) {
        box->setEnabled(scraperSupports.contains(ConcertScraperInfos(box->myData().toInt())));
        box->setChecked((infos.contains(ConcertScraperInfos(box->myData().toInt())) || infos.isEmpty())
                        && scraperSupports.contains(ConcertScraperInfos(box->myData().toInt())));
    }
    chkToggled();
}
