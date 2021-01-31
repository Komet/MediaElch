#pragma once

#include "Image.h"

#include <QAbstractListModel>
#include <QList>
#include <QObject>

class ImageModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ImageRoles
    {
        FilenameRole = Qt::UserRole + 1,
        RawDataRole = Qt::UserRole + 2,
        DeletionRole = Qt::UserRole + 3,
        ImageDataRole = Qt::UserRole + 4,
        BookletNumberRole = Qt::UserRole + 5,
        IdRole = Qt::UserRole + 6
    };

public:
    explicit ImageModel(QObject* parent = nullptr);
    ~ImageModel() override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    Q_INVOKABLE QVariant data(int row, const QString& roleName) const;
    int role(const QString& roleName) const;
    void addImage(Image* image);
    void removeImage(Image* image);
    Q_INVOKABLE void move(int from, int to);
    QList<Image*> images();
    Image* image(int row) const;
    Image* image(const QModelIndex& index) const;
    int rowById(int id) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    bool setData(int row, const QVariant& value, int role);
    Q_INVOKABLE bool setData(int row, const QVariant& value, const QString& roleName);
    void clear();

    bool hasChanged() const;
    void setHasChanged(bool hasChanged);

    void cutImage(int row);

signals:
    void rowCountChanged();
    void hasChangedChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<Image*> m_images;
    bool m_hasChanged;
};
