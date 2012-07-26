#include "TvShowWidgetTvShow.h"
#include "ui_TvShowWidgetTvShow.h"

#include <QFileDialog>
#include <QMovie>
#include <QPainter>
#include "ImagePreviewDialog.h"
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
    ui->bannerResolution->clear();
    ui->genres->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->actors->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->actors->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    QFont font = ui->showTitle->font();
    font.setPointSize(font.pointSize()+4);
    ui->showTitle->setFont(font);
    ui->buttonPreviewBackdrop->setEnabled(false);
    ui->buttonPreviewPoster->setEnabled(false);
    ui->buttonPreviewBanner->setEnabled(false);

    font = ui->posterResolution->font();
    #ifdef Q_WS_WIN
    font.setPointSize(font.pointSize()-1);
    #else
    font.setPointSize(font.pointSize()-2);
    #endif
    ui->posterResolution->setFont(font);
    ui->backdropResolution->setFont(font);
    ui->bannerResolution->setFont(font);
    ui->actorResolution->setFont(font);

    m_loadingMovie = new QMovie(":/img/spinner.gif");
    m_loadingMovie->start();
    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    m_posterDownloadManager = new DownloadManager(this);

    connect(ui->name, SIGNAL(textChanged(QString)), ui->showTitle, SLOT(setText(QString)));
    connect(ui->buttonAddGenre, SIGNAL(clicked()), this, SLOT(onAddGenre()));
    connect(ui->buttonRemoveGenre, SIGNAL(clicked()), this, SLOT(onRemoveGenre()));
    connect(ui->buttonAddActor, SIGNAL(clicked()), this, SLOT(onAddActor()));
    connect(ui->buttonRemoveActor, SIGNAL(clicked()), this, SLOT(onRemoveActor()));
    connect(ui->poster, SIGNAL(clicked()), this, SLOT(onChoosePoster()));
    connect(ui->backdrop, SIGNAL(clicked()), this, SLOT(onChooseBackdrop()));
    connect(ui->banner, SIGNAL(clicked()), this, SLOT(onChooseBanner()));
    connect(m_posterDownloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(onPosterDownloadFinished(DownloadManagerElement)));
    connect(m_posterDownloadManager, SIGNAL(downloadsLeft(int,DownloadManagerElement)), this, SLOT(onDownloadsLeft(int,DownloadManagerElement)));
    connect(ui->buttonPreviewPoster, SIGNAL(clicked()), this, SLOT(onPreviewPoster()));
    connect(ui->buttonPreviewBackdrop, SIGNAL(clicked()), this, SLOT(onPreviewBackdrop()));
    connect(ui->buttonPreviewBanner, SIGNAL(clicked()), this, SLOT(onPreviewBanner()));
    connect(ui->actors, SIGNAL(itemSelectionChanged()), this, SLOT(onActorChanged()));
    connect(ui->actor, SIGNAL(clicked()), this, SLOT(onChangeActorImage()));

    onClear();

    // Connect GUI change events to movie object
    connect(ui->name, SIGNAL(textEdited(QString)), this, SLOT(onNameChange(QString)));
    connect(ui->certification, SIGNAL(editTextChanged(QString)), this, SLOT(onCertificationChange(QString)));
    connect(ui->rating, SIGNAL(valueChanged(double)), this, SLOT(onRatingChange(double)));
    connect(ui->firstAired, SIGNAL(dateChanged(QDate)), this, SLOT(onFirstAiredChange(QDate)));
    connect(ui->studio, SIGNAL(textEdited(QString)), this, SLOT(onStudioChange(QString)));
    connect(ui->overview, SIGNAL(textChanged()), this, SLOT(onOverviewChange()));
    connect(ui->actors, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onActorEdited(QTableWidgetItem*)));
    connect(ui->genres, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onGenreEdited(QTableWidgetItem*)));

    onSetEnabled(false);

    QPixmap zoomIn(":/img/zoom_in.png");
    QPainter p;
    p.begin(&zoomIn);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(zoomIn.rect(), QColor(0, 0, 0, 150));
    p.end();
    ui->buttonPreviewBackdrop->setIcon(QIcon(zoomIn));
    ui->buttonPreviewPoster->setIcon(QIcon(zoomIn));
    ui->buttonPreviewBanner->setIcon(QIcon(zoomIn));
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
    ui->certification->clear();
    ui->firstAired->setDate(QDate::currentDate());
    ui->studio->clear();
    ui->overview->clear();
    ui->poster->setPixmap(QPixmap(":/img/film_reel.png"));
    ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt_small.png"));
    ui->tabWidget->setCurrentIndex(0);
    ui->posterResolution->clear();
    ui->backdropResolution->clear();
    ui->bannerResolution->clear();

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
}

void TvShowWidgetTvShow::setTvShow(TvShow *show)
{
    m_show = show;
    show->loadImages(Manager::instance()->mediaCenterInterface());
    updateTvShowInfo();
    if (show->downloadsInProgress()) {
        onSetEnabled(false);
        emit sigSetActionSearchEnabled(false, WidgetTvShows);
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
    } else {
        onSetEnabled(true);
        emit sigSetActionSearchEnabled(true, WidgetTvShows);
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
    }

}

void TvShowWidgetTvShow::updateTvShowInfo()
{
    if (m_show == 0)
        return;

    ui->certification->blockSignals(true);
    ui->rating->blockSignals(true);
    ui->firstAired->blockSignals(true);
    ui->overview->blockSignals(true);

    onClear();

    ui->dir->setText(m_show->dir());
    ui->name->setText(m_show->name());
    ui->rating->setValue(m_show->rating());
    ui->firstAired->setDate(m_show->firstAired());
    ui->studio->setText(m_show->network());
    ui->overview->setPlainText(m_show->overview());

    ui->actors->blockSignals(true);
    foreach (Actor *actor, m_show->actorsPointer()) {
        int row = ui->actors->rowCount();
        ui->actors->insertRow(row);
        ui->actors->setItem(row, 0, new QTableWidgetItem(actor->name));
        ui->actors->setItem(row, 1, new QTableWidgetItem(actor->role));
        ui->actors->item(row, 0)->setData(Qt::UserRole, actor->thumb);
        ui->actors->item(row, 1)->setData(Qt::UserRole, QVariant::fromValue(actor));
    }
    ui->actors->blockSignals(false);

    ui->genres->blockSignals(true);
    foreach (QString *genre, m_show->genresPointer()) {
        int row = ui->genres->rowCount();
        ui->genres->insertRow(row);
        ui->genres->setItem(row, 0, new QTableWidgetItem(*genre));
        ui->genres->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(genre));
    }
    ui->genres->blockSignals(false);

    QStringList certifications = m_show->certifications();
    certifications.prepend("");
    ui->certification->addItems(certifications);
    ui->certification->setCurrentIndex(certifications.indexOf(m_show->certification()));

    if (!m_show->posterImage()->isNull()) {
        ui->poster->setPixmap(QPixmap::fromImage(*m_show->posterImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(m_show->posterImage()->width()).arg(m_show->posterImage()->height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = *m_show->posterImage();
    } else {
        ui->poster->setPixmap(QPixmap(":/img/film_reel.png"));
        ui->posterResolution->setText("");
        ui->buttonPreviewPoster->setEnabled(false);
    }

    if (!m_show->backdropImage()->isNull()) {
        ui->backdrop->setPixmap(QPixmap::fromImage(*m_show->backdropImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(m_show->backdropImage()->width()).arg(m_show->backdropImage()->height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = *m_show->backdropImage();
    } else {
        ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->backdropResolution->setText("");
        ui->buttonPreviewBackdrop->setEnabled(false);
    }

    if (!m_show->bannerImage()->isNull()) {
        ui->banner->setPixmap(QPixmap::fromImage(*m_show->bannerImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->bannerResolution->setText(QString("%1x%2").arg(m_show->bannerImage()->width()).arg(m_show->bannerImage()->height()));
        ui->buttonPreviewBanner->setEnabled(true);
        m_currentBanner = *m_show->bannerImage();
    } else {
        ui->banner->setPixmap(QPixmap(":/img/pictures_alt_small.png"));
        ui->bannerResolution->setText("");
        ui->buttonPreviewBanner->setEnabled(false);
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

    ui->certification->blockSignals(false);
    ui->rating->blockSignals(false);
    ui->firstAired->blockSignals(false);
    ui->overview->blockSignals(false);
}

void TvShowWidgetTvShow::onSaveInformation()
{
    if (m_show == 0)
        return;

    onSetEnabled(false);
    m_savingWidget->show();
    m_show->saveData(Manager::instance()->mediaCenterInterface());
    m_savingWidget->hide();
    onSetEnabled(true);
    MessageBox::instance()->showMessage(tr("<b>\"%1\"</b> Saved").arg(m_show->name()));
}

void TvShowWidgetTvShow::onStartScraperSearch()
{
    if (m_show == 0)
        return;
    emit sigSetActionSaveEnabled(false, WidgetTvShows);
    emit sigSetActionSearchEnabled(false, WidgetTvShows);
    TvShowSearch::instance()->setChkUpdateAllVisible(true);
    TvShowSearch::instance()->exec(m_show->name());
    if (TvShowSearch::instance()->result() == QDialog::Accepted) {
        onSetEnabled(false);
        m_show->loadData(TvShowSearch::instance()->scraperId(), Manager::instance()->tvScrapers().at(0), TvShowSearch::instance()->updateAll());
        connect(m_show, SIGNAL(sigLoaded(TvShow*)), this, SLOT(onLoadDone(TvShow*)), Qt::UniqueConnection);
    } else {
        emit sigSetActionSearchEnabled(true, WidgetTvShows);
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
    }
}

void TvShowWidgetTvShow::onLoadDone(TvShow *show)
{
    if (m_show == 0)
        return;

    if (m_show == show)
        updateTvShowInfo();

    int downloadsSize = 0;
    if (show->posters().size() > 0) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypePoster;
        d.url = show->posters().at(0).originalUrl;
        d.show = show;
        m_posterDownloadManager->addDownload(d);
        downloadsSize++;
        if (m_show == show) {
            ui->poster->setPixmap(QPixmap());
            ui->poster->setMovie(m_loadingMovie);
        }
    }

    if (show->backdrops().size() > 0) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = show->backdrops().at(0).originalUrl;
        d.show = show;
        m_posterDownloadManager->addDownload(d);
        downloadsSize++;
        if (m_show == show) {
            ui->backdrop->setPixmap(QPixmap());
            ui->backdrop->setMovie(m_loadingMovie);
        }
    }

    if (show->banners().size() > 0) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeBanner;
        d.url = show->banners().at(0).originalUrl;
        d.show = show;
        m_posterDownloadManager->addDownload(d);
        downloadsSize++;
        if (m_show == show) {
            ui->banner->setPixmap(QPixmap());
            ui->banner->setMovie(m_loadingMovie);
        }
    }

    QList<Actor*> actors = show->actorsPointer();
    for (int i=0, n=actors.size() ; i<n ; ++i) {
        if (actors.at(i)->thumb.isEmpty())
            continue;
        DownloadManagerElement d;
        d.imageType = TypeActor;
        d.url = QUrl(actors.at(i)->thumb);
        d.actor = actors.at(i);
        d.show = show;
        m_posterDownloadManager->addDownload(d);
        downloadsSize++;
    }

    foreach (int season, show->seasons()) {
        if (!show->seasonPosters(season).isEmpty()) {
            emit sigSetActionSaveEnabled(false, WidgetTvShows);
            DownloadManagerElement d;
            d.imageType = TypeSeasonPoster;
            d.url = show->seasonPosters(season).at(0).originalUrl;
            d.season = season;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            downloadsSize++;
            if (m_show == show && m_seasonLayoutWidgets.contains(season)) {
                static_cast<MyLabel*>(m_seasonLayoutWidgets[season].at(1))->setPixmap(QPixmap());
                static_cast<MyLabel*>(m_seasonLayoutWidgets[season].at(1))->setMovie(m_loadingMovie);
            }
        }
    }

    foreach (TvShowEpisode *episode, show->episodes()) {
        if (episode->thumbnail().isEmpty() || !episode->hasChanged())
            continue;
        DownloadManagerElement d;
        d.imageType = TypeShowThumbnail;
        d.url = episode->thumbnail();
        d.episode = episode;
        d.show = show;
        m_posterDownloadManager->addDownload(d);
        downloadsSize++;
    }

    show->setDownloadsInProgress(downloadsSize > 0);

    if (downloadsSize > 0) {
        emit sigDownloadsStarted(tr("Downloading Missing Actor Images and Episode Thumbnails..."), Constants::TvShowProgressMessageId+show->showId());
        connect(m_posterDownloadManager, SIGNAL(allDownloadsFinished(TvShow*)), this, SLOT(onDownloadsFinished(TvShow*)), Qt::UniqueConnection);
    } else if (show == m_show) {
        onSetEnabled(true);
        emit sigSetActionSearchEnabled(true, WidgetTvShows);
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
    }
}

void TvShowWidgetTvShow::onChoosePoster()
{
    if (m_show == 0)
        return;

    MovieImageDialog::instance()->setImageType(TypePoster);
    MovieImageDialog::instance()->clear();
    MovieImageDialog::instance()->setDownloads(m_show->posters());
    MovieImageDialog::instance()->exec(MovieImageDialogType::TvShowPoster);

    if (MovieImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypePoster;
        d.url = MovieImageDialog::instance()->imageUrl();
        d.show = m_show;
        m_posterDownloadManager->addDownload(d);
        ui->poster->setPixmap(QPixmap());
        ui->poster->setMovie(m_loadingMovie);
        ui->buttonPreviewPoster->setEnabled(false);
    }
}

void TvShowWidgetTvShow::onChooseSeasonPoster(int season)
{
    if (m_show == 0)
        return;

    MovieImageDialog::instance()->setImageType(TypePoster);
    MovieImageDialog::instance()->clear();
    MovieImageDialog::instance()->setDownloads(m_show->seasonPosters(season));
    MovieImageDialog::instance()->exec(MovieImageDialogType::TvShowSeason);
    if (MovieImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeSeasonPoster;
        d.url = MovieImageDialog::instance()->imageUrl();
        d.season = season;
        d.show = m_show;
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
    MovieImageDialog::instance()->exec(MovieImageDialogType::TvShowBackdrop);

    if (MovieImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = MovieImageDialog::instance()->imageUrl();
        d.show = m_show;
        m_posterDownloadManager->addDownload(d);
        ui->backdrop->setPixmap(QPixmap());
        ui->backdrop->setMovie(m_loadingMovie);
        ui->buttonPreviewBackdrop->setEnabled(false);
    }
}

void TvShowWidgetTvShow::onChooseBanner()
{
    if (m_show == 0)
        return;

    MovieImageDialog::instance()->setImageType(TypeBanner);
    MovieImageDialog::instance()->clear();
    MovieImageDialog::instance()->setDownloads(m_show->banners());
    MovieImageDialog::instance()->exec(MovieImageDialogType::TvShowBanner);

    if (MovieImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeBanner;
        d.url = MovieImageDialog::instance()->imageUrl();
        d.show = m_show;
        m_posterDownloadManager->addDownload(d);
        ui->banner->setPixmap(QPixmap());
        ui->banner->setMovie(m_loadingMovie);
        ui->buttonPreviewBanner->setEnabled(false);
    }
}

void TvShowWidgetTvShow::onPosterDownloadFinished(DownloadManagerElement elem)
{
    if (elem.imageType == TypePoster) {
        if (m_show == elem.show) {
            ui->poster->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->posterResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
            ui->buttonPreviewPoster->setEnabled(true);
            m_currentPoster = elem.image;
        }
        elem.show->setPosterImage(elem.image);
    } else if (elem.imageType == TypeBackdrop) {
        if ((elem.image.width() != 1920 || elem.image.height() != 1080) &&
            elem.image.width() > 1915 && elem.image.width() < 1925 && elem.image.height() > 1075 && elem.image.height() < 1085)
            elem.image = elem.image.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        if ((elem.image.width() != 1280 || elem.image.height() != 720) &&
            elem.image.width() > 1275 && elem.image.width() < 1285 && elem.image.height() > 715 && elem.image.height() < 725)
            elem.image = elem.image.scaled(1280, 720, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        if (m_show == elem.show) {
            ui->backdrop->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->backdropResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
            ui->buttonPreviewBackdrop->setEnabled(true);
            m_currentBackdrop = elem.image;
        }
        elem.show->setBackdropImage(elem.image);
    } else if (elem.imageType == TypeBanner) {
        if (m_show == elem.show) {
            ui->banner->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 37, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->bannerResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
            ui->buttonPreviewBanner->setEnabled(true);
            m_currentBanner = elem.image;
        }
        elem.show->setBannerImage(elem.image);
    } else if (elem.imageType == TypeSeasonPoster) {
        int season = elem.season;
        elem.show->setSeasonPosterImage(season, elem.image);
        if (m_show == elem.show && m_seasonLayoutWidgets.contains(season)) {
            static_cast<MyLabel*>(m_seasonLayoutWidgets[season].at(1))->setPixmap(QPixmap::fromImage(elem.image).scaled(150, 225, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}

void TvShowWidgetTvShow::onDownloadsFinished(TvShow *show)
{
    emit sigDownloadsFinished(Constants::TvShowProgressMessageId+show->showId());
    if (show == m_show) {
        onSetEnabled(true);
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
        emit sigSetActionSearchEnabled(true, WidgetTvShows);
    }
    show->setDownloadsInProgress(false);
}

void TvShowWidgetTvShow::onDownloadsLeft(int left, DownloadManagerElement elem)
{
    emit sigDownloadsProgress(elem.show->actors().size()+elem.show->episodes().size()-left, elem.show->actors().size()+elem.show->episodes().size(),
                              Constants::TvShowProgressMessageId+elem.show->showId());
}

/*** add/remove/edit Actors, Genres, Countries and Studios ***/

void TvShowWidgetTvShow::onGenreEdited(QTableWidgetItem *item)
{
    QString *genre = ui->genres->item(item->row(), 0)->data(Qt::UserRole).value<QString*>();
    genre->clear();
    genre->append(item->text());
    m_show->setChanged(true);
}

void TvShowWidgetTvShow::onAddGenre()
{
    QString g = tr("Unknown Genre");
    m_show->addGenre(g);
    QString *genre = m_show->genresPointer().last();

    ui->genres->blockSignals(true);
    int row = ui->genres->rowCount();
    ui->genres->insertRow(row);
    ui->genres->setItem(row, 0, new QTableWidgetItem(g));
    ui->genres->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(genre));
    ui->genres->scrollToBottom();
    ui->genres->blockSignals(false);
}

void TvShowWidgetTvShow::onRemoveGenre()
{
    int row = ui->genres->currentRow();
    if (row < 0 || row >= ui->genres->rowCount() || !ui->genres->currentItem()->isSelected())
        return;

    QString *genre = ui->genres->item(row, 0)->data(Qt::UserRole).value<QString*>();
    m_show->removeGenre(genre);
    ui->genres->blockSignals(true);
    ui->genres->removeRow(row);
    ui->genres->blockSignals(false);
}

void TvShowWidgetTvShow::onActorEdited(QTableWidgetItem *item)
{
    Actor *actor = ui->actors->item(item->row(), 1)->data(Qt::UserRole).value<Actor*>();
    if (item->column() == 0)
        actor->name = item->text();
    else if (item->column() == 1)
        actor->role = item->text();
    m_show->setChanged(true);
}

void TvShowWidgetTvShow::onAddActor()
{
    Actor a;
    a.name = tr("Unknown Actor");
    a.role = tr("Unknown Role");
    m_show->addActor(a);

    Actor *actor = m_show->actorsPointer().last();

    ui->actors->blockSignals(true);
    int row = ui->actors->rowCount();
    ui->actors->insertRow(row);
    ui->actors->setItem(row, 0, new QTableWidgetItem(actor->name));
    ui->actors->setItem(row, 1, new QTableWidgetItem(actor->role));
    ui->actors->item(row, 1)->setData(Qt::UserRole, QVariant::fromValue(actor));
    ui->actors->scrollToBottom();
    ui->actors->blockSignals(false);
}

void TvShowWidgetTvShow::onRemoveActor()
{
    int row = ui->actors->currentRow();
    if (row < 0 || row >= ui->actors->rowCount() || !ui->actors->currentItem()->isSelected())
        return;

    Actor *actor = ui->actors->item(row, 1)->data(Qt::UserRole).value<Actor*>();
    m_show->removeActor(actor);
    ui->actors->blockSignals(true);
    ui->actors->removeRow(row);
    ui->actors->blockSignals(false);
}

void TvShowWidgetTvShow::onPreviewBackdrop()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentBackdrop));
    ImagePreviewDialog::instance()->exec();
}

void TvShowWidgetTvShow::onPreviewPoster()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentPoster));
    ImagePreviewDialog::instance()->exec();
}

void TvShowWidgetTvShow::onPreviewBanner()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentBanner));
    ImagePreviewDialog::instance()->exec();
}

void TvShowWidgetTvShow::onActorChanged()
{
    if (ui->actors->currentRow() < 0 || ui->actors->currentRow() >= ui->actors->rowCount() ||
        ui->actors->currentColumn() < 0 || ui->actors->currentColumn() >= ui->actors->colorCount()) {
        ui->actor->setPixmap(QPixmap(":/img/man.png"));
        ui->actorResolution->setText("");
        return;
    }

    Actor *actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
    if (actor->image.isNull()) {
        ui->actor->setPixmap(QPixmap(":/img/man.png"));
        ui->actorResolution->setText("");
        return;
    }
    ui->actor->setPixmap(QPixmap::fromImage(actor->image).scaled(120, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->actorResolution->setText(QString("%1 x %2").arg(actor->image.width()).arg(actor->image.height()));
}

void TvShowWidgetTvShow::onChangeActorImage()
{
    if (ui->actors->currentRow() < 0 || ui->actors->currentRow() >= ui->actors->rowCount() ||
        ui->actors->currentColumn() < 0 || ui->actors->currentColumn() >= ui->actors->colorCount()) {
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(parentWidget(), tr("Choose Image"), QDir::homePath(), tr("Images (*.jpg *.jpeg)"));
    if (!fileName.isNull()) {
        QImage img(fileName);
        if (!img.isNull()) {
            Actor *actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
            actor->image.load(fileName);
            actor->imageHasChanged = true;
            onActorChanged();
            m_show->setChanged(true);
        }
    }
}

/*** Pass GUI events to tv show object ***/

void TvShowWidgetTvShow::onNameChange(QString text)
{
    m_show->setName(text);
}

void TvShowWidgetTvShow::onCertificationChange(QString text)
{
    m_show->setCertification(text);
}

void TvShowWidgetTvShow::onRatingChange(double value)
{
    m_show->setRating(value);
}

void TvShowWidgetTvShow::onFirstAiredChange(QDate date)
{
    m_show->setFirstAired(date);
}

void TvShowWidgetTvShow::onStudioChange(QString studio)
{
    m_show->setNetwork(studio);
}

void TvShowWidgetTvShow::onOverviewChange()
{
    m_show->setOverview(ui->overview->toPlainText());
}
