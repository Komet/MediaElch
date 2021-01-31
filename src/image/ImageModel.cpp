#include "ImageModel.h"

#include <QBuffer>
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QtMath>

#include "settings/Settings.h"

ImageModel::ImageModel(QObject* parent) : QAbstractListModel(parent), m_hasChanged{false}
{
}

ImageModel::~ImageModel()
{
    qDeleteAll(m_images);
}

void ImageModel::clear()
{
    if (m_images.isEmpty()) {
        return;
    }
    beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
    QList<Image*> imgs = m_images;
    m_images.clear();
    endRemoveRows();
    qDeleteAll(imgs);
}

int ImageModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_images.count();
}

QVariant ImageModel::data(int row, const QString& roleName) const
{
    return data(createIndex(row, 0), role(roleName));
}

QVariant ImageModel::data(const QModelIndex& index, int role) const
{
    Image* img = image(index);
    if (img == nullptr) {
        return QVariant();
    }

    switch (role) {
    case ImageRoles::FilenameRole: return img->fileName();
    case ImageRoles::RawDataRole: return img->rawData();
    case ImageRoles::DeletionRole: return img->deletion();
    case ImageRoles::ImageDataRole: {
        img->load();
        return img->rawData();
    }
    case ImageRoles::BookletNumberRole: return m_images.indexOf(img);
    case ImageRoles::IdRole: return img->imageId();
    default: return {};
    }
}

int ImageModel::role(const QString& roleName) const
{
    return roleNames().key(roleName.toUtf8());
}

void ImageModel::addImage(Image* image)
{
    image->setParent(this);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_images.append(image);
    endInsertRows();
    emit rowCountChanged();
    setHasChanged(true);
}

void ImageModel::removeImage(Image* image)
{
    int row = m_images.indexOf(image);
    if (row == -1) {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_images.removeAt(row);
    endRemoveRows();
    emit rowCountChanged();
    setHasChanged(true);
}

void ImageModel::ImageModel::move(int from, int to)
{
    if (from == to) {
        return;
    }
    if (from < to) {
        qSwap(from, to);
    }
    beginMoveRows(QModelIndex(), from, from, QModelIndex(), to);
    m_images.move(from, to);
    endMoveRows();
    setHasChanged(true);
}

QList<Image*> ImageModel::images()
{
    return m_images;
}

Image* ImageModel::image(int row) const
{
    if (row < 0 || row >= m_images.count()) {
        return nullptr;
    }
    return m_images.at(row);
}

Image* ImageModel::image(const QModelIndex& index) const
{
    return image(index.row());
}

QHash<int, QByteArray> ImageModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ImageRoles::FilenameRole] = "fileName";
    roles[ImageRoles::RawDataRole] = "rawData";
    roles[ImageRoles::DeletionRole] = "deletion";
    roles[ImageRoles::ImageDataRole] = "imageData";
    roles[ImageRoles::BookletNumberRole] = "bookletNum";
    roles[ImageRoles::IdRole] = "imageId";
    return roles;
}

bool ImageModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    return setData(index.row(), value, role);
}

bool ImageModel::setData(int row, const QVariant& value, const QString& roleName)
{
    return setData(row, value, role(roleName));
}

bool ImageModel::setData(int row, const QVariant& value, int role)
{
    if (row < 0 || row >= m_images.count()) {
        return false;
    }

    Image* img = image(row);

    switch (role) {
    case FilenameRole: {
        if (value.toString() == img->fileName()) {
            return false;
        }
        img->setFileName(value.toString());
        break;
    }
    case RawDataRole: {
        if (value.toByteArray() == img->rawData()) {
            return false;
        }
        img->setRawData(value.toByteArray());
        break;
    }
    case DeletionRole: {
        if (value.toBool() == img->deletion()) {
            return false;
        }
        img->setDeletion(value.toBool());
        break;
    }
    default: return false;
    }

    setHasChanged(true);
    emit dataChanged(createIndex(row, 0), createIndex(row, 0));
    return true;
}

int ImageModel::rowById(int id) const
{
    for (int i = 0, n = m_images.count(); i < n; ++i) {
        if (m_images.at(i)->imageId() == id) {
            return i;
        }
    }
    return -1;
}

bool ImageModel::hasChanged() const
{
    return m_hasChanged;
}

void ImageModel::setHasChanged(bool hasChanged)
{
    if (hasChanged == m_hasChanged) {
        return;
    }
    m_hasChanged = hasChanged;
    emit hasChangedChanged();
}

void ImageModel::cutImage(int row)
{
    if (row < 0 || row >= m_images.count()) {
        return;
    }

    Image* image1 = m_images.at(row);

    auto cut = static_cast<qreal>(Settings::instance()->advanced()->bookletCut());
    QImage img = QImage::fromData(data(row, "rawData").toByteArray());

    int width1 = qFloor(static_cast<qreal>(img.width()) / 2.0 * (1.0 - (cut / 100.0)));
    int width2 = qCeil(static_cast<qreal>(img.width()) / 2.0 * (1.0 - (cut / 100.0)));

    QImage img1 = img.copy(0, 0, width1, img.height());
    QImage img2 = img.copy(img.width() - width2, 0, width2, img.height());

    QByteArray ba1;
    QBuffer buffer1(&ba1);
    buffer1.open(QIODevice::WriteOnly);
    img1.save(&buffer1, "JPG", 90);
    buffer1.close();

    QByteArray ba2;
    QBuffer buffer2(&ba2);
    buffer2.open(QIODevice::WriteOnly);
    img2.save(&buffer2, "JPG", 90);
    buffer2.close();

    auto* image2 = new Image;
    image2->setRawData(ba2);

    beginInsertRows(QModelIndex(), row + 1, row + 1);
    m_images.insert(row + 1, image2);
    endInsertRows();

    image1->setRawData(ba1);
    image1->resetIdCounter();

    emit dataChanged(createIndex(row, 0), createIndex(row, 1));
    setHasChanged(true);
}
