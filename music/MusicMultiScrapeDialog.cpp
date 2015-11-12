#include "MusicMultiScrapeDialog.h"
#include "ui_MusicMultiScrapeDialog.h"

#include "../globals/Manager.h"
#include "../settings/Settings.h"

MusicMultiScrapeDialog::MusicMultiScrapeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MusicMultiScrapeDialog)
{
    ui->setupUi(this);

    QFont font = ui->itemCounter->font();
#ifdef Q_OS_WIN32
    font.setPointSize(font.pointSize()-1);
#else
    font.setPointSize(font.pointSize()-2);
#endif
    ui->itemCounter->setFont(font);

    m_executed = false;
    m_currentArtist = 0;
    m_currentAlbum = 0;

    ui->chkName->setMyData(MusicScraperInfos::Name);
    ui->chkBorn->setMyData(MusicScraperInfos::Born);
    ui->chkFormed->setMyData(MusicScraperInfos::Formed);
    ui->chkYearsActive->setMyData(MusicScraperInfos::YearsActive);
    ui->chkDisbanded->setMyData(MusicScraperInfos::Disbanded);
    ui->chkDied->setMyData(MusicScraperInfos::Died);
    ui->chkBiography->setMyData(MusicScraperInfos::Biography);
    ui->chkArtist->setMyData(MusicScraperInfos::Artist);
    ui->chkLabel->setMyData(MusicScraperInfos::Label);
    ui->chkReview->setMyData(MusicScraperInfos::Review);
    ui->chkYear->setMyData(MusicScraperInfos::Year);
    ui->chkRating->setMyData(MusicScraperInfos::Rating);
    ui->chkReleaseDate->setMyData(MusicScraperInfos::ReleaseDate);
    ui->chkTitle->setMyData(MusicScraperInfos::Title);
    ui->chkGenres->setMyData(MusicScraperInfos::Genres);
    ui->chkStyles->setMyData(MusicScraperInfos::Styles);
    ui->chkMoods->setMyData(MusicScraperInfos::Moods);
    ui->chkThumbnail->setMyData(MusicScraperInfos::Thumb);
    ui->chkFanart->setMyData(MusicScraperInfos::Fanart);
    ui->chkExtraFanarts->setMyData(MusicScraperInfos::ExtraFanarts);
    ui->chkLogo->setMyData(MusicScraperInfos::Logo);
    ui->chkCover->setMyData(MusicScraperInfos::Cover);
    ui->chkCdArt->setMyData(MusicScraperInfos::CdArt);
    ui->chkDiscography->setMyData(MusicScraperInfos::Discography);

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0)
            connect(box, SIGNAL(clicked()), this, SLOT(onChkToggled()));
    }
    connect(ui->chkUnCheckAll, SIGNAL(clicked(bool)), this, SLOT(onChkAllToggled(bool)));
    connect(ui->btnStartScraping, SIGNAL(clicked()), this, SLOT(onStartScraping()));
}

MusicMultiScrapeDialog::~MusicMultiScrapeDialog()
{
    delete ui;
}

MusicMultiScrapeDialog *MusicMultiScrapeDialog::instance(QWidget *parent)
{
    static MusicMultiScrapeDialog *m_instance = 0;
    if (m_instance == 0)
        m_instance = new MusicMultiScrapeDialog(parent);
    return m_instance;
}

void MusicMultiScrapeDialog::onChkToggled()
{
    m_albumInfosToLoad.clear();
    m_artistInfosToLoad.clear();
    bool allToggled = true;
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (!box->isChecked()) {
            allToggled = false;
            continue;
        }
        if (box->property("type").toString() == "artist" || box->property("type").toString() == "both")
            m_artistInfosToLoad.append(box->myData().toInt());
        if (box->property("type").toString() == "album" || box->property("type").toString() == "both")
            m_albumInfosToLoad.append(box->myData().toInt());
    }
    ui->chkUnCheckAll->setChecked(allToggled);
    ui->btnStartScraping->setEnabled(!m_albumInfosToLoad.isEmpty() || !m_artistInfosToLoad.isEmpty());
}

void MusicMultiScrapeDialog::onChkAllToggled(bool toggled)
{
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0)
            box->setChecked(toggled);
    }
    onChkToggled();
}

bool MusicMultiScrapeDialog::isExecuted()
{
    return m_executed;
}

int MusicMultiScrapeDialog::exec()
{
    m_queue.clear();
    ui->itemCounter->setVisible(false);
    ui->btnCancel->setVisible(true);
    ui->btnClose->setVisible(false);
    ui->btnStartScraping->setVisible(true);
    ui->btnStartScraping->setEnabled(true);
    ui->chkAutoSave->setEnabled(true);
    ui->chkScrapeAllAlbums->setEnabled(true);
    ui->progressAll->setValue(0);
    ui->progressItem->setValue(0);
    ui->groupBox->setEnabled(true);
    ui->itemName->clear();
    m_currentArtist = 0;
    m_currentAlbum = 0;
    m_executed = true;
    onChkToggled();
    adjustSize();

    return QDialog::exec();
}

void MusicMultiScrapeDialog::accept()
{
    disconnectScrapers();
    m_executed = false;
    QDialog::accept();
}

void MusicMultiScrapeDialog::reject()
{
    disconnectScrapers();
    m_executed = false;
    if (m_currentAlbum)
        m_currentAlbum->controller()->abortDownloads();
    if (m_currentArtist)
        m_currentArtist->controller()->abortDownloads();
    m_queue.clear();
    QDialog::reject();
}

void MusicMultiScrapeDialog::disconnectScrapers()
{
    foreach (MusicScraperInterface *scraper, Manager::instance()->musicScrapers())
        disconnect(scraper, SIGNAL(sigSearchDone(QList<ScraperSearchResult>)), this, SLOT(onSearchFinished(QList<ScraperSearchResult>)));
}

void MusicMultiScrapeDialog::onStartScraping()
{
    disconnectScrapers();

    ui->groupBox->setEnabled(false);
    ui->btnStartScraping->setEnabled(false);
    ui->chkAutoSave->setEnabled(false);
    ui->chkScrapeAllAlbums->setEnabled(false);

    m_scraperInterface = Manager::instance()->musicScrapers().at(0);
    connect(m_scraperInterface, SIGNAL(sigSearchDone(QList<ScraperSearchResult>)), this, SLOT(onSearchFinished(QList<ScraperSearchResult>)), Qt::UniqueConnection);

    QList<Album*> queueAlbums;
    foreach (Artist *artist, m_artists) {
        QueueItem item1;
        item1.album = 0;
        item1.artist = artist;
        m_queue.append(item1);
        if (ui->chkScrapeAllAlbums->isChecked()) {
            foreach (Album *album, artist->albums()) {
                QueueItem item2;
                item2.album = album;
                item2.artist = 0;
                m_queue.append(item2);
                queueAlbums.append(album);
            }
        }
    }

    foreach (Album *album, m_albums) {
        if (!queueAlbums.contains(album)) {
            QueueItem item;
            item.album = album;
            item.artist = 0;
            m_queue.append(item);
            queueAlbums.append(album);
        }
    }

    ui->itemCounter->setText(QString("0/%1").arg(m_queue.count()));
    ui->itemCounter->setVisible(true);
    ui->progressAll->setMaximum(m_queue.count());
    scrapeNext();
}

void MusicMultiScrapeDialog::onScrapingFinished()
{
    ui->itemCounter->setVisible(false);
    ui->itemName->setText(tr("Scraping of %n items has finished.", "", ui->progressAll->maximum()));
    ui->progressItem->setValue(ui->progressItem->maximum());
    ui->progressAll->setValue(ui->progressAll->maximum());
    ui->btnCancel->setVisible(false);
    ui->btnClose->setVisible(true);
    ui->btnStartScraping->setVisible(false);
}

void MusicMultiScrapeDialog::scrapeNext()
{
    if (!isExecuted())
        return;

    if (m_currentAlbum && ui->chkAutoSave->isChecked())
        m_currentAlbum->controller()->saveData(Manager::instance()->mediaCenterInterface());

    if (m_currentArtist && ui->chkAutoSave->isChecked())
        m_currentArtist->controller()->saveData(Manager::instance()->mediaCenterInterface());

    if (m_queue.isEmpty()) {
        onScrapingFinished();
        return;
    }

    QueueItem item = m_queue.dequeue();
    m_currentAlbum = item.album;
    m_currentArtist = item.artist;
    if (m_currentAlbum) {
        ui->itemName->setText(m_currentAlbum->title());
    } else if (m_currentArtist) {
        ui->itemName->setText(m_currentArtist->name());
    }
    ui->itemCounter->setText(QString("%1/%2").arg(ui->progressAll->maximum()-m_queue.count()).arg(ui->progressAll->maximum()));
    ui->progressAll->setValue(ui->progressAll->maximum()-m_queue.size()-1);
    ui->progressItem->setValue(0);

    if (m_currentAlbum) {
        connect(m_currentAlbum->controller(), SIGNAL(sigLoadDone(Album*)), this, SLOT(scrapeNext()), Qt::UniqueConnection);
        connect(m_currentAlbum->controller(), SIGNAL(sigDownloadProgress(Album*,int,int)), this, SLOT(onProgress(Album*,int,int)), Qt::UniqueConnection);
        if (!m_currentAlbum->mbAlbumId().isEmpty()) {
            m_currentAlbum->controller()->loadData(m_currentAlbum->mbAlbumId(), m_currentAlbum->mbReleaseGroupId(), m_scraperInterface, m_albumInfosToLoad);
        } else {
            m_scraperInterface->searchAlbum((m_currentAlbum->artist().isEmpty() && m_currentAlbum->artistObj()) ? m_currentAlbum->artistObj()->name() : m_currentAlbum->artist(),
                                            m_currentAlbum->title());
        }
    } else if (m_currentArtist) {
        connect(m_currentArtist->controller(), SIGNAL(sigLoadDone(Artist*)), this, SLOT(scrapeNext()), Qt::UniqueConnection);
        connect(m_currentArtist->controller(), SIGNAL(sigDownloadProgress(Artist*,int,int)), this, SLOT(onProgress(Artist*,int,int)), Qt::UniqueConnection);
        if (!m_currentArtist->mbId().isEmpty()) {
            m_currentArtist->controller()->loadData(m_currentArtist->mbId(), m_scraperInterface, m_artistInfosToLoad);
        } else {
            m_scraperInterface->searchArtist(m_currentArtist->name());
        }
    }
}

void MusicMultiScrapeDialog::onSearchFinished(QList<ScraperSearchResult> results)
{
    if (!isExecuted())
        return;
    if (results.isEmpty()) {
        scrapeNext();
        return;
    }

    if (m_currentArtist)
        m_currentArtist->controller()->loadData(results.first().id, m_scraperInterface, m_artistInfosToLoad);
    else if (m_currentAlbum)
        m_currentAlbum->controller()->loadData(results.first().id, results.first().id2, m_scraperInterface, m_albumInfosToLoad);
}

void MusicMultiScrapeDialog::onProgress(Artist *artist, int current, int maximum)
{
    Q_UNUSED(artist);
    if (!isExecuted())
        return;
    ui->progressItem->setValue(maximum-current);
    ui->progressItem->setMaximum(maximum);
}

void MusicMultiScrapeDialog::onProgress(Album *album, int current, int maximum)
{
    Q_UNUSED(album);
    if (!isExecuted())
        return;
    ui->progressItem->setValue(maximum-current);
    ui->progressItem->setMaximum(maximum);
}

void MusicMultiScrapeDialog::setItems(QList<Artist *> artists, QList<Album *> albums)
{
    m_artists = artists;
    m_albums = albums;
}
