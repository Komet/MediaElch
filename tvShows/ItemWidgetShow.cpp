#include "ItemWidgetShow.h"
#include "ui_ItemWidgetShow.h"

ItemWidgetShow::ItemWidgetShow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ItemWidgetShow)
{
    ui->setupUi(this);

    QFont font = ui->episodes->font();
#ifdef Q_OS_MAC
    font.setPointSize(font.pointSize()-2);
#else
    font.setPointSize(font.pointSize()-1);
#endif
    ui->episodes->setFont(font);

    QPixmap iSync(":/img/reload_orange.png");
    iSync = iSync.scaled(ui->iconSync->size() * ui->iconSync->devicePixelRatio(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    iSync.setDevicePixelRatio(ui->iconSync->devicePixelRatio());
    ui->iconSync->setPixmap(iSync);

    QPixmap iNew(":/img/star_blue.png");
    iNew = iNew.scaled(ui->iconNew->size() * ui->iconNew->devicePixelRatio(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    iNew.setDevicePixelRatio(ui->iconNew->devicePixelRatio());
    ui->iconNew->setPixmap(iNew);

    QPixmap iMissing(":/img/missing.png");
    iMissing = iMissing.scaled(ui->iconMissing->size() * ui->iconMissing->devicePixelRatio(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    iMissing.setDevicePixelRatio(ui->iconMissing->devicePixelRatio());
    ui->iconMissing->setPixmap(iMissing);


}

ItemWidgetShow::~ItemWidgetShow()
{
    delete ui;
}

void ItemWidgetShow::setTitle(const QString &title)
{
    ui->title->setText(title);
}

void ItemWidgetShow::setEpisodeCount(const int &episodeCount)
{
    ui->episodes->setText(tr("%n Episodes", "", episodeCount));
}

void ItemWidgetShow::setSyncNeeded(const bool &syncNeeded)
{
    ui->iconSync->setVisible(syncNeeded);
}

void ItemWidgetShow::setNew(const bool &isNew)
{
    ui->iconNew->setVisible(isNew);
}

void ItemWidgetShow::setHasPoster(const bool &has)
{
    QPixmap p = ui->iconPoster->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconPoster->size() * ui->iconPoster->devicePixelRatio(),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p.setDevicePixelRatio(ui->iconPoster->devicePixelRatio());
    ui->iconPoster->setPixmap(p);
}

void ItemWidgetShow::setHasFanart(const bool &has)
{
    QPixmap p = ui->iconFanart->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconFanart->size() * ui->iconFanart->devicePixelRatio(),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p.setDevicePixelRatio(ui->iconFanart->devicePixelRatio());
    ui->iconFanart->setPixmap(p);
}

void ItemWidgetShow::setHasExtraFanart(const bool &has)
{
    QPixmap p = ui->iconExtraFanarts->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconExtraFanarts->size() * ui->iconExtraFanarts->devicePixelRatio(),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p.setDevicePixelRatio(ui->iconExtraFanarts->devicePixelRatio());
    ui->iconExtraFanarts->setPixmap(p);
}

void ItemWidgetShow::setHasLogo(const bool &has)
{
    QPixmap p = ui->iconLogo->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconLogo->size() * ui->iconLogo->devicePixelRatio(),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p.setDevicePixelRatio(ui->iconLogo->devicePixelRatio());
    ui->iconLogo->setPixmap(p);
}

void ItemWidgetShow::setHasClearArt(const bool &has)
{
    QPixmap p = ui->iconClearArt->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconClearArt->size() * ui->iconClearArt->devicePixelRatio(),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p.setDevicePixelRatio(ui->iconClearArt->devicePixelRatio());
    ui->iconClearArt->setPixmap(p);
}

void ItemWidgetShow::setHasCharacterArt(const bool &has)
{
    QPixmap p = ui->iconCharacterArt->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconCharacterArt->size() * ui->iconCharacterArt->devicePixelRatio(),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p.setDevicePixelRatio(ui->iconCharacterArt->devicePixelRatio());
    ui->iconCharacterArt->setPixmap(p);
}

void ItemWidgetShow::setHasBanner(const bool &has)
{
    QPixmap p = ui->iconBanner->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconBanner->size() * ui->iconBanner->devicePixelRatio(),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p.setDevicePixelRatio(ui->iconBanner->devicePixelRatio());
    ui->iconBanner->setPixmap(p);
}

void ItemWidgetShow::setHasThumb(const bool &has)
{
    QPixmap p = ui->iconThumb->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconThumb->size() * ui->iconThumb->devicePixelRatio(),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p.setDevicePixelRatio(ui->iconThumb->devicePixelRatio());
    ui->iconThumb->setPixmap(p);
}

void ItemWidgetShow::setMissingEpisodes(const bool &missing)
{
    ui->iconMissing->setVisible(missing);
}

void ItemWidgetShow::setLogoPath(const QString &path)
{
    Q_UNUSED(path);
}
