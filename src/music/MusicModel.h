#pragma once

#include "music/Artist.h"

#include <QAbstractItemModel>
#include <QIcon>
#include <QObject>

class MusicModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit MusicModel(QObject* parent = nullptr);
    ~MusicModel();

    QVariant data(const QModelIndex& index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    MusicModelItem* appendChild(Artist* artist);
    void clear();
    MusicModelItem* getItem(const QModelIndex& index) const;
    QVector<Artist*> artists();
    void removeArtist(Artist* artist);
    int hasNewArtistsOrAlbums();

private slots:
    void onSigChanged(MusicModelItem* artistItem, MusicModelItem* albumItem);
    void onArtistChanged(Artist* artist);

private:
    MusicModelItem* m_rootItem;
    QIcon m_newIcon;
};
