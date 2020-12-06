#include "MusicModelItem.h"

#include "music/Album.h"
#include "music/Artist.h"

MusicModelItem::MusicModelItem(MusicModelItem* parent) :
    QObject(nullptr), m_parentItem{parent}, m_artist{nullptr}, m_album{nullptr}
{
}

MusicModelItem::~MusicModelItem()
{
    qDeleteAll(m_childItems);
}

MusicModelItem* MusicModelItem::child(int number)
{
    return m_childItems.value(number);
}

int MusicModelItem::childCount() const
{
    return m_childItems.count();
}

int MusicModelItem::childNumber() const
{
    if (m_parentItem != nullptr) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        return m_parentItem->m_childItems.indexOf(const_cast<MusicModelItem*>(this));
    }

    return 0;
}

int MusicModelItem::columnCount() const
{
    return 1;
}

QVariant MusicModelItem::data(int column) const
{
    switch (column) {
    case MusicRoles::Type: return static_cast<int>(type());
    case MusicRoles::HasChanged:
        if (m_album != nullptr) {
            return m_album->hasChanged();
        }
        if (m_artist != nullptr) {
            return m_artist->hasChanged();
        }
        break;
    case MusicRoles::NumOfAlbums:
        if (m_artist != nullptr) {
            return m_artist->albums().count();
        }
        break;
    case MusicRoles::IsNew: {
        if (m_album != nullptr) {
            return !m_album->controller()->infoLoaded();
        }

        if ((m_artist != nullptr) && !m_artist->controller()->infoLoaded()) {
            return true;
        }

        if (m_artist != nullptr) {
            for (Album* album : m_artist->albums()) {
                if (!album->controller()->infoLoaded()) {
                    return true;
                }
            }
        }
        return false;
    }
    default:
        if (m_artist != nullptr) {
            return m_artist->name();
        } else if (m_album != nullptr) {
            return m_album->title();
        }
    }

    return QVariant();
}

MusicModelItem* MusicModelItem::appendChild(Artist* artist)
{
    auto* item = new MusicModelItem(this);
    item->setArtist(artist);
    artist->setModelItem(item);
    m_childItems.append(item);
    return item;
}

MusicModelItem* MusicModelItem::appendChild(Album* album)
{
    auto* item = new MusicModelItem(this);
    item->setAlbum(album);
    album->setModelItem(item);
    m_childItems.append(item);
    connect(album, &Album::sigChanged, this, &MusicModelItem::onAlbumChanged, Qt::UniqueConnection);
    connect(
        album->controller(), &AlbumController::sigSaved, this, &MusicModelItem::onAlbumChanged, Qt::UniqueConnection);
    return item;
}

MusicModelItem* MusicModelItem::parent()
{
    return m_parentItem;
}

bool MusicModelItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems.size()) {
        return false;
    }

    for (int row = 0; row < count; ++row) {
        delete m_childItems.takeAt(position);
    }

    return true;
}

void MusicModelItem::setArtist(Artist* artist)
{
    m_artist = artist;
}

void MusicModelItem::setAlbum(Album* album)
{
    m_album = album;
}

Artist* MusicModelItem::artist()
{
    return m_artist;
}

Album* MusicModelItem::album()
{
    return m_album;
}

MusicType MusicModelItem::type() const
{
    if (m_artist != nullptr) {
        return MusicType::Artist;
    }
    if (m_album != nullptr) {
        return MusicType::Album;
    }

    return MusicType::None;
}

void MusicModelItem::onAlbumChanged(Album* album)
{
    emit sigIntChanged(this, album->modelItem());
}
