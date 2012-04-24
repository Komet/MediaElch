#include "TvShowWidgetEpisode.h"
#include "ui_TvShowWidgetEpisode.h"

#include <QMovie>
#include "Manager.h"
#include "MessageBox.h"
#include "MovieImageDialog.h"
#include "TvShowSearch.h"

TvShowWidgetEpisode::TvShowWidgetEpisode(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TvShowWidgetEpisode)
{
    ui->setupUi(this);

    m_episode = 0;

    ui->episodeName->clear();
    ui->thumbnailResolution->clear();
    ui->directors->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->writers->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);

    QFont font = ui->episodeName->font();
    font.setPointSize(font.pointSize()+4);
    ui->episodeName->setFont(font);

    font = ui->thumbnailResolution->font();
    #ifdef Q_WS_WIN
    font.setPointSize(font.pointSize()-1);
    #else
    font.setPointSize(font.pointSize()-2);
    #endif
    ui->thumbnailResolution->setFont(font);

    m_posterDownloadManager = new DownloadManager(this);

    connect(ui->name, SIGNAL(textChanged(QString)), ui->episodeName, SLOT(setText(QString)));
    connect(ui->buttonAddDirector, SIGNAL(clicked()), this, SLOT(onAddDirector()));
    connect(ui->buttonRemoveDirector, SIGNAL(clicked()), this, SLOT(onRemoveDirector()));
    connect(ui->buttonAddWriter, SIGNAL(clicked()), this, SLOT(onAddWriter()));
    connect(ui->buttonRemoveWriter, SIGNAL(clicked()), this, SLOT(onRemoveWriter()));
    connect(ui->thumbnail, SIGNAL(clicked()), this, SLOT(onChooseThumbnail()));
    connect(m_posterDownloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(onPosterDownloadFinished(DownloadManagerElement)));

    m_loadingMovie = new QMovie(":/img/spinner.gif");
    m_loadingMovie->start();
    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    onSetEnabled(false);
    onClear();
}

TvShowWidgetEpisode::~TvShowWidgetEpisode()
{
    delete ui;
}

void TvShowWidgetEpisode::resizeEvent(QResizeEvent *event)
{
    m_savingWidget->move(size().width()/2-m_savingWidget->width(), height()/2-m_savingWidget->height());
    QWidget::resizeEvent(event);
}

void TvShowWidgetEpisode::onClear()
{
    ui->tabWidget->setCurrentIndex(0);
    ui->directors->setRowCount(0);
    ui->writers->setRowCount(0);
    ui->thumbnailResolution->clear();
    ui->thumbnail->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->episodeName->clear();
    ui->files->clear();
    ui->files->setToolTip("");
    ui->name->clear();
    ui->showTitle->clear();
    ui->season->clear();
    ui->episode->clear();
    ui->rating->clear();
    ui->firstAired->setDate(QDate::currentDate());
    ui->playCount->clear();
    ui->lastPlayed->setDateTime(QDateTime::currentDateTime());
    ui->studio->clear();
    ui->overview->clear();
    m_chosenThumbnail = QImage();
}

void TvShowWidgetEpisode::onSetEnabled(bool enabled)
{
    ui->groupBox_3->setEnabled(enabled);
}

void TvShowWidgetEpisode::setEpisode(TvShowEpisode *episode)
{
    m_episode = episode;
    episode->loadImages(Manager::instance()->mediaCenterInterface());
    updateEpisodeInfo();
}

void TvShowWidgetEpisode::updateEpisodeInfo()
{
    if (m_episode == 0)
        return;

    onClear();

    ui->files->setText(m_episode->files().join(", "));
    ui->files->setToolTip(m_episode->files().join("\n"));
    ui->name->setText(m_episode->name());
    ui->showTitle->setText(m_episode->showTitle());
    ui->season->setValue(m_episode->season());
    ui->episode->setValue(m_episode->episode());
    ui->rating->setValue(m_episode->rating());
    ui->firstAired->setDate(m_episode->firstAired());
    ui->playCount->setValue(m_episode->playCount());
    ui->lastPlayed->setDateTime(m_episode->lastPlayed());
    ui->studio->setText(m_episode->network());
    ui->overview->setText(m_episode->overview());

    foreach (const QString &writer, m_episode->writers()) {
        int row = ui->writers->rowCount();
        ui->writers->insertRow(row);
        ui->writers->setItem(row, 0, new QTableWidgetItem(writer));
    }
    foreach (const QString &director, m_episode->directors()) {
        int row = ui->directors->rowCount();
        ui->directors->insertRow(row);
        ui->directors->setItem(row, 0, new QTableWidgetItem(director));
    }

    if (m_episode->tvShow()) {
        QStringList certifications = m_episode->tvShow()->certifications();
        certifications.prepend("");
        ui->certification->addItems(certifications);
        ui->certification->setCurrentIndex(certifications.indexOf(m_episode->certification()));
    } else {
        ui->certification->addItem(m_episode->certification());
    }

    if (!m_episode->thumbnailImage()->isNull()) {
        ui->thumbnail->setPixmap(QPixmap::fromImage(*m_episode->thumbnailImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->thumbnailResolution->setText(QString("%1x%2").arg(m_episode->thumbnailImage()->width()).arg(m_episode->thumbnailImage()->height()));
    } else {
        ui->thumbnail->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->thumbnailResolution->setText("");
    }
}

void TvShowWidgetEpisode::onAddDirector()
{
    int row = ui->directors->rowCount();
    ui->directors->insertRow(row);
    ui->directors->setItem(row, 0, new QTableWidgetItem(tr("Unknown Director")));
    ui->directors->scrollToBottom();
}

void TvShowWidgetEpisode::onRemoveDirector()
{
    int row = ui->directors->currentRow();
    if (row < 0 || row >= ui->directors->rowCount() || !ui->directors->currentItem()->isSelected())
        return;
    ui->directors->removeRow(row);
}

void TvShowWidgetEpisode::onAddWriter()
{
    int row = ui->writers->rowCount();
    ui->writers->insertRow(row);
    ui->writers->setItem(row, 0, new QTableWidgetItem(tr("Unknown Writer")));
    ui->writers->scrollToBottom();
}

void TvShowWidgetEpisode::onRemoveWriter()
{
    int row = ui->writers->currentRow();
    if (row < 0 || row >= ui->writers->rowCount() || !ui->writers->currentItem()->isSelected())
        return;
    ui->writers->removeRow(row);
}

void TvShowWidgetEpisode::onSaveInformation()
{
    if (m_episode == 0)
        return;

    onSetEnabled(false);
    m_savingWidget->show();

    m_episode->setCertification(ui->certification->currentText());
    m_episode->setEpisode(ui->episode->value());
    m_episode->setFirstAired(ui->firstAired->date());
    m_episode->setLastPlayed(ui->lastPlayed->dateTime());
    m_episode->setName(ui->name->text());
    m_episode->setNetwork(ui->studio->text());
    m_episode->setOverview(ui->overview->toPlainText());
    m_episode->setPlayCount(ui->playCount->value());
    m_episode->setRating(ui->rating->value());
    m_episode->setSeason(ui->season->value());
    m_episode->setShowTitle(ui->showTitle->text());

    QStringList directors;
    for (int i=0, n=ui->directors->rowCount() ; i<n ; ++i)
        directors.append(ui->directors->item(i, 0)->text());
    m_episode->setDirectors(directors);

    QStringList writers;
    for (int i=0, n=ui->writers->rowCount() ; i<n ; ++i)
        writers.append(ui->writers->item(i, 0)->text());
    m_episode->setWriters(writers);

    if (!m_chosenThumbnail.isNull())
        m_episode->setThumbnailImage(m_chosenThumbnail);

    m_episode->saveData(Manager::instance()->mediaCenterInterface());
    onSetEnabled(true);
    m_savingWidget->hide();
    MessageBox::instance()->showMessage(tr("Episode Saved"));
}

void TvShowWidgetEpisode::onStartScraperSearch()
{
    if (m_episode == 0)
        return;
    emit sigSetActionSearchEnabled(false, WidgetTvShows);
    emit sigSetActionSaveEnabled(false, WidgetTvShows);
    TvShowSearch::instance()->exec(m_episode->showTitle());
    if (TvShowSearch::instance()->result() == QDialog::Accepted) {
        onSetEnabled(false);
        m_episode->loadData(TvShowSearch::instance()->scraperId(), Manager::instance()->tvScrapers().at(0));
        connect(m_episode, SIGNAL(sigLoaded()), this, SLOT(onLoadDone()), Qt::UniqueConnection);
    } else {
        emit sigSetActionSearchEnabled(true, WidgetTvShows);
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
    }
}

void TvShowWidgetEpisode::onLoadDone()
{
    if (m_episode == 0)
        return;

    updateEpisodeInfo();
    onSetEnabled(true);

    if (!m_episode->thumbnail().isEmpty()) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = m_episode->thumbnail();
        m_posterDownloadManager->addDownload(d);
        ui->thumbnail->setPixmap(QPixmap());
        ui->thumbnail->setMovie(m_loadingMovie);
    }
}

void TvShowWidgetEpisode::onChooseThumbnail()
{
    if (m_episode == 0)
        return;

    MovieImageDialog::instance()->setImageType(TypeBackdrop);
    MovieImageDialog::instance()->clear();
    Poster p;
    p.originalUrl = m_episode->thumbnail();
    p.thumbUrl = m_episode->thumbnail();
    QList<Poster> posters;
    posters << p;
    MovieImageDialog::instance()->setDownloads(posters);
    MovieImageDialog::instance()->exec();

    if (MovieImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = MovieImageDialog::instance()->imageUrl();
        m_posterDownloadManager->addDownload(d);
        ui->thumbnail->setPixmap(QPixmap());
        ui->thumbnail->setMovie(m_loadingMovie);
    }
}

void TvShowWidgetEpisode::onPosterDownloadFinished(DownloadManagerElement elem)
{
    if (elem.imageType == TypeBackdrop) {
        m_chosenThumbnail = elem.image;
        ui->thumbnail->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->thumbnailResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
    }
    if (m_posterDownloadManager->downloadQueueSize() == 0)
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
}
