#include "MusicSearchWidget.h"
#include "ui_MusicSearchWidget.h"

#include "globals/Manager.h"
#include "scrapers/music/MusicScraperInterface.h"
#include "ui/small_widgets/MyLabel.h"
#include <QDebug>

MusicSearchWidget::MusicSearchWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MusicSearchWidget)
{
    ui->setupUi(this);

    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

    for (MusicScraperInterface* scraper : Manager::instance()->musicScrapers()) {
        ui->comboScraper->addItem(scraper->name(), Manager::instance()->musicScrapers().indexOf(scraper));
        connect(scraper, &MusicScraperInterface::sigSearchDone, this, &MusicSearchWidget::showResults);
    }

    connect(ui->comboScraper, SIGNAL(currentIndexChanged(int)), this, SLOT(search()));
    connect(ui->searchString, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(ui->results, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(resultClicked(QTableWidgetItem*)));

    ui->chkName->setMyData(static_cast<int>(MusicScraperInfos::Name));
    ui->chkBorn->setMyData(static_cast<int>(MusicScraperInfos::Born));
    ui->chkFormed->setMyData(static_cast<int>(MusicScraperInfos::Formed));
    ui->chkYearsActive->setMyData(static_cast<int>(MusicScraperInfos::YearsActive));
    ui->chkDisbanded->setMyData(static_cast<int>(MusicScraperInfos::Disbanded));
    ui->chkDied->setMyData(static_cast<int>(MusicScraperInfos::Died));
    ui->chkBiography->setMyData(static_cast<int>(MusicScraperInfos::Biography));
    ui->chkArtist->setMyData(static_cast<int>(MusicScraperInfos::Artist));
    ui->chkLabel->setMyData(static_cast<int>(MusicScraperInfos::Label));
    ui->chkReview->setMyData(static_cast<int>(MusicScraperInfos::Review));
    ui->chkYear->setMyData(static_cast<int>(MusicScraperInfos::Year));
    ui->chkRating->setMyData(static_cast<int>(MusicScraperInfos::Rating));
    ui->chkReleaseDate->setMyData(static_cast<int>(MusicScraperInfos::ReleaseDate));
    ui->chkTitle->setMyData(static_cast<int>(MusicScraperInfos::Title));
    ui->chkGenres->setMyData(static_cast<int>(MusicScraperInfos::Genres));
    ui->chkStyles->setMyData(static_cast<int>(MusicScraperInfos::Styles));
    ui->chkMoods->setMyData(static_cast<int>(MusicScraperInfos::Moods));
    ui->chkThumb->setMyData(static_cast<int>(MusicScraperInfos::Thumb));
    ui->chkFanart->setMyData(static_cast<int>(MusicScraperInfos::Fanart));
    ui->chkExtraFanarts->setMyData(static_cast<int>(MusicScraperInfos::ExtraFanarts));
    ui->chkLogo->setMyData(static_cast<int>(MusicScraperInfos::Logo));
    ui->chkCover->setMyData(static_cast<int>(MusicScraperInfos::Cover));
    ui->chkCdArt->setMyData(static_cast<int>(MusicScraperInfos::CdArt));
    ui->chkDiscography->setMyData(static_cast<int>(MusicScraperInfos::Discography));

    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            connect(box, &QAbstractButton::clicked, this, &MusicSearchWidget::chkToggled);
        }
    }
    connect(ui->chkUnCheckAll, &QAbstractButton::clicked, this, &MusicSearchWidget::chkAllToggled);

    m_signalMapper = new QSignalMapper(ui->results);
    connect(m_signalMapper, SIGNAL(mapped(int)), this, SLOT(resultClicked(int)));
}

MusicSearchWidget::~MusicSearchWidget()
{
    delete ui;
}

void MusicSearchWidget::search(QString searchString)
{
    ui->searchString->setText(searchString.replace(".", " ").trimmed());
    search();
}

void MusicSearchWidget::clear()
{
    ui->results->clearContents();
    ui->results->setRowCount(0);
}

void MusicSearchWidget::search()
{
    int index = ui->comboScraper->currentIndex();
    if (index < 0 || index >= Manager::instance()->musicScrapers().size()) {
        return;
    }
    m_scraperNo = ui->comboScraper->itemData(index, Qt::UserRole).toInt();
    setCheckBoxesEnabled(Manager::instance()->musicScrapers().at(m_scraperNo)->scraperSupports());
    clear();
    ui->comboScraper->setEnabled(false);
    ui->searchString->setLoading(true);
    if (m_type == "artist") {
        Manager::instance()->musicScrapers().at(m_scraperNo)->searchArtist(ui->searchString->text().trimmed());
    } else if (m_type == "album") {
        Manager::instance()
            ->musicScrapers()
            .at(m_scraperNo)
            ->searchAlbum(m_artistName, ui->searchString->text().trimmed());
    }
}

void MusicSearchWidget::showResults(QVector<ScraperSearchResult> results)
{
    ui->comboScraper->setEnabled(true);
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();

    for (const ScraperSearchResult& result : results) {
        auto label = new MyLabel(ui->results);
        QString name = result.name;
        if (result.released.isValid()) {
            name.append(QString(" (%1)").arg(result.released.toString("yyyy")));
        }

        label->setText(name + "<br /><span style=\"color: #999999;\">" + result.id + "</span>");
        label->setMargin(8);

        auto* item = new QTableWidgetItem;
        item->setData(Qt::UserRole, result.id);
        item->setData(Qt::UserRole + 1, result.id2);
        int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
        ui->results->setCellWidget(row, 0, label);

        connect(label, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
        m_signalMapper->setMapping(label, row);
    }
}

void MusicSearchWidget::resultClicked(QTableWidgetItem* item)
{
    m_scraperId = item->data(Qt::UserRole).toString();
    m_scraperId2 = item->data(Qt::UserRole + 1).toString();
    emit sigResultClicked();
}

void MusicSearchWidget::resultClicked(int row)
{
    if (row < 0 || row >= ui->results->rowCount()) {
        return;
    }
    resultClicked(ui->results->item(row, 0));
}

void MusicSearchWidget::chkToggled()
{
    m_infosToLoad.clear();
    bool allToggled = true;
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isChecked() && box->myData().toInt() > 0 && box->isEnabled() && !box->isHidden()) {
            m_infosToLoad.append(MusicScraperInfos(box->myData().toInt()));
        }
        if (!box->isChecked() && box->myData().toInt() > 0 && box->isEnabled() && !box->isHidden()) {
            allToggled = false;
        }
    }
    ui->chkUnCheckAll->setChecked(allToggled);

    int scraperNo = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toInt();
    Settings::instance()->setScraperInfos(MainWidgets::Music, m_type + "/" + QString::number(scraperNo), m_infosToLoad);
}

void MusicSearchWidget::chkAllToggled(bool toggled)
{
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0 && box->isEnabled() && !box->isHidden()) {
            box->setChecked(toggled);
        }
    }
    chkToggled();
}

int MusicSearchWidget::scraperNo()
{
    return m_scraperNo;
}

QString MusicSearchWidget::scraperId()
{
    return m_scraperId;
}

QString MusicSearchWidget::scraperId2()
{
    return m_scraperId2;
}

QVector<MusicScraperInfos> MusicSearchWidget::infosToLoad()
{
    return m_infosToLoad;
}

void MusicSearchWidget::setCheckBoxesEnabled(QVector<MusicScraperInfos> scraperSupports)
{
    int scraperNo = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toInt();
    QVector<MusicScraperInfos> infos =
        Settings::instance()->scraperInfos<MusicScraperInfos>(m_type + "/" + QString::number(scraperNo));

    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        box->setEnabled(scraperSupports.contains(MusicScraperInfos(box->myData().toInt())));
        box->setChecked((infos.contains(MusicScraperInfos(box->myData().toInt())) || infos.isEmpty())
                        && scraperSupports.contains(MusicScraperInfos(box->myData().toInt())));
    }
    chkToggled();
}

void MusicSearchWidget::setType(const QString& type)
{
    m_type = type;
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        box->setVisible(box->property("type").toString() == "both" || box->property("type").toString() == type);
    }
}

void MusicSearchWidget::setArtistName(const QString& artistName)
{
    m_artistName = artistName;
}
