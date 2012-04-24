#include "TvShowWidgetTvShow.h"
#include "ui_TvShowWidgetTvShow.h"

#include <QMovie>
#include "Manager.h"
#include "MessageBox.h"
#include "TvShowSearch.h"
#include "MovieImageDialog.h"

TvShowWidgetTvShow::TvShowWidgetTvShow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TvShowWidgetTvShow)
{
    ui->setupUi(this);

    m_show = 0;

    ui->showTitle->clear();
    ui->posterResolution->clear();
    ui->backdropResolution->clear();
    ui->genres->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->actors->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->actors->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    QFont font = ui->showTitle->font();
    font.setPointSize(font.pointSize()+4);
    ui->showTitle->setFont(font);

    font = ui->posterResolution->font();
    #ifdef Q_WS_WIN
    font.setPointSize(font.pointSize()-1);
    #else
    font.setPointSize(font.pointSize()-2);
    #endif
    ui->posterResolution->setFont(font);
    ui->backdropResolution->setFont(font);

    m_loadingMovie = new QMovie(":/img/spinner.gif");
    m_loadingMovie->start();
    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    m_progressMessageId = Constants::TvShowWidgetProgressMessageId;
    m_posterDownloadManager = new DownloadManager(this);

    connect(ui->name, SIGNAL(textChanged(QString)), ui->showTitle, SLOT(setText(QString)));
    connect(ui->buttonAddGenre, SIGNAL(clicked()), this, SLOT(onAddGenre()));
    connect(ui->buttonRemoveGenre, SIGNAL(clicked()), this, SLOT(onRemoveGenre()));
    connect(ui->buttonAddActor, SIGNAL(clicked()), this, SLOT(onAddActor()));
    connect(ui->buttonRemoveActor, SIGNAL(clicked()), this, SLOT(onRemoveActor()));
    connect(ui->poster, SIGNAL(clicked()), this, SLOT(onChoosePoster()));
    connect(ui->backdrop, SIGNAL(clicked()), this, SLOT(onChooseBackdrop()));
    connect(m_posterDownloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(onPosterDownloadFinished(DownloadManagerElement)));
    connect(m_posterDownloadManager, SIGNAL(downloadsLeft(int)), this, SLOT(onDownloadsLeft(int)));

    onSetEnabled(false);
    onClear();
}

TvShowWidgetTvShow::~TvShowWidgetTvShow()
{
    delete ui;
}

void TvShowWidgetTvShow::resizeEvent(QResizeEvent *event)
{
    m_savingWidget->move(size().width()/2-m_savingWidget->width(), height()/2-m_savingWidget->height());
    QWidget::resizeEvent(event);
}

void TvShowWidgetTvShow::onClear()
{
    ui->showTitle->clear();
    ui->genres->setRowCount(0);
    ui->actors->setRowCount(0);
    ui->dir->clear();
    ui->name->clear();
    ui->rating->clear();
    ui->firstAired->setDate(QDate::currentDate());
    ui->studio->clear();
    ui->overview->clear();
    ui->poster->setPixmap(QPixmap(":/img/film_reel.png"));
    ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->tabWidget->setCurrentIndex(0);
    ui->posterResolution->clear();
    ui->backdropResolution->clear();
    m_chosenPoster = QImage();
    m_chosenBackdrop = QImage();
    m_chosenSeasonPosters.clear();
    m_loadedFromScraper = false;

    QMapIterator<int, QList<QWidget*> > it(m_seasonLayoutWidgets);
    while (it.hasNext()) {
        it.next();
        ui->seasonsLayout->removeWidget(it.value().at(0));
        ui->seasonsLayout->removeWidget(it.value().at(1));
        it.value().at(0)->deleteLater();
        it.value().at(1)->deleteLater();
    }
    m_seasonLayoutWidgets.clear();
}

void TvShowWidgetTvShow::onSetEnabled(bool enabled)
{
    ui->groupBox_3->setEnabled(enabled);
    emit sigSetActionSearchEnabled(enabled, WidgetTvShows);
    emit sigSetActionSaveEnabled(enabled, WidgetTvShows);
}

void TvShowWidgetTvShow::setTvShow(TvShow *show)
{
    m_show = show;
    show->loadImages(Manager::instance()->mediaCenterInterface());
    updateTvShowInfo();
}

void TvShowWidgetTvShow::updateTvShowInfo()
{
    if (m_show == 0)
        return;

    onClear();

    ui->dir->setText(m_show->dir());
    ui->name->setText(m_show->name());
    ui->rating->setValue(m_show->rating());
    ui->firstAired->setDate(m_show->firstAired());
    ui->studio->setText(m_show->network());
    ui->overview->setText(m_show->overview());

    foreach (const QString &genre, m_show->genres()) {
        int row = ui->genres->rowCount();
        ui->genres->insertRow(row);
        ui->genres->setItem(row, 0, new QTableWidgetItem(genre));
    }

    foreach (const Actor &actor, m_show->actors()) {
        int row = ui->actors->rowCount();
        ui->actors->insertRow(row);
        ui->actors->setItem(row, 0, new QTableWidgetItem(actor.name));
        ui->actors->setItem(row, 1, new QTableWidgetItem(actor.role));
        ui->actors->item(row, 0)->setData(Qt::UserRole, actor.thumb);
    }

    QStringList certifications = m_show->certifications();
    certifications.prepend("");
    ui->certification->addItems(certifications);
    ui->certification->setCurrentIndex(certifications.indexOf(m_show->certification()));

    if (!m_show->posterImage()->isNull()) {
        ui->poster->setPixmap(QPixmap::fromImage(*m_show->posterImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(m_show->posterImage()->width()).arg(m_show->posterImage()->height()));
    } else {
        ui->poster->setPixmap(QPixmap(":/img/film_reel.png"));
        ui->posterResolution->setText("");
    }

    if (!m_show->backdropImage()->isNull()) {
        ui->backdrop->setPixmap(QPixmap::fromImage(*m_show->backdropImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(m_show->backdropImage()->width()).arg(m_show->backdropImage()->height()));
    } else {
        ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->backdropResolution->setText("");
    }

    QMapIterator<int, QList<QWidget*> > it(m_seasonLayoutWidgets);
    while (it.hasNext()) {
        it.next();
        ui->seasonsLayout->removeWidget(it.value().at(0));
        ui->seasonsLayout->removeWidget(it.value().at(1));
        it.value().at(0)->deleteLater();
        it.value().at(1)->deleteLater();
    }
    m_seasonLayoutWidgets.clear();
    int row=0;
    foreach (int season, m_show->seasons()) {
        QLabel *label = new QLabel(tr("Season %1").arg(season));
        MyLabel *poster = new MyLabel;
        poster->setMinimumSize(150, 225);
        poster->setToolTip(tr("Click to Change"));
        poster->setCursor(Qt::PointingHandCursor);
        poster->setSeason(season);
        poster->setAlignment(Qt::AlignCenter);
        if (!m_show->seasonPosterImage(season)->isNull())
            poster->setPixmap(QPixmap::fromImage(*m_show->seasonPosterImage(season)).scaledToWidth(150, Qt::SmoothTransformation));
        else
            poster->setPixmap(QPixmap(":/img/film_reel.png"));
        connect(poster, SIGNAL(seasonClicked(int)), this, SLOT(onChooseSeasonPoster(int)));
        ui->seasonsLayout->setWidget(row, QFormLayout::LabelRole, label);
        ui->seasonsLayout->setWidget(row, QFormLayout::FieldRole, poster);
        row++;
        QList<QWidget*> widgets;
        widgets.append(label);
        widgets.append(poster);
        m_seasonLayoutWidgets.insert(season, widgets);
    }
}

void TvShowWidgetTvShow::onAddGenre()
{
    int row = ui->genres->rowCount();
    ui->genres->insertRow(row);
    ui->genres->setItem(row, 0, new QTableWidgetItem(tr("Unknown Genre")));
    ui->genres->scrollToBottom();
}

void TvShowWidgetTvShow::onRemoveGenre()
{
    int row = ui->genres->currentRow();
    if (row < 0 || row >= ui->genres->rowCount() || !ui->genres->currentItem()->isSelected())
        return;
    ui->genres->removeRow(row);
}

void TvShowWidgetTvShow::onAddActor()
{
    int row = ui->actors->rowCount();
    ui->actors->insertRow(row);
    ui->actors->setItem(row, 0, new QTableWidgetItem(tr("Unknown Actor")));
    ui->actors->setItem(row, 1, new QTableWidgetItem(tr("Unkown Role")));
    ui->actors->scrollToBottom();
}

void TvShowWidgetTvShow::onRemoveActor()
{
    int row = ui->actors->currentRow();
    if (row < 0 || row >= ui->actors->rowCount() || !ui->actors->currentItem()->isSelected())
        return;
    ui->actors->removeRow(row);
}

void TvShowWidgetTvShow::onSaveInformation()
{
    if (m_show == 0)
        return;

    onSetEnabled(false);
    m_savingWidget->show();

    m_show->setCertification(ui->certification->currentText());
    m_show->setFirstAired(ui->firstAired->date());
    m_show->setName(ui->name->text());
    m_show->setNetwork(ui->studio->text());
    m_show->setOverview(ui->overview->toPlainText());
    m_show->setRating(ui->rating->value());
    m_show->setShowTitle(ui->name->text());

    QList<Actor> actors;
    for (int i=0, n=ui->actors->rowCount() ; i<n ; i++) {
        Actor a;
        a.name = ui->actors->item(i, 0)->text();
        a.role = ui->actors->item(i, 1)->text();
        a.thumb = ui->actors->item(i, 0)->data(Qt::UserRole).toString();
        actors.append(a);
    }
    m_show->setActors(actors);

    if (!m_chosenPoster.isNull())
        m_show->setPosterImage(m_chosenPoster);
    if (!m_chosenBackdrop.isNull())
        m_show->setBackdropImage(m_chosenBackdrop);
    QMapIterator<int, QImage> it(m_chosenSeasonPosters);
    while (it.hasNext()) {
        it.next();
        m_show->setSeasonPosterImage(it.key(), it.value());
    }

    if (m_loadedFromScraper) {
        QList<DownloadManagerElement> downloads;
        QList<Actor*> actors = m_show->actorsPointer();
        for (int i=0, n=actors.size() ; i<n ; ++i) {
            if (actors.at(i)->thumb.isEmpty())
                continue;
            DownloadManagerElement d;
            d.imageType = TypeActor;
            d.url = QUrl(actors.at(i)->thumb);
            d.actor = actors.at(i);
            downloads.append(d);
        }
        foreach (TvShowEpisode *episode, m_show->episodes()) {
            if (episode->thumbnail().isEmpty())
                continue;
            DownloadManagerElement d;
            d.imageType = TypeShowThumbnail;
            d.url = episode->thumbnail();
            d.episode = episode;
            downloads.append(d);
        }
        m_currentDownloadsSize = downloads.count();
        m_posterDownloadManager->setDownloads(downloads);
        emit sigDownloadsStarted(tr("Downloading Missing Actor Images and Episode Thumbnails..."), m_progressMessageId);
        connect(m_posterDownloadManager, SIGNAL(allDownloadsFinished()), this, SLOT(onDownloadsFinished()), Qt::UniqueConnection);
    } else {
        onDownloadsFinished();
    }
}

void TvShowWidgetTvShow::onStartScraperSearch()
{
    if (m_show == 0)
        return;
    emit sigSetActionSearchEnabled(false, WidgetTvShows);
    emit sigSetActionSaveEnabled(false, WidgetTvShows);
    TvShowSearch::instance()->exec(m_show->name());
    if (TvShowSearch::instance()->result() == QDialog::Accepted) {
        onSetEnabled(false);
        m_show->loadData(TvShowSearch::instance()->scraperId(), Manager::instance()->tvScrapers().at(0));
        connect(m_show, SIGNAL(sigLoaded()), this, SLOT(onLoadDone()), Qt::UniqueConnection);
    } else {
        emit sigSetActionSearchEnabled(true, WidgetTvShows);
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
    }
}

void TvShowWidgetTvShow::onLoadDone()
{
    if (m_show == 0)
        return;

    updateTvShowInfo();
    onSetEnabled(true);
    m_loadedFromScraper = true;

    if (m_show->posters().size() > 0) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypePoster;
        d.url = m_show->posters().at(0).originalUrl;
        m_posterDownloadManager->addDownload(d);
        ui->poster->setPixmap(QPixmap());
        ui->poster->setMovie(m_loadingMovie);
    }

    if (m_show->backdrops().size() > 0) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = m_show->backdrops().at(0).originalUrl;
        m_posterDownloadManager->addDownload(d);
        ui->backdrop->setPixmap(QPixmap());
        ui->backdrop->setMovie(m_loadingMovie);
    }

    QList<DownloadManagerElement> downloads;
    foreach (int season, m_show->seasons()) {
        if (!m_show->seasonPosters(season).isEmpty() && m_seasonLayoutWidgets.contains(season)) {
            emit sigSetActionSaveEnabled(false, WidgetTvShows);
            DownloadManagerElement d;
            d.imageType = TypeSeasonPoster;
            d.url = m_show->seasonPosters(season).at(0).originalUrl;
            d.season = season;
            downloads.append(d);
            static_cast<MyLabel*>(m_seasonLayoutWidgets[season].at(1))->setPixmap(QPixmap());
            static_cast<MyLabel*>(m_seasonLayoutWidgets[season].at(1))->setMovie(m_loadingMovie);
        }
    }
    foreach (DownloadManagerElement d, downloads)
        m_posterDownloadManager->addDownload(d);
}

void TvShowWidgetTvShow::onChoosePoster()
{
    if (m_show == 0)
        return;

    MovieImageDialog::instance()->setImageType(TypePoster);
    MovieImageDialog::instance()->clear();
    MovieImageDialog::instance()->setDownloads(m_show->posters());
    MovieImageDialog::instance()->exec();

    if (MovieImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypePoster;
        d.url = MovieImageDialog::instance()->imageUrl();
        m_posterDownloadManager->addDownload(d);
        ui->poster->setPixmap(QPixmap());
        ui->poster->setMovie(m_loadingMovie);
    }
}

void TvShowWidgetTvShow::onChooseSeasonPoster(int season)
{
    if (m_show == 0)
        return;

    MovieImageDialog::instance()->setImageType(TypePoster);
    MovieImageDialog::instance()->clear();
    MovieImageDialog::instance()->setDownloads(m_show->seasonPosters(season));
    MovieImageDialog::instance()->exec();
    if (MovieImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeSeasonPoster;
        d.url = MovieImageDialog::instance()->imageUrl();
        d.season = season;
        m_posterDownloadManager->addDownload(d);
        if (m_seasonLayoutWidgets.contains(season)) {
            static_cast<MyLabel*>(m_seasonLayoutWidgets[season].at(1))->setPixmap(QPixmap());
            static_cast<MyLabel*>(m_seasonLayoutWidgets[season].at(1))->setMovie(m_loadingMovie);
        }
    }
}

void TvShowWidgetTvShow::onChooseBackdrop()
{
    if (m_show == 0)
        return;

    MovieImageDialog::instance()->setImageType(TypeBackdrop);
    MovieImageDialog::instance()->clear();
    MovieImageDialog::instance()->setDownloads(m_show->backdrops());
    MovieImageDialog::instance()->exec();

    if (MovieImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = MovieImageDialog::instance()->imageUrl();
        m_posterDownloadManager->addDownload(d);
        ui->backdrop->setPixmap(QPixmap());
        ui->backdrop->setMovie(m_loadingMovie);
    }
}

void TvShowWidgetTvShow::onPosterDownloadFinished(DownloadManagerElement elem)
{
    if (elem.imageType == TypePoster) {
        m_chosenPoster = elem.image;
        ui->poster->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
    } else if (elem.imageType == TypeBackdrop) {
        m_chosenBackdrop = elem.image;
        ui->backdrop->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
    } else if (elem.imageType == TypeSeasonPoster) {
        int season = elem.season;
        if (m_seasonLayoutWidgets.contains(season)) {
            static_cast<MyLabel*>(m_seasonLayoutWidgets[season].at(1))->setPixmap(QPixmap::fromImage(elem.image).scaled(150, 225, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            if (!m_chosenSeasonPosters.contains(season))
                m_chosenSeasonPosters.insert(season, QImage());
            m_chosenSeasonPosters[season] = elem.image;
        }
    }
    if (m_posterDownloadManager->downloadQueueSize() == 0)
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
}

void TvShowWidgetTvShow::onDownloadsFinished()
{
    emit sigDownloadsFinished(m_progressMessageId);
    m_show->saveData(Manager::instance()->mediaCenterInterface());
    onSetEnabled(true);
    m_savingWidget->hide();
    MessageBox::instance()->showMessage(tr("TV Show and all Episodes Saved"));
}

void TvShowWidgetTvShow::onDownloadsLeft(int left)
{
    emit sigDownloadsProgress(m_currentDownloadsSize-left, m_currentDownloadsSize, m_progressMessageId);
}
