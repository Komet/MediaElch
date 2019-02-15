#pragma once

#include "globals/Globals.h"

#include <QObject>

class Artist;
class Album;

class MusicModelItem : public QObject
{
    Q_OBJECT
public:
    explicit MusicModelItem(MusicModelItem* parent = nullptr);
    ~MusicModelItem() override;

    MusicModelItem* child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    MusicModelItem* appendChild(Artist* artist);
    MusicModelItem* appendChild(Album* album);
    MusicModelItem* parent();
    bool removeChildren(int position, int count);
    Q_INVOKABLE int childNumber() const;
    void setArtist(Artist* artist);
    void setAlbum(Album* album);
    Artist* artist();
    Album* album();
    MusicType type() const;

signals:
    void sigIntChanged(MusicModelItem*, MusicModelItem*);
    void sigChanged(MusicModelItem*, MusicModelItem*);

private slots:
    void onAlbumChanged(Album* album);

private:
    QVector<MusicModelItem*> m_childItems;
    MusicModelItem* m_parentItem;
    Artist* m_artist;
    Album* m_album;
};
