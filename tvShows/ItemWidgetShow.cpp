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
    ui->iconPoster->setPixmap(ui->iconPoster->property(has ? "iconGreen" : "iconRed").value<QPixmap>());
}

void ItemWidgetShow::setHasFanart(const bool &has)
{
    ui->iconFanart->setPixmap(ui->iconFanart->property(has ? "iconGreen" : "iconRed").value<QPixmap>());
}

void ItemWidgetShow::setHasExtraFanart(const bool &has)
{
    ui->iconExtraFanarts->setPixmap(ui->iconExtraFanarts->property(has ? "iconGreen" : "iconRed").value<QPixmap>());
}

void ItemWidgetShow::setHasLogo(const bool &has)
{
    ui->iconLogo->setPixmap(ui->iconLogo->property(has ? "iconGreen" : "iconRed").value<QPixmap>());
}

void ItemWidgetShow::setHasClearArt(const bool &has)
{
    ui->iconClearArt->setPixmap(ui->iconClearArt->property(has ? "iconGreen" : "iconRed").value<QPixmap>());
}

void ItemWidgetShow::setHasCharacterArt(const bool &has)
{
    ui->iconCharacterArt->setPixmap(ui->iconCharacterArt->property(has ? "iconGreen" : "iconRed").value<QPixmap>());
}

void ItemWidgetShow::setHasBanner(const bool &has)
{
    ui->iconBanner->setPixmap(ui->iconBanner->property(has ? "iconGreen" : "iconRed").value<QPixmap>());
}

void ItemWidgetShow::setHasThumb(const bool &has)
{
    ui->iconThumb->setPixmap(ui->iconThumb->property(has ? "iconGreen" : "iconRed").value<QPixmap>());
}

void ItemWidgetShow::setMissingEpisodes(const bool &missing)
{
    ui->iconMissing->setVisible(missing);
}

void ItemWidgetShow::setLogoPath(const QString &path)
{
    Q_UNUSED(path);
}
