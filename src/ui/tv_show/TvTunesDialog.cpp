#include "TvTunesDialog.h"
#include "ui_TvTunesDialog.h"

#include "network/NetworkRequest.h"
#include "scrapers/music/TvTunes.h"

#include <QMessageBox>

TvTunesDialog::TvTunesDialog(TvShow& show, QWidget* parent) : QDialog(parent), ui(new Ui::TvTunesDialog), m_show{show}
{
    using namespace mediaelch::scraper;
    ui->setupUi(this);

    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);
    ui->seekSlider->setEnabled(false);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

#ifdef Q_OS_MAC
    QFont font = ui->time->font();
    font.setPointSize(font.pointSize() - 1);
    ui->time->setFont(font);
#endif

    m_tvTunes = new TvTunes(this);
    m_network = new mediaelch::network::NetworkManager(this);

    connect(ui->btnClose, &QAbstractButton::clicked, this, &TvTunesDialog::onClose);
    connect(ui->searchString, &QLineEdit::returnPressed, this, &TvTunesDialog::onSearch);
    connect(ui->results, &QTableWidget::itemClicked, this, &TvTunesDialog::onResultClicked);
    connect(ui->buttonDownload, &QAbstractButton::clicked, this, &TvTunesDialog::startDownload);
    connect(ui->buttonCancelDownload, &QAbstractButton::clicked, this, &TvTunesDialog::cancelDownload);
    connect(m_tvTunes, &mediaelch::scraper::TvTunes::sigSearchDone, this, &TvTunesDialog::onShowResults);

    // Do not set QMediaPlayer's parent to this. See ~TvTunesDialog for more details.
    m_mediaPlayer = new QMediaPlayer();
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &TvTunesDialog::onNewTotalTime);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &TvTunesDialog::onUpdateTime);
    connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &TvTunesDialog::onStateChanged);
    connect(ui->btnPlayPause, &QAbstractButton::clicked, this, &TvTunesDialog::onPlayPause);
}

TvTunesDialog::~TvTunesDialog()
{
    // Don't delete the MediaPlayer directly because there may still be signals that
    // could be triggered. Otherwise we *will* run into use-after-free crashes.
    m_mediaPlayer->deleteLater();
    delete ui;
}

void TvTunesDialog::clear()
{
    m_mediaPlayer->stop();
    ui->btnPlayPause->setEnabled(false);
    ui->buttonDownload->setEnabled(false);
    ui->results->clearContents();
    ui->results->setRowCount(0);
}

int TvTunesDialog::exec()
{
    adjustSize();
    ui->buttonCancelDownload->setVisible(false);
    ui->buttonDownload->setVisible(true);
    ui->progress->clear();
    ui->progressBar->setVisible(false);
    ui->progressBar->setValue(0);
    m_downloadInProgress = false;
    m_fileDownloaded = false;
    ui->searchString->setText(m_show.title());
    onSearch();
    return QDialog::exec();
}

void TvTunesDialog::onSearch()
{
    clear();
    ui->searchString->setLoading(true);
    m_tvTunes->search(ui->searchString->text());
}

void TvTunesDialog::onShowResults(QVector<ScraperSearchResult> results)
{
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();
    for (const ScraperSearchResult& result : results) {
        auto* item = new QTableWidgetItem(result.name);
        item->setData(Qt::UserRole, result.id);
        int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
    }
}

void TvTunesDialog::onResultClicked(QTableWidgetItem* item)
{
    m_mediaPlayer->stop();
    QString url = item->data(Qt::UserRole).toString();
    m_themeUrl = url;
    m_totalTime = 0;
    m_mediaPlayer->setMedia(QMediaContent(url));
    m_mediaPlayer->play();
    ui->btnPlayPause->setEnabled(true);
    ui->buttonDownload->setEnabled(true);
}

void TvTunesDialog::onNewTotalTime(qint64 totalTime)
{
    m_totalTime = (totalTime < 0) ? 0 : totalTime;
    onUpdateTime(m_mediaPlayer->position());
}

void TvTunesDialog::onUpdateTime(qint64 currentTime)
{
    QString tTime = "??:??";
    if (m_totalTime > 0) {
        tTime = QString("%1:%2").arg(m_totalTime / 1000 / 60).arg((m_totalTime / 1000) % 60, 2, 10, QChar('0'));
    }
    QString cTime = QString("%1:%2").arg(currentTime / 1000 / 60).arg((currentTime / 1000) % 60, 2, 10, QChar('0'));
    ui->time->setText(QString("%1 / %2").arg(cTime).arg(tTime));

    int position = 0;
    if (m_totalTime > 0) {
        position = qRound((static_cast<float>(currentTime) / m_totalTime) * 100.0f);
    }
    ui->seekSlider->setValue(position);
}

void TvTunesDialog::onStateChanged(QMediaPlayer::State newState)
{
    switch (newState) {
    case QMediaPlayer::PlayingState: ui->btnPlayPause->setIcon(QIcon(":/img/video_pause_64.png")); break;
    case QMediaPlayer::StoppedState:
    case QMediaPlayer::PausedState: ui->btnPlayPause->setIcon(QIcon(":/img/video_play_64.png")); break;
    }
}

void TvTunesDialog::onPlayPause()
{
    switch (m_mediaPlayer->state()) {
    case QMediaPlayer::PlayingState: m_mediaPlayer->stop(); break;
    case QMediaPlayer::StoppedState:
    case QMediaPlayer::PausedState: m_mediaPlayer->play(); break;
    }
}

void TvTunesDialog::startDownload()
{
    if (!m_show.dir().isValid()) {
        return;
    }

    m_output.setFileName(m_show.dir().filePath("theme.mp3.download"));

    if (!m_output.open(QIODevice::WriteOnly)) {
        return;
    }

    ui->buttonDownload->setVisible(false);
    ui->buttonCancelDownload->setVisible(true);
    ui->btnClose->setEnabled(false);
    ui->progressBar->setVisible(true);
    ui->searchString->setEnabled(false);
    ui->progress->clear();

    m_downloadInProgress = true;
    // No NetworkReplyWatcher to avoid timeout.
    m_downloadReply = m_network->get(mediaelch::network::requestWithDefaults(m_themeUrl));
    connect(m_downloadReply, &QNetworkReply::finished, this, &TvTunesDialog::downloadFinished);
    connect(m_downloadReply, &QNetworkReply::downloadProgress, this, &TvTunesDialog::downloadProgress);
    connect(m_downloadReply, &QIODevice::readyRead, this, &TvTunesDialog::downloadReadyRead);
    m_downloadTime.start();
}

void TvTunesDialog::cancelDownload()
{
    ui->buttonDownload->setVisible(true);
    ui->buttonCancelDownload->setVisible(false);
    ui->btnClose->setEnabled(true);
    ui->progressBar->setVisible(false);
    ui->progressBar->setValue(0);
    ui->progress->clear();
    ui->searchString->setEnabled(true);

    m_downloadReply->abort();
    m_downloadReply->deleteLater();

    m_output.close();
    m_downloadInProgress = false;
}

void TvTunesDialog::downloadProgress(qint64 received, qint64 total)
{
    ui->progressBar->setRange(0, total);
    ui->progressBar->setValue(received);

    double speed = received * 1000.0 / m_downloadTime.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "bytes/sec";
    } else if (speed < 1024 * 1024) {
        speed /= 1024;
        unit = "kB/s";
    } else {
        speed /= 1024 * 1024;
        unit = "MB/s";
    }
    ui->progress->setText(QString("%1 %2").arg(speed, 3, 'f', 1).arg(unit));
}

void TvTunesDialog::downloadFinished()
{
    if (m_downloadReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302
        || m_downloadReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 301) {
        const QUrl url = m_downloadReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        qDebug() << "[TvTunesDialog] Got redirect" << url;
        m_downloadReply = m_network->get(mediaelch::network::requestWithDefaults(url));
        connect(m_downloadReply, &QNetworkReply::finished, this, &TvTunesDialog::downloadFinished);
        connect(m_downloadReply, &QNetworkReply::downloadProgress, this, &TvTunesDialog::downloadProgress);
        connect(m_downloadReply, &QIODevice::readyRead, this, &TvTunesDialog::downloadReadyRead);
        return;
    }

    m_downloadInProgress = false;
    m_output.close();

    QFile file(m_show.dir().filePath("theme.mp3.download"));
    if (m_downloadReply->error() == QNetworkReply::NoError) {
        ui->progress->setText(tr("Download Finished"));
        QString newFileName = m_show.dir().filePath("theme.mp3");
        QFileInfo fi(newFileName);
        if (fi.exists()) {
            QMessageBox msgBox;
            msgBox.setText(tr("The file %1 already exists.").arg(fi.fileName()));
            //: "it" refers to the file
            msgBox.setInformativeText(tr("Do you want to overwrite it?"));
            msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
            msgBox.setDefaultButton(QMessageBox::Yes);
            if (msgBox.exec() == QMessageBox::Yes) {
                QFile oldFile(newFileName);
                oldFile.remove();
                file.rename(newFileName);
                m_fileDownloaded = true;
            }
        } else {
            file.rename(newFileName);
            m_fileDownloaded = true;
        }
    } else {
        ui->progress->setText(tr("Download Canceled"));
        file.remove();
    }

    ui->buttonDownload->setVisible(true);
    ui->buttonCancelDownload->setVisible(false);
    ui->progressBar->setVisible(false);
    ui->btnClose->setEnabled(true);
    ui->searchString->setEnabled(true);

    m_downloadReply->deleteLater();
}

void TvTunesDialog::downloadReadyRead()
{
    m_output.write(m_downloadReply->readAll());
}

void TvTunesDialog::onClose()
{
    m_mediaPlayer->stop();
    if (m_downloadInProgress) {
        cancelDownload();
    }
    if (m_fileDownloaded) {
        QDialog::accept();
    } else {
        QDialog::reject();
    }
}
