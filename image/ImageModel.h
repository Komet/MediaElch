#ifndef IMAGEMODEL_H
#define IMAGEMODEL_H

#include <QAbstractListModel>
#include <QObject>

#include "Image.h"

class ImageModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ImageModel(QObject *parent = 0);
    ~ImageModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    Q_INVOKABLE QVariant data(int row, const QString &roleName) const;
    int role(const QString &roleName) const;
    void addImage(Image *image);
    void removeImage(Image *image);
    Q_INVOKABLE void move(int from, int to);
    QList<Image*> images();
    Image *image(int row) const;
    Image *image(const QModelIndex &index) const;
    int rowById(int id) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool setData(int row, const QVariant &value, int role);
    Q_INVOKABLE bool setData(int row, const QVariant &value, const QString &roleName);
    void clear();

    bool hasChanged() const;
    void setHasChanged(bool hasChanged);

    void cutImage(int row);

signals:
    void rowCountChanged();
    void hasChangedChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<Image*> m_images;
    bool m_hasChanged;
};

#endif // IMAGEMODEL_H
