#ifndef MUSICMODEL_H
#define MUSICMODEL_H

#include <QAbstractItemModel>
#include <QObject>
#include "Artist.h"

class MusicModel : public QAbstractItemModel
{
public:
    MusicModel(QObject *parent = 0);
    ~MusicModel();

    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());
    MusicModelItem *appendChild(Artist *artist);
    void clear();
    MusicModelItem *getItem(const QModelIndex &index) const;
    QList<Artist*> artists();
    void removeArtist(Artist *artist);

private slots:
    void onSigChanged(MusicModelItem *artistItem, MusicModelItem *albumItem);
    void onArtistChanged(Artist *artist);

private:
    MusicModelItem *m_rootItem;
};

#endif // MUSICMODEL_H
