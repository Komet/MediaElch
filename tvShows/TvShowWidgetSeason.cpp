#include "TvShowWidgetSeason.h"
#include "ui_TvShowWidgetSeason.h"

#include <QPainter>
#include "data/ImageCache.h"
#include "globals/Helper.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "main/MessageBox.h"

TvShowWidgetSeason::TvShowWidgetSeason(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TvShowWidgetSeason)
{
    ui->setupUi(this);
    m_show = 0;
    m_season = -1;

    ui->title->clear();
    QFont font = ui->title->font();
    font.setPointSize(font.pointSize()+4);
    ui->title->setFont(font);

    font = ui->labelPoster->font();
    #ifdef Q_OS_WIN32
        font.setPointSize(font.pointSize()-1);
    #else
        font.setPointSize(font.pointSize()-2);
    #endif

    font.setBold(true);
    ui->labelFanart->setFont(font);
    ui->labelBanner->setFont(font);
    ui->labelPoster->setFont(font);
    ui->labelThumb->setFont(font);

    ui->poster->setDefaultPixmap(QPixmap(":/img/film_reel.png"));
    ui->backdrop->setDefaultPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->banner->setDefaultPixmap(QPixmap(":/img/pictures_alt.png").scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->thumb->setDefaultPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    m_downloadManager = new DownloadManager(this);

    m_loadingMovie = new QMovie(":/img/spinner.gif");
    m_loadingMovie->start();

    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    onSetEnabled(false);
    onClear();

    QPainter p;
    QPixmap revert(":/img/arrow_circle_left.png");
    p.begin(&revert);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(revert.rect(), QColor(0, 0, 0, 200));
    p.end();
    ui->buttonRevert->setIcon(QIcon(revert));
    ui->buttonRevert->setVisible(false);

    ui->poster->setImageType(ImageType::TvShowPoster);
    ui->backdrop->setImageType(ImageType::TvShowBackdrop);
    ui->banner->setImageType(ImageType::TvShowBanner);
    ui->thumb->setImageType(ImageType::TvShowThumb);
    foreach (ClosableImage *image, ui->groupBox_3->findChildren<ClosableImage*>()) {
        connect(image, SIGNAL(clicked()), this, SLOT(onChooseImage()));
        connect(image, SIGNAL(sigClose()), this, SLOT(onDeleteImage()));
    }

    connect(ui->buttonRevert, SIGNAL(clicked()), this, SLOT(onRevertChanges()));
    connect(m_downloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(onDownloadFinished(DownloadManagerElement)));
}

TvShowWidgetSeason::~TvShowWidgetSeason()
{
    delete ui;
}

void TvShowWidgetSeason::resizeEvent(QResizeEvent *event)
{
    m_savingWidget->move(size().width()/2-m_savingWidget->width(), height()/2-m_savingWidget->height());
    QWidget::resizeEvent(event);
}

void TvShowWidgetSeason::setSeason(TvShow *show, int season)
{
    onClear();

    m_show = show;
    m_season = season;

    emit sigSetActionSearchEnabled(false, WidgetTvShows);
    ui->title->setText(QString(show->name()) + " - " + tr("Season %1").arg(season));

    updateImages(QList<int>() << ImageType::TvShowSeasonPoster << ImageType::TvShowSeasonBackdrop << ImageType::TvShowSeasonBanner << ImageType::TvShowSeasonThumb);

    onSetEnabled(!show->downloadsInProgress());
    emit sigSetActionSaveEnabled(!show->downloadsInProgress(), WidgetTvShows);
}

void TvShowWidgetSeason::updateImages(QList<int> images)
{
    if (images.contains(ImageType::TvShowSeasonPoster)) {
        if (!m_show->seasonPosterImage(m_season).isNull())
            ui->poster->setImage(m_show->seasonPosterImage(m_season));
        else if (!Manager::instance()->mediaCenterInterfaceTvShow()->imageFileName(m_show, ImageType::TvShowSeasonPoster, m_season).isEmpty() &&
                (!m_show->imagesToRemove().contains(ImageType::TvShowSeasonPoster) || !m_show->imagesToRemove().value(ImageType::TvShowSeasonPoster).contains(m_season)))
            ui->poster->setImage(Manager::instance()->mediaCenterInterfaceTvShow()->imageFileName(m_show, ImageType::TvShowSeasonPoster, m_season));
    }

    if (images.contains(ImageType::TvShowSeasonBackdrop)) {
        if (!m_show->seasonBackdropImage(m_season).isNull())
            ui->backdrop->setImage(m_show->seasonBackdropImage(m_season));
        else if (!Manager::instance()->mediaCenterInterfaceTvShow()->imageFileName(m_show, ImageType::TvShowSeasonBackdrop, m_season).isEmpty() &&
                (!m_show->imagesToRemove().contains(ImageType::TvShowSeasonBackdrop) || !m_show->imagesToRemove().value(ImageType::TvShowSeasonBackdrop).contains(m_season)))
            ui->backdrop->setImage(Manager::instance()->mediaCenterInterfaceTvShow()->imageFileName(m_show, ImageType::TvShowSeasonBackdrop, m_season));
    }

    if (images.contains(ImageType::TvShowSeasonBanner)) {
        if (!m_show->seasonBannerImage(m_season).isNull())
            ui->banner->setImage(m_show->seasonBannerImage(m_season));
        else if (!Manager::instance()->mediaCenterInterfaceTvShow()->imageFileName(m_show, ImageType::TvShowSeasonBanner, m_season).isEmpty() &&
                (!m_show->imagesToRemove().contains(ImageType::TvShowSeasonBanner) || !m_show->imagesToRemove().value(ImageType::TvShowSeasonBanner).contains(m_season)))
            ui->banner->setImage(Manager::instance()->mediaCenterInterfaceTvShow()->imageFileName(m_show, ImageType::TvShowSeasonBanner, m_season));
    }

    if (images.contains(ImageType::TvShowSeasonThumb)) {
        if (!m_show->seasonThumbImage(m_season).isNull())
            ui->thumb->setImage(m_show->seasonThumbImage(m_season));
        else if (!Manager::instance()->mediaCenterInterfaceTvShow()->imageFileName(m_show, ImageType::TvShowSeasonThumb, m_season).isEmpty() &&
                (!m_show->imagesToRemove().contains(ImageType::TvShowSeasonThumb) || !m_show->imagesToRemove().value(ImageType::TvShowSeasonThumb).contains(m_season)))
            ui->thumb->setImage(Manager::instance()->mediaCenterInterfaceTvShow()->imageFileName(m_show, ImageType::TvShowSeasonThumb, m_season));
    }
}

void TvShowWidgetSeason::onClear()
{
    ui->title->clear();
    ui->poster->clear();
    ui->backdrop->clear();
    ui->banner->clear();
    ui->thumb->clear();
    ui->buttonRevert->setVisible(false);
}

void TvShowWidgetSeason::onSaveInformation()
{
    if (!m_show)
        return;
    onSetEnabled(false);
    m_savingWidget->show();
    m_show->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
    m_savingWidget->hide();
    onSetEnabled(true);
    MessageBox::instance()->showMessage(tr("<b>\"%1\"</b> Saved").arg(m_show->name()));
}

void TvShowWidgetSeason::onSetEnabled(bool enabled)
{
    ui->groupBox_3->setEnabled(enabled);
}

void TvShowWidgetSeason::onRevertChanges()
{
    // @todo: implement
}

void TvShowWidgetSeason::onDownloadFinished(DownloadManagerElement elem)
{
    if (elem.imageType == ImageType::TvShowSeasonPoster) {
        if (m_show == elem.show)
            ui->poster->setImage(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->imageFileName(elem.show, ImageType::TvShowSeasonPoster, elem.season));
        elem.show->setSeasonPosterImage(elem.season, elem.data);
    } else if (elem.imageType == ImageType::TvShowSeasonBackdrop) {
        Helper::resizeBackdrop(elem.data);
        if (m_show == elem.show)
            ui->backdrop->setImage(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->imageFileName(elem.show, ImageType::TvShowSeasonBackdrop, elem.season));
        elem.show->setSeasonBackdropImage(elem.season, elem.data);
    } else if (elem.imageType == ImageType::TvShowSeasonBanner) {
        if (m_show == elem.show)
            ui->banner->setImage(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->imageFileName(elem.show, ImageType::TvShowSeasonBanner, elem.season));
        elem.show->setSeasonBannerImage(elem.season, elem.data);
    } else if (elem.imageType == ImageType::TvShowSeasonThumb) {
        if (m_show == elem.show)
            ui->thumb->setImage(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->imageFileName(elem.show, ImageType::TvShowSeasonThumb, elem.season));
        elem.show->setSeasonThumbImage(elem.season, elem.data);
    }

    if (m_downloadManager->downloadsLeftForShow(m_show) == 0)
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
}

void TvShowWidgetSeason::onChooseImage()
{
    if (m_show == 0)
        return;

    ClosableImage *image = static_cast<ClosableImage*>(QObject::sender());
    if (!image)
        return;

    ImageDialog::instance()->setImageType(image->imageType());
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setTvShow(m_show);
    ImageDialog::instance()->setSeason(m_season);
    if (image->imageType() == ImageType::TvShowSeasonPoster)
        ImageDialog::instance()->setDownloads(m_show->seasonPosters(m_season));
    else if (image->imageType() == ImageType::TvShowSeasonBackdrop)
        ImageDialog::instance()->setDownloads(m_show->seasonBackdrops(m_season));
    else if (image->imageType() == ImageType::TvShowSeasonBanner)
        ImageDialog::instance()->setDownloads(m_show->seasonBanners(m_season));
    else
        ImageDialog::instance()->setDownloads(QList<Poster>());
    ImageDialog::instance()->exec(image->imageType());

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = image->imageType();
        d.url = ImageDialog::instance()->imageUrl();
        d.season = m_season;
        d.show = m_show;
        m_downloadManager->addDownload(d);
        image->setLoading(true);
    }
}

void TvShowWidgetSeason::onDeleteImage()
{
    if (m_show == 0)
        return;

    ClosableImage *image = static_cast<ClosableImage*>(QObject::sender());
    if (!image)
        return;

    m_show->removeImage(image->imageType(), m_season);
    updateImages(QList<int>() << image->imageType());
}
