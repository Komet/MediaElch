#include "ItemWidgetShow.h"
#include "ui_ItemWidgetShow.h"

#include "globals/Helper.h"

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
    iSync = iSync.scaled(ui->iconSync->size() * Helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    Helper::setDevicePixelRatio(iSync, Helper::devicePixelRatio(this));
    ui->iconSync->setPixmap(iSync);

    QPixmap iNew(":/img/star_blue.png");
    iNew = iNew.scaled(ui->iconNew->size() * Helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    Helper::setDevicePixelRatio(iNew, Helper::devicePixelRatio(this));
    ui->iconNew->setPixmap(iNew);

    QPixmap iMissing(":/img/missing.png");
    iMissing = iMissing.scaled(ui->iconMissing->size() * Helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    Helper::setDevicePixelRatio(iMissing, Helper::devicePixelRatio(this));
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
    QPixmap p = ui->iconPoster->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconPoster->size() * Helper::devicePixelRatio(this),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    Helper::setDevicePixelRatio(p, Helper::devicePixelRatio(this));
    ui->iconPoster->setPixmap(p);
}

void ItemWidgetShow::setHasFanart(const bool &has)
{
    QPixmap p = ui->iconFanart->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconFanart->size() * Helper::devicePixelRatio(this),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    Helper::setDevicePixelRatio(p, Helper::devicePixelRatio(this));
    ui->iconFanart->setPixmap(p);
}

void ItemWidgetShow::setHasExtraFanart(const bool &has)
{
    QPixmap p = ui->iconExtraFanarts->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconExtraFanarts->size() * Helper::devicePixelRatio(this),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    Helper::setDevicePixelRatio(p, Helper::devicePixelRatio(this));
    ui->iconExtraFanarts->setPixmap(p);
}

void ItemWidgetShow::setHasLogo(const bool &has)
{
    QPixmap p = ui->iconLogo->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconLogo->size() * Helper::devicePixelRatio(this),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    Helper::setDevicePixelRatio(p, Helper::devicePixelRatio(this));
    ui->iconLogo->setPixmap(p);
}

void ItemWidgetShow::setHasClearArt(const bool &has)
{
    QPixmap p = ui->iconClearArt->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconClearArt->size() * Helper::devicePixelRatio(this),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    Helper::setDevicePixelRatio(p, Helper::devicePixelRatio(this));
    ui->iconClearArt->setPixmap(p);
}

void ItemWidgetShow::setHasCharacterArt(const bool &has)
{
    QPixmap p = ui->iconCharacterArt->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconCharacterArt->size() * Helper::devicePixelRatio(this),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    Helper::setDevicePixelRatio(p, Helper::devicePixelRatio(this));
    ui->iconCharacterArt->setPixmap(p);
}

void ItemWidgetShow::setHasBanner(const bool &has)
{
    QPixmap p = ui->iconBanner->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconBanner->size() * Helper::devicePixelRatio(this),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    Helper::setDevicePixelRatio(p, Helper::devicePixelRatio(this));
    ui->iconBanner->setPixmap(p);
}

void ItemWidgetShow::setHasThumb(const bool &has)
{
    QPixmap p = ui->iconThumb->property(has ? "iconGreen" : "iconRed").value<QPixmap>().scaled(ui->iconThumb->size() * Helper::devicePixelRatio(this),
                                                                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
    Helper::setDevicePixelRatio(p, Helper::devicePixelRatio(this));
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
