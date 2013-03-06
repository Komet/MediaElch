#include "TvShowWidgetSeason.h"
#include "ui_TvShowWidgetSeason.h"

#include <QPainter>
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
    ui->posterResolution->clear();
    ui->backdropResolution->clear();
    ui->bannerResolution->clear();
    QFont font = ui->title->font();
    font.setPointSize(font.pointSize()+4);
    ui->title->setFont(font);
    ui->buttonPreviewBackdrop->setEnabled(false);
    ui->buttonPreviewPoster->setEnabled(false);
    ui->buttonPreviewBanner->setEnabled(false);

    font = ui->posterResolution->font();
    #ifdef Q_OS_WIN32
    font.setPointSize(font.pointSize()-1);
    #else
    font.setPointSize(font.pointSize()-2);
    #endif
    ui->posterResolution->setFont(font);
    ui->backdropResolution->setFont(font);
    ui->bannerResolution->setFont(font);

    m_downloadManager = new DownloadManager(this);

    m_loadingMovie = new QMovie(":/img/spinner.gif");
    m_loadingMovie->start();

    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    onSetEnabled(false);
    onClear();

    QPixmap zoomIn(":/img/zoom_in.png");
    QPainter p;
    p.begin(&zoomIn);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(zoomIn.rect(), QColor(0, 0, 0, 150));
    p.end();
    ui->buttonPreviewBackdrop->setIcon(QIcon(zoomIn));
    ui->buttonPreviewPoster->setIcon(QIcon(zoomIn));
    ui->buttonPreviewBanner->setIcon(QIcon(zoomIn));

    QPixmap revert(":/img/reload.png");
    p.begin(&revert);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(revert.rect(), QColor(0, 0, 0, 200));
    p.end();
    ui->buttonRevert->setIcon(QIcon(revert));
    ui->buttonRevert->setVisible(false);

    connect(ui->poster, SIGNAL(clicked()), this, SLOT(onChoosePoster()));
    connect(ui->backdrop, SIGNAL(clicked()), this, SLOT(onChooseBackdrop()));
    connect(ui->banner, SIGNAL(clicked()), this, SLOT(onChooseBanner()));
    connect(ui->buttonPreviewPoster, SIGNAL(clicked()), this, SLOT(onPreviewPoster()));
    connect(ui->buttonPreviewBackdrop, SIGNAL(clicked()), this, SLOT(onPreviewBackdrop()));
    connect(ui->buttonPreviewBanner, SIGNAL(clicked()), this, SLOT(onPreviewBanner()));
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

    if (!m_show->seasonPosterImage(season).isNull()) {
        QImage img = QImage::fromData(m_show->seasonPosterImage(season));
        ui->poster->setPixmap(QPixmap::fromImage(img).scaledToWidth(200, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = img;
    } else if (!Manager::instance()->mediaCenterInterfaceTvShow()->seasonPosterImageName(m_show, season).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterfaceTvShow()->seasonPosterImageName(m_show, season));
        ui->poster->setPixmap(p.scaledToWidth(200, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = p.toImage();
    } else {
        ui->poster->setPixmap(QPixmap(":/img/poster.png"));
        ui->posterResolution->clear();
        ui->buttonPreviewPoster->setEnabled(false);
    }

    if (!m_show->seasonBackdropImage(season).isNull()) {
        QImage img = QImage::fromData(m_show->seasonBackdropImage(season));
        ui->backdrop->setPixmap(QPixmap::fromImage(img).scaledToWidth(200, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = img;
    } else if (!Manager::instance()->mediaCenterInterfaceTvShow()->seasonBackdropImageName(m_show, season).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterfaceTvShow()->seasonBackdropImageName(m_show, season));
        ui->backdrop->setPixmap(p.scaledToWidth(200, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = p.toImage();
    } else {
        ui->backdrop->setPixmap(QPixmap(":/img/missing_art.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->backdropResolution->clear();
        ui->buttonPreviewBackdrop->setEnabled(false);
    }

    if (!m_show->seasonBannerImage(season).isNull()) {
        QImage img = QImage::fromData(m_show->seasonBannerImage(season));
        ui->banner->setPixmap(QPixmap::fromImage(img).scaledToWidth(200, Qt::SmoothTransformation));
        ui->bannerResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
        ui->buttonPreviewBanner->setEnabled(true);
        m_currentBanner = img;
    } else if (!Manager::instance()->mediaCenterInterfaceTvShow()->seasonBannerImageName(m_show, season).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterfaceTvShow()->seasonBannerImageName(m_show, season));
        ui->banner->setPixmap(p.scaledToWidth(200, Qt::SmoothTransformation));
        ui->bannerResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewBanner->setEnabled(true);
        m_currentBanner = p.toImage();
    } else {
        ui->banner->setPixmap(QPixmap(":/img/missing_art_small.png"));
        ui->bannerResolution->clear();
        ui->buttonPreviewBanner->setEnabled(false);
    }

    onSetEnabled(!show->downloadsInProgress());
    emit sigSetActionSaveEnabled(!show->downloadsInProgress(), WidgetTvShows);
}

void TvShowWidgetSeason::onClear()
{
    ui->title->clear();
    ui->poster->setPixmap(QPixmap(":/img/poster.png"));
    ui->backdrop->setPixmap(QPixmap(":/img/missing_art.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->banner->setPixmap(QPixmap());
    ui->posterResolution->clear();
    ui->backdropResolution->clear();
    ui->bannerResolution->clear();
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
        ui->poster->setPixmap(QPixmap());
        ui->poster->setMovie(m_loadingMovie);
        ui->posterResolution->clear();
        ui->buttonPreviewPoster->setEnabled(false);
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
        ui->backdrop->setPixmap(QPixmap());
        ui->backdrop->setMovie(m_loadingMovie);
        ui->backdropResolution->clear();
        ui->buttonPreviewBackdrop->setEnabled(false);
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
        ui->banner->setPixmap(QPixmap());
        ui->banner->setMovie(m_loadingMovie);
        ui->bannerResolution->clear();
        ui->buttonPreviewBanner->setEnabled(false);
    }
}

void TvShowWidgetSeason::onPreviewPoster()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentPoster));
    ImagePreviewDialog::instance()->exec();
}

void TvShowWidgetSeason::onPreviewBackdrop()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentBackdrop));
    ImagePreviewDialog::instance()->exec();
}

void TvShowWidgetSeason::onPreviewBanner()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentBanner));
    ImagePreviewDialog::instance()->exec();
}

void TvShowWidgetSeason::onRevertChanges()
{
    // @todo: implement
}

void TvShowWidgetSeason::onDownloadFinished(DownloadManagerElement elem)
{
    if (elem.imageType == TypeSeasonPoster) {
        if (m_show == elem.show) {
            QImage img = QImage::fromData(elem.data);
            ui->poster->setPixmap(QPixmap::fromImage(img).scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->posterResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
            ui->buttonPreviewPoster->setEnabled(true);
            m_currentPoster = img;
        }
        elem.show->setSeasonPosterImage(elem.season, elem.data);
    } else if (elem.imageType == TypeSeasonBackdrop) {
        Helper::resizeBackdrop(elem.data);
        if (m_show == elem.show) {
            QImage img = QImage::fromData(elem.data);
            ui->backdrop->setPixmap(QPixmap::fromImage(img).scaled(200, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->backdropResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
            ui->buttonPreviewBackdrop->setEnabled(true);
            m_currentBackdrop = img;
        }
        elem.show->setSeasonBackdropImage(elem.season, elem.data);
    } else if (elem.imageType == TypeSeasonBanner) {
        if (m_show == elem.show) {
            QImage img = QImage::fromData(elem.data);
            ui->banner->setPixmap(QPixmap::fromImage(img).scaled(200, 37, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->bannerResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
            ui->buttonPreviewBanner->setEnabled(true);
            m_currentBanner = img;
        }
        elem.show->setSeasonBannerImage(elem.season, elem.data);
    }

    if (m_downloadManager->downloadsLeftForShow(m_show) == 0)
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
}
