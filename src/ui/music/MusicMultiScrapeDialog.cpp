#include "MusicMultiScrapeDialog.h"
#include "ui_MusicMultiScrapeDialog.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "globals/Manager.h"
#include "settings/Settings.h"

MusicMultiScrapeDialog::MusicMultiScrapeDialog(QWidget* parent) : QDialog(parent), ui(new Ui::MusicMultiScrapeDialog)
{
    ui->setupUi(this);

    QFont font = ui->itemCounter->font();
#ifdef Q_OS_WIN
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif
    ui->itemCounter->setFont(font);

    m_executed = false;
    m_currentArtist = nullptr;
    m_currentAlbum = nullptr;

    ui->chkName->setMyData(static_cast<int>(MusicScraperInfo::Name));
    ui->chkBorn->setMyData(static_cast<int>(MusicScraperInfo::Born));
    ui->chkFormed->setMyData(static_cast<int>(MusicScraperInfo::Formed));
    ui->chkYearsActive->setMyData(static_cast<int>(MusicScraperInfo::YearsActive));
    ui->chkDisbanded->setMyData(static_cast<int>(MusicScraperInfo::Disbanded));
    ui->chkDied->setMyData(static_cast<int>(MusicScraperInfo::Died));
    ui->chkBiography->setMyData(static_cast<int>(MusicScraperInfo::Biography));
    ui->chkArtist->setMyData(static_cast<int>(MusicScraperInfo::Artist));
    ui->chkLabel->setMyData(static_cast<int>(MusicScraperInfo::Label));
    ui->chkReview->setMyData(static_cast<int>(MusicScraperInfo::Review));
    ui->chkYear->setMyData(static_cast<int>(MusicScraperInfo::Year));
    ui->chkRating->setMyData(static_cast<int>(MusicScraperInfo::Rating));
    ui->chkReleaseDate->setMyData(static_cast<int>(MusicScraperInfo::ReleaseDate));
    ui->chkTitle->setMyData(static_cast<int>(MusicScraperInfo::Title));
    ui->chkGenres->setMyData(static_cast<int>(MusicScraperInfo::Genres));
    ui->chkStyles->setMyData(static_cast<int>(MusicScraperInfo::Styles));
    ui->chkMoods->setMyData(static_cast<int>(MusicScraperInfo::Moods));
    ui->chkThumbnail->setMyData(static_cast<int>(MusicScraperInfo::Thumb));
    ui->chkFanart->setMyData(static_cast<int>(MusicScraperInfo::Fanart));
    ui->chkExtraFanarts->setMyData(static_cast<int>(MusicScraperInfo::ExtraFanarts));
    ui->chkLogo->setMyData(static_cast<int>(MusicScraperInfo::Logo));
    ui->chkCover->setMyData(static_cast<int>(MusicScraperInfo::Cover));
    ui->chkCdArt->setMyData(static_cast<int>(MusicScraperInfo::CdArt));
    ui->chkDiscography->setMyData(static_cast<int>(MusicScraperInfo::Discography));

    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            connect(box, &QAbstractButton::clicked, this, &MusicMultiScrapeDialog::onChkToggled);
        }
    }
    connect(ui->chkUnCheckAll, &QAbstractButton::clicked, this, &MusicMultiScrapeDialog::onChkAllToggled);
    connect(ui->btnStartScraping, &QAbstractButton::clicked, this, &MusicMultiScrapeDialog::onStartScraping);
}

MusicMultiScrapeDialog::~MusicMultiScrapeDialog()
{
    delete ui;
}

void MusicMultiScrapeDialog::onChkToggled()
{
    m_albumInfosToLoad.clear();
    m_artistInfosToLoad.clear();
    bool allToggled = true;
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (!box->isChecked()) {
            allToggled = false;
            continue;
        }
        if (box->property("type").toString() == "artist" || box->property("type").toString() == "both") {
            m_artistInfosToLoad.insert(MusicScraperInfo(box->myData().toInt()));
        }
        if (box->property("type").toString() == "album" || box->property("type").toString() == "both") {
            m_albumInfosToLoad.insert(MusicScraperInfo(box->myData().toInt()));
        }
    }
    ui->chkUnCheckAll->setChecked(allToggled);
    ui->btnStartScraping->setEnabled(!m_albumInfosToLoad.isEmpty() || !m_artistInfosToLoad.isEmpty());
}

void MusicMultiScrapeDialog::onChkAllToggled(bool toggled)
{
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            box->setChecked(toggled);
        }
    }
    onChkToggled();
}

bool MusicMultiScrapeDialog::isExecuted() const
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
    m_currentArtist = nullptr;
    m_currentAlbum = nullptr;
    m_executed = true;
    onChkToggled();
    adjustSize();

    return QDialog::exec();
}

void MusicMultiScrapeDialog::accept()
{
    m_executed = false;
    QDialog::accept();
}

void MusicMultiScrapeDialog::reject()
{
    m_executed = false;
    if (m_currentAlbum != nullptr) {
        m_currentAlbum->controller()->abortDownloads();
    }
    if (m_currentArtist != nullptr) {
        m_currentArtist->controller()->abortDownloads();
    }
    m_queue.clear();
    QDialog::reject();
}

void MusicMultiScrapeDialog::onStartScraping()
{
    ui->groupBox->setEnabled(false);
    ui->btnStartScraping->setEnabled(false);
    ui->chkAutoSave->setEnabled(false);
    ui->chkScrapeAllAlbums->setEnabled(false);

    // TODO: Not correct if we ever add more than one music scraper!
    m_scraperInterface = Manager::instance()->scrapers().musicScrapers().at(0);

    QVector<Album*> queueAlbums;
    for (Artist* artist : asConst(m_artists)) {
        QueueItem item1{};
        item1.album = nullptr;
        item1.artist = artist;
        m_queue.append(item1);
        if (ui->chkScrapeAllAlbums->isChecked()) {
            const auto albums = artist->albums();
            for (Album* album : albums) {
                m_queue.append({nullptr, album});
                queueAlbums.append(album);
            }
        }
    }

    for (Album* album : asConst(m_albums)) {
        if (!queueAlbums.contains(album)) {
            m_queue.append({nullptr, album});
            queueAlbums.append(album);
        }
    }

    ui->itemCounter->setText(QString("0/%1").arg(m_queue.count()));
    ui->itemCounter->setVisible(true);
    ui->progressAll->setMaximum(qsizetype_to_int(m_queue.count()));
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
    if (!isExecuted()) {
        return;
    }

    if ((m_currentAlbum != nullptr) && ui->chkAutoSave->isChecked()) {
        m_currentAlbum->controller()->saveData(Manager::instance()->mediaCenterInterface());
    }

    if ((m_currentArtist != nullptr) && ui->chkAutoSave->isChecked()) {
        m_currentArtist->controller()->saveData(Manager::instance()->mediaCenterInterface());
    }

    if (m_queue.isEmpty()) {
        onScrapingFinished();
        return;
    }

    QueueItem item = m_queue.dequeue();
    m_currentAlbum = item.album;
    m_currentArtist = item.artist;
    if (m_currentAlbum != nullptr) {
        ui->itemName->setText(m_currentAlbum->title().trimmed());
    } else if (m_currentArtist != nullptr) {
        ui->itemName->setText(m_currentArtist->name().trimmed());
    }
    ui->itemCounter->setText(
        QString("%1/%2").arg(ui->progressAll->maximum() - m_queue.count()).arg(ui->progressAll->maximum()));
    ui->progressAll->setValue(ui->progressAll->maximum() - qsizetype_to_int(m_queue.size()) - 1);
    ui->progressItem->setValue(0);

    if (m_currentAlbum != nullptr) {
        connect(m_currentAlbum->controller(),
            &AlbumController::sigLoadDone,
            this,
            &MusicMultiScrapeDialog::scrapeNext,
            Qt::UniqueConnection);

        connect(m_currentAlbum->controller(),
            &AlbumController::sigDownloadProgress,
            this,
            &MusicMultiScrapeDialog::onAlbumProgress,
            Qt::UniqueConnection);

        if (m_currentAlbum->mbAlbumId().isValid()) {
            m_currentAlbum->controller()->loadData(m_currentAlbum->mbAlbumId(),
                m_currentAlbum->mbReleaseGroupId(),
                m_scraperInterface,
                m_albumInfosToLoad);

        } else {
            mediaelch::scraper::AlbumSearchJob::Config config;
            config.albumQuery = m_currentAlbum->title();
            config.artistName = (m_currentAlbum->artist().isEmpty() && (m_currentAlbum->artistObj() != nullptr))
                                    ? m_currentAlbum->artistObj()->name().trimmed()
                                    : m_currentAlbum->artist().trimmed();
            auto* searchJob = m_scraperInterface->searchAlbum(config);
            connect(searchJob,
                &mediaelch::scraper::AlbumSearchJob::searchFinished,
                this,
                &MusicMultiScrapeDialog::onAlbumSearchFinished);
            searchJob->start();
        }

    } else if (m_currentArtist != nullptr) {
        connect(m_currentArtist->controller(),
            &ArtistController::sigLoadDone,
            this,
            &MusicMultiScrapeDialog::scrapeNext,
            Qt::UniqueConnection);

        connect(m_currentArtist->controller(),
            &ArtistController::sigDownloadProgress,
            this,
            &MusicMultiScrapeDialog::onArtistProgress,
            Qt::UniqueConnection);

        if (m_currentArtist->mbId().isValid()) {
            m_currentArtist->controller()->loadData(m_currentArtist->mbId(), m_scraperInterface, m_artistInfosToLoad);

        } else {
            mediaelch::scraper::ArtistSearchJob::Config config;
            config.query = m_currentArtist->name().trimmed();

            auto* searchJob = m_scraperInterface->searchArtist(config);
            connect(searchJob,
                &mediaelch::scraper::ArtistSearchJob::searchFinished,
                this,
                &MusicMultiScrapeDialog::onArtistSearchFinished);
            searchJob->start();
        }
    }
}

void MusicMultiScrapeDialog::onAlbumSearchFinished(mediaelch::scraper::AlbumSearchJob* searchJob)
{
    auto dls = makeDeleteLaterScope(searchJob);
    if (!isExecuted()) {
        return;
    }
    auto results = searchJob->results();
    if (results.isEmpty()) {
        scrapeNext();
        return;
    }

    MediaElch_Debug_Expects(m_currentArtist == nullptr);
    MediaElch_Debug_Expects(m_currentAlbum != nullptr);

    m_currentAlbum->controller()->loadData(MusicBrainzId(results.first().identifier),
        MusicBrainzId(results.first().groupIdentifier),
        m_scraperInterface,
        m_albumInfosToLoad);
}


void MusicMultiScrapeDialog::onArtistSearchFinished(mediaelch::scraper::ArtistSearchJob* searchJob)
{
    auto dls = makeDeleteLaterScope(searchJob);
    if (!isExecuted()) {
        return;
    }
    auto results = searchJob->results();
    if (results.isEmpty()) {
        scrapeNext();
        return;
    }

    MediaElch_Debug_Expects(m_currentArtist != nullptr);
    MediaElch_Debug_Expects(m_currentAlbum == nullptr);

    m_currentArtist->controller()->loadData(
        MusicBrainzId(results.first().identifier), m_scraperInterface, m_artistInfosToLoad);
}

void MusicMultiScrapeDialog::onArtistProgress(Artist* artist, int current, int maximum)
{
    Q_UNUSED(artist);
    if (!isExecuted()) {
        return;
    }
    ui->progressItem->setValue(maximum - current);
    ui->progressItem->setMaximum(maximum);
}

void MusicMultiScrapeDialog::onAlbumProgress(Album* album, int current, int maximum)
{
    Q_UNUSED(album);
    if (!isExecuted()) {
        return;
    }
    ui->progressItem->setValue(maximum - current);
    ui->progressItem->setMaximum(maximum);
}

void MusicMultiScrapeDialog::setItems(QVector<Artist*> artists, QVector<Album*> albums)
{
    m_artists = artists;
    m_albums = albums;
}
