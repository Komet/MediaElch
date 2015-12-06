#include "ImageWidget.h"
#include "ui_ImageWidget.h"

#include <QQmlContext>
#include <QQuickView>
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "main/MainWindow.h"
#include "qml/AlbumImageProvider.h"

ImageWidget::ImageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageWidget)
{
    ui->setupUi(this);
    ui->quickWidget->rootContext()->setContextProperty("imageWidget", this);
    ui->quickWidget->rootContext()->setContextProperty("album", 0);
    ui->quickWidget->rootContext()->setContextProperty("loading", false);
#ifdef Q_OS_MAC
    ui->quickWidget->rootContext()->setContextProperty("isOsx", true);
#else
    ui->quickWidget->rootContext()->setContextProperty("isOsx", false);
#endif
    ui->quickWidget->engine()->addImageProvider(QLatin1String("album"), new AlbumImageProvider);
    ui->quickWidget->setSource(QUrl("qrc:/ui/ImageView.qml"));
}

ImageWidget::~ImageWidget()
{
    delete ui;
}

void ImageWidget::setAlbum(Album *album)
{
    ui->quickWidget->rootContext()->setContextProperty("album", album);
}

void ImageWidget::zoomImage(int artistIndex, int albumIndex, int imageId)
{
    if (Manager::instance()->musicModel()->artists().count() <= artistIndex)
        return;

    Artist *artist = Manager::instance()->musicModel()->artists().at(artistIndex);

    if (artist->albums().count() <= albumIndex)
        return;

    Album *album = artist->albums().at(albumIndex);

    int row = album->bookletModel()->rowById(imageId);
    QImage img = QImage::fromData(album->bookletModel()->data(album->bookletModel()->index(row, 0), Qt::UserRole+4).toByteArray());

    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(img));
    ImagePreviewDialog::instance()->exec();
}

void ImageWidget::imagesDropped(QVariantList urls)
{
    QList<QUrl> u;
    foreach (QVariant v, urls)
        u << QUrl(v.toString());
    emit sigImageDropped(u);
}

void ImageWidget::setLoading(bool loading)
{
    ui->quickWidget->rootContext()->setContextProperty("loading", loading);
}

void ImageWidget::cutImage(int artistIndex, int albumIndex, int imageId)
{
    if (Manager::instance()->musicModel()->artists().count() <= artistIndex)
        return;

    Artist *artist = Manager::instance()->musicModel()->artists().at(artistIndex);

    if (artist->albums().count() <= albumIndex)
        return;

    Album *album = artist->albums().at(albumIndex);

    int row = album->bookletModel()->rowById(imageId);
    album->bookletModel()->cutImage(row);
}
