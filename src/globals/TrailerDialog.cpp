#include "TrailerDialog.h"
#include "ui_TrailerDialog.h"

#include <QMessageBox>

#include "globals/Manager.h"
#include "network/NetworkRequest.h"
#include "scrapers/trailer/TrailerProvider.h"

TrailerDialog::TrailerDialog(QWidget* parent) : QDialog(parent), ui(new Ui::TrailerDialog)
{
    using namespace mediaelch::scraper;

    ui->setupUi(this);

#ifdef Q_OS_MAC
    QFont font = ui->trailers->font();
    font.setPointSize(font.pointSize() - 1);
    ui->trailers->setFont(font);
    ui->time->setFont(font);
#endif

    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->trailers->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->trailers->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->trailers->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);
    ui->stackedWidget->setAnimation(QEasingCurve::OutCubic);
    ui->stackedWidget->setSpeed(400);

#ifdef Q_OS_MAC
    // setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    m_network = new mediaelch::network::NetworkManager(this);

    for (TrailerProvider* provider : Manager::instance()->trailerProviders()) {
        ui->comboScraper->addItem(provider->name(), Manager::instance()->trailerProviders().indexOf(provider));
        connect(provider, &TrailerProvider::sigSearchDone, this, &TrailerDialog::showResults);
        connect(provider, &TrailerProvider::sigLoadDone, this, &TrailerDialog::showTrailers);
    }

    connect(ui->comboScraper, elchOverload<int>(&QComboBox::currentIndexChanged), this, &TrailerDialog::searchIndex);
    connect(ui->searchString, &QLineEdit::returnPressed, this, &TrailerDialog::search);
    connect(ui->results, &QTableWidget::itemClicked, this, &TrailerDialog::resultClicked);
    connect(ui->trailers, &QTableWidget::itemClicked, this, &TrailerDialog::trailerClicked);
    connect(ui->buttonBackToResults, &QAbstractButton::clicked, this, &TrailerDialog::backToResults);
    connect(ui->buttonBackToTrailers, &QAbstractButton::clicked, this, &TrailerDialog::backToTrailers);
    connect(ui->buttonDownload, &QAbstractButton::clicked, this, &TrailerDialog::startDownload);
    connect(ui->buttonCancelDownload, &QAbstractButton::clicked, this, &TrailerDialog::cancelDownload);
    connect(ui->stackedWidget, &SlidingStackedWidget::animationFinished, this, &TrailerDialog::onAnimationFinished);

    m_mediaPlayer = new QMediaPlayer();
    m_videoWidget = new QVideoWidget(this);
    m_mediaPlayer->setVideoOutput(m_videoWidget);
    auto* layout = new QVBoxLayout(ui->video);
    layout->addWidget(m_videoWidget);
    ui->video->setLayout(layout);

    connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &TrailerDialog::onStateChanged);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &TrailerDialog::onNewTotalTime);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &TrailerDialog::onUpdateTime);
    QObject::connect(m_mediaPlayer,
        static_cast<void (QMediaPlayer::*)(QMediaPlayer::Error)>(&QMediaPlayer::error),
        this,
        &TrailerDialog::onTrailerError);
    connect(ui->btnPlayPause, &QAbstractButton::clicked, this, &TrailerDialog::onPlayPause);
    connect(ui->seekSlider, &QAbstractSlider::sliderReleased, this, &TrailerDialog::onSliderPositionChanged);
}

TrailerDialog::~TrailerDialog()
{
    m_mediaPlayer->deleteLater();
    delete ui;
}

void TrailerDialog::clear()
{
    ui->results->clearContents();
    ui->results->setRowCount(0);
    ui->comboScraper->setEnabled(true);
    ui->searchString->setEnabled(true);
}

int TrailerDialog::exec(Movie* movie)
{
    m_videoWidget->hide();
    QSize newSize;
    newSize.setHeight(qMin(600, parentWidget()->size().height() - 200));
    newSize.setWidth(qMin(600, parentWidget()->size().width() - 400));
    resize(newSize);

    m_downloadInProgress = false;
    m_currentMovie = movie;
    ui->stackedWidget->setCurrentIndex(0);
    ui->searchString->setText(movie->name());
    search();
    return QDialog::exec();
}

int TrailerDialog::exec()
{
    return 0;
}

void TrailerDialog::reject()
{
    if (m_downloadInProgress) {
        cancelDownload();
    }
    m_mediaPlayer->stop();
    m_mediaPlayer->setMedia(QMediaContent());
    m_videoWidget->hide();
    QDialog::reject();
}

void TrailerDialog::search()
{
    const int index = ui->comboScraper->currentIndex();
    searchIndex(index);
}

void TrailerDialog::searchIndex(int comboIndex)
{
    if (comboIndex < 0 || comboIndex >= Manager::instance()->trailerProviders().size()) {
        qWarning() << "[Trailer] Invalid Index, cannot start search.";
        return;
    }

    QString searchString = ui->searchString->text();
    qInfo() << "[Trailer] Search for movie:" << searchString;

    m_providerNo = ui->comboScraper->itemData(comboIndex, Qt::UserRole).toInt();

    clear();
    ui->comboScraper->setEnabled(false);
    ui->searchString->setLoading(true);
    // Start trailer search
    Manager::instance()->trailerProviders().at(m_providerNo)->searchMovie(searchString);
}

void TrailerDialog::showResults(QVector<ScraperSearchResult> results)
{
    qInfo() << "[Trailer] Found" << results.size() << "trailers";

    if (results.count() != 1) {
        ui->stackedWidget->slideInIdx(0);
    }

    ui->comboScraper->setEnabled(true);
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();
    for (const ScraperSearchResult& result : results) {
        auto* item = new QTableWidgetItem(QString("%1").arg(result.name));
        item->setData(Qt::UserRole, result.id);
        const int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
    }

    if (results.count() == 1) {
        ui->stackedWidget->setCurrentIndex(1);
        resultClicked(ui->results->item(0, 0));
    }
}

void TrailerDialog::resultClicked(QTableWidgetItem* item)
{
    m_providerId = item->data(Qt::UserRole).toString();
    ui->trailers->clearContents();
    ui->trailers->setRowCount(0);
    ui->stackedWidget->slideInIdx(1);
    Manager::instance()->trailerProviders().at(m_providerNo)->loadMovieTrailers(item->data(Qt::UserRole).toString());
}

void TrailerDialog::showTrailers(QVector<TrailerResult> trailers)
{
    m_currentTrailers = trailers;
    bool hasPreview = false;
    bool hasLanguage = false;
    for (int i = 0, n = trailers.size(); i < n; ++i) {
        TrailerResult trailer = trailers.at(i);
        int row = ui->trailers->rowCount();
        ui->trailers->insertRow(row);
        auto* trailerPreview = new QLabel(ui->trailers);
        trailerPreview->setMargin(4);
        if (!trailer.previewImage.isNull()) {
            trailerPreview->setPixmap(
                QPixmap::fromImage(trailer.previewImage.scaledToWidth(100, Qt::SmoothTransformation)));
            hasPreview = true;
        }
        auto* item = new QTableWidgetItem(trailer.name);
        item->setData(Qt::UserRole, i);
        ui->trailers->setCellWidget(row, 0, trailerPreview);
        ui->trailers->setItem(row, 1, new QTableWidgetItem(trailer.language));
        ui->trailers->setItem(row, 2, item);
        if (!trailer.language.isEmpty()) {
            hasLanguage = true;
        }
    }
    ui->trailers->setColumnHidden(0, !hasPreview);
    ui->trailers->setColumnHidden(1, !hasLanguage);
}

void TrailerDialog::trailerClicked(QTableWidgetItem* item)
{
    int row = item->row();
    if (row < 0 || row >= ui->trailers->rowCount()) {
        return;
    }
    int trailerNo = ui->trailers->item(row, 2)->data(Qt::UserRole).toInt();
    if (trailerNo < 0 || trailerNo >= m_currentTrailers.count()) {
        return;
    }

    TrailerResult result = m_currentTrailers.at(trailerNo);
    ui->url->setText(result.trailerUrl.toString());

    ui->buttonCancelDownload->setVisible(false);
    ui->buttonDownload->setVisible(true);
    ui->progress->clear();
    ui->progressBar->setVisible(false);
    ui->progressBar->setValue(0);

    ui->stackedWidget->slideInIdx(2);
    m_mediaPlayer->setMedia(QMediaContent(result.trailerUrl));
}

void TrailerDialog::backToResults()
{
    ui->stackedWidget->slideInIdx(0);
}

void TrailerDialog::backToTrailers()
{
    m_mediaPlayer->stop();
    m_videoWidget->hide();
    ui->stackedWidget->slideInIdx(1);
}

void TrailerDialog::startDownload()
{
    if (m_currentMovie->files().isEmpty()) {
        return;
    }

    QFileInfo fi(m_currentMovie->files().at(0).toString());
    m_trailerFileName =
        QString("%1%2%3-trailer").arg(fi.canonicalPath()).arg(QDir::separator()).arg(fi.completeBaseName());
    m_output.setFileName(QString("%1.download").arg(m_trailerFileName));

    if (!m_output.open(QIODevice::WriteOnly)) {
        return;
    }

    ui->buttonDownload->setVisible(false);
    ui->buttonCancelDownload->setVisible(true);
    ui->buttonBackToTrailers->setEnabled(false);
    ui->buttonClose3->setEnabled(false);
    ui->progressBar->setVisible(true);

    QNetworkRequest request = mediaelch::network::requestWithDefaults(QUrl(ui->url->text()));

    if (ui->url->text().contains("//trailers.apple.com") || ui->url->text().contains("//movietrailers.apple.com")) {
        request.setHeader(QNetworkRequest::UserAgentHeader, "QuickTime/7.7");
    }

    m_downloadInProgress = true;
    m_downloadReply = m_network->get(request);
    connect(m_downloadReply, &QNetworkReply::finished, this, &TrailerDialog::downloadFinished);
    connect(m_downloadReply, &QNetworkReply::downloadProgress, this, &TrailerDialog::downloadProgress);
    connect(m_downloadReply, &QIODevice::readyRead, this, &TrailerDialog::downloadReadyRead);
    m_downloadTime.start();
}

void TrailerDialog::cancelDownload()
{
    ui->buttonDownload->setVisible(true);
    ui->buttonCancelDownload->setVisible(false);
    ui->buttonBackToTrailers->setEnabled(true);
    ui->buttonClose3->setEnabled(true);
    ui->progressBar->setVisible(false);
    ui->progressBar->setValue(0);
    ui->progress->clear();

    m_downloadReply->abort();
    m_downloadReply->deleteLater();

    m_output.close();
    m_downloadInProgress = false;

#ifdef Q_OS_MAC
    TrailerDialog::resize(width() + 1, height() + 1);
    TrailerDialog::resize(width() - 1, height() - 1);
#endif
}

void TrailerDialog::downloadProgress(int received, int total)
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

void TrailerDialog::downloadFinished()
{
    const int statusCode = m_downloadReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    m_downloadInProgress = false;
    m_output.close();

    QFile file(QString("%1.download").arg(m_trailerFileName));
    if (m_downloadReply->error() == QNetworkReply::NoError) {
        ui->progress->setText(tr("Download Finished"));
        QString extension = QUrl(ui->url->text()).path();
        if (extension.lastIndexOf(".") != -1) {
            extension = extension.right(extension.length() - extension.lastIndexOf(".") - 1);
        } else {
            extension = "mov";
        }
        QString newFileName = QString("%1.%2").arg(m_trailerFileName).arg(extension);
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
            }
        } else {
            file.rename(newFileName);
        }

    } else if (m_downloadReply->error() == QNetworkReply::OperationCanceledError) {
        ui->progress->setText(tr("Download Canceled"));
        file.remove();

    } else if (m_downloadReply->error() == QNetworkReply::ContentNotFoundError) {
        ui->progress->setText(tr("Download Not Found (404)"));
        file.remove();

    } else {
        ui->progress->setText(tr("Download Error (%1)").arg(QString::number(statusCode)));
        file.remove();
    }

    ui->buttonDownload->setVisible(true);
    ui->buttonCancelDownload->setVisible(false);
    ui->progressBar->setVisible(false);
    ui->buttonBackToTrailers->setEnabled(true);
    ui->buttonClose3->setEnabled(true);

    m_downloadReply->deleteLater();

#ifdef Q_OS_MAC
    TrailerDialog::resize(width() + 1, height() + 1);
    TrailerDialog::resize(width() - 1, height() - 1);
#endif
}

void TrailerDialog::downloadReadyRead()
{
    m_output.write(m_downloadReply->readAll());
}

void TrailerDialog::onNewTotalTime(qint64 totalTime)
{
    m_totalTime = (totalTime < 0) ? 0 : totalTime;
    onUpdateTime(m_mediaPlayer->position());
    ui->seekSlider->setEnabled(m_totalTime != 0);
}

void TrailerDialog::onUpdateTime(qint64 currentTime)
{
    QString tTime = QString("%1:%2").arg(m_totalTime / 1000 / 60).arg((m_totalTime / 1000) % 60, 2, 10, QChar('0'));
    QString cTime = QString("%1:%2").arg(currentTime / 1000 / 60).arg((currentTime / 1000) % 60, 2, 10, QChar('0'));
    ui->time->setText(QString("%1 / %2").arg(cTime).arg(tTime));

    int position = 0;
    if (m_totalTime > 0) {
        position = qRound((static_cast<float>(currentTime) / m_totalTime) * 100.0f);
    }
    ui->seekSlider->setValue(position);
}

void TrailerDialog::onTrailerError(QMediaPlayer::Error error)
{
    const QString msg = [error]() {
        switch (error) {
        case QMediaPlayer::NetworkError: return tr("Network Error");
        case QMediaPlayer::ResourceError: return tr("Resource could not be played");
        case QMediaPlayer::FormatError: return tr("Video format error");
        default: return QString{};
        }
    }();
    if (!msg.isEmpty()) {
        ui->progress->setText(msg);
    }
}

void TrailerDialog::onStateChanged(QMediaPlayer::State newState)
{
    switch (newState) {
    case QMediaPlayer::PlayingState: ui->btnPlayPause->setIcon(QIcon(":/img/video_pause_64.png")); break;
    case QMediaPlayer::StoppedState:
    case QMediaPlayer::PausedState: ui->btnPlayPause->setIcon(QIcon(":/img/video_play_64.png")); break;
    }
}

void TrailerDialog::onPlayPause()
{
#ifdef Q_OS_MAC
    TrailerDialog::resize(width() + 1, height() + 1);
    TrailerDialog::resize(width() - 1, height() - 1);
#endif

    switch (m_mediaPlayer->state()) {
    case QMediaPlayer::PlayingState: m_mediaPlayer->pause(); break;
    case QMediaPlayer::StoppedState:

#ifdef Q_OS_MAC
        TrailerDialog::resize(width() + 1, height() + 1);
        TrailerDialog::resize(width() - 1, height() - 1);
#endif

    case QMediaPlayer::PausedState: m_mediaPlayer->play(); break;
    }

#ifdef Q_OS_MAC
    TrailerDialog::resize(width() + 1, height() + 1);
    TrailerDialog::resize(width() - 1, height() - 1);
#endif
}

void TrailerDialog::onAnimationFinished()
{
    if (ui->stackedWidget->currentIndex() == 2) {
        m_videoWidget->show();
    }
}

void TrailerDialog::onSliderPositionChanged()
{
    if (m_totalTime == 0) {
        return;
    }
    m_mediaPlayer->setPosition(
        qRound(static_cast<float>(m_totalTime) * static_cast<float>(ui->seekSlider->value()) / 100.0f));
}
