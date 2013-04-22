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

    ui->poster->setDefaultPixmap(QPixmap(":/img/film_reel.png"));
    ui->backdrop->setDefaultPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->banner->setDefaultPixmap(QPixmap());

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

    connect(ui->poster, SIGNAL(clicked()), this, SLOT(onChoosePoster()));
    connect(ui->backdrop, SIGNAL(clicked()), this, SLOT(onChooseBackdrop()));
    connect(ui->banner, SIGNAL(clicked()), this, SLOT(onChooseBanner()));
    connect(ui->poster, SIGNAL(sigClose()), this, SLOT(onDeletePoster()));
    connect(ui->backdrop, SIGNAL(sigClose()), this, SLOT(onDeleteBackdrop()));
    connect(ui->banner, SIGNAL(sigClose()), this, SLOT(onDeleteBanner()));
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

    updateImages(QList<ImageType>() << TypeSeasonPoster << TypeSeasonBackdrop << TypeSeasonBanner);

    onSetEnabled(!show->downloadsInProgress());
    emit sigSetActionSaveEnabled(!show->downloadsInProgress(), WidgetTvShows);
}

void TvShowWidgetSeason::updateImages(QList<ImageType> images)
{
    if (images.contains(TypeSeasonPoster)) {
        if (!m_show->seasonPosterImage(m_season).isNull())
            ui->poster->setImage(m_show->seasonPosterImage(m_season));
        else if (!Manager::instance()->mediaCenterInterfaceTvShow()->seasonPosterImageName(m_show, m_season).isEmpty() &&
                (!m_show->imagesToRemove().contains(TypeSeasonPoster) || !m_show->imagesToRemove().value(TypeSeasonPoster).contains(m_season)))
            ui->poster->setImage(Manager::instance()->mediaCenterInterfaceTvShow()->seasonPosterImageName(m_show, m_season));
    }

    if (images.contains(TypeSeasonBackdrop)) {
        if (!m_show->seasonBackdropImage(m_season).isNull())
            ui->backdrop->setImage(m_show->seasonBackdropImage(m_season));
        else if (!Manager::instance()->mediaCenterInterfaceTvShow()->seasonPosterImageName(m_show, m_season).isEmpty() &&
                (!m_show->imagesToRemove().contains(TypeSeasonBackdrop) || !m_show->imagesToRemove().value(TypeSeasonBackdrop).contains(m_season)))
            ui->backdrop->setImage(Manager::instance()->mediaCenterInterfaceTvShow()->seasonBackdropImageName(m_show, m_season));
    }

    if (images.contains(TypeSeasonBanner)) {
        if (!m_show->seasonBannerImage(m_season).isNull())
            ui->banner->setImage(m_show->seasonBannerImage(m_season));
        else if (!Manager::instance()->mediaCenterInterfaceTvShow()->seasonBannerImageName(m_show, m_season).isEmpty() &&
                (!m_show->imagesToRemove().contains(TypeSeasonBanner) || !m_show->imagesToRemove().value(TypeSeasonBanner).contains(m_season)))
            ui->banner->setImage(Manager::instance()->mediaCenterInterfaceTvShow()->seasonBannerImageName(m_show, m_season));
    }
}

void TvShowWidgetSeason::onClear()
{
    ui->title->clear();
    ui->poster->clear();
    ui->backdrop->clear();
    ui->banner->clear();
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

void TvShowWidgetSeason::onChoosePoster()
{
    if (!m_show)
        return;

    ImageDialog::instance()->setImageType(TypePoster);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setTvShow(m_show);
    ImageDialog::instance()->setSeason(m_season);
    ImageDialog::instance()->setDownloads(m_show->seasonPosters(m_season));
    ImageDialog::instance()->exec(ImageDialogType::TvShowSeason);
    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeSeasonPoster;
        d.url = ImageDialog::instance()->imageUrl();
        d.season = m_season;
        d.show = m_show;
        m_downloadManager->addDownload(d);
        ui->poster->setLoading(true);
    }
}

void TvShowWidgetSeason::onChooseBackdrop()
{
    if (!m_show)
        return;

    ImageDialog::instance()->setImageType(TypeBackdrop);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setTvShow(m_show);
    ImageDialog::instance()->setSeason(m_season);
    ImageDialog::instance()->setDownloads(m_show->seasonBackdrops(m_season));
    ImageDialog::instance()->exec(ImageDialogType::TvShowSeasonBackdrop);
    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeSeasonBackdrop;
        d.url = ImageDialog::instance()->imageUrl();
        d.season = m_season;
        d.show = m_show;
        m_downloadManager->addDownload(d);
        ui->backdrop->setLoading(true);
    }
}

void TvShowWidgetSeason::onChooseBanner()
{
    if (!m_show)
        return;

    ImageDialog::instance()->setImageType(TypeBanner);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setTvShow(m_show);
    ImageDialog::instance()->setSeason(m_season);
    ImageDialog::instance()->setDownloads(m_show->seasonBanners(m_season));
    ImageDialog::instance()->exec(ImageDialogType::TvShowSeasonBanner);
    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeSeasonBanner;
        d.url = ImageDialog::instance()->imageUrl();
        d.season = m_season;
        d.show = m_show;
        m_downloadManager->addDownload(d);
        ui->banner->setLoading(true);
    }
}

void TvShowWidgetSeason::onDeletePoster()
{
    m_show->removeImage(TypeSeasonPoster, m_season);
    updateImages(QList<ImageType>() << TypeSeasonPoster);
}

void TvShowWidgetSeason::onDeleteBackdrop()
{
    m_show->removeImage(TypeSeasonBackdrop, m_season);
    updateImages(QList<ImageType>() << TypeSeasonBackdrop);
}

void TvShowWidgetSeason::onDeleteBanner()
{
    m_show->removeImage(TypeSeasonBanner, m_season);
    updateImages(QList<ImageType>() << TypeSeasonBanner);
}

void TvShowWidgetSeason::onRevertChanges()
{
    // @todo: implement
}

void TvShowWidgetSeason::onDownloadFinished(DownloadManagerElement elem)
{
    if (elem.imageType == TypeSeasonPoster) {
        if (m_show == elem.show)
            ui->poster->setImage(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->seasonPosterImageName(elem.show, elem.season));
        elem.show->setSeasonPosterImage(elem.season, elem.data);
    } else if (elem.imageType == TypeSeasonBackdrop) {
        Helper::resizeBackdrop(elem.data);
        if (m_show == elem.show)
            ui->backdrop->setImage(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->seasonBackdropImageName(elem.show, elem.season));
        elem.show->setSeasonBackdropImage(elem.season, elem.data);
    } else if (elem.imageType == TypeSeasonBanner) {
        if (m_show == elem.show)
            ui->banner->setImage(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->seasonBannerImageName(elem.show, elem.season));
        elem.show->setSeasonBannerImage(elem.season, elem.data);
    }

    if (m_downloadManager->downloadsLeftForShow(m_show) == 0)
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
}
