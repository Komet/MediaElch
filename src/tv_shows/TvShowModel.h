#pragma once

#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"
#include "tv_shows/model/TvShowRootModelItem.h"

#include <QAbstractItemModel>
#include <QIcon>
#include <QModelIndex>
#include <QVariant>

class TvShowModelItem;
class SeasonModelItem;
class EpisodeModelItem;

/// @brief The TvShowModel is responsible for handling *all* TV shows and episodes. A single
/// show or season is represented by TvShowModelItem
class TvShowModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TvShowModel(QObject* parent = nullptr);
    ~TvShowModel() override = default;

    QVariant data(const QModelIndex& index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex()) override;

    TvShowModelItem* appendChild(TvShow* show);
    void removeShow(TvShow* show);
    const TvShowBaseModelItem& getItem(const QModelIndex& index) const;
    TvShowBaseModelItem& getItem(const QModelIndex& index);
    void clear();

    QVector<TvShow*> tvShows();
    int hasNewShowOrEpisode();

private slots:
    void onSigChanged(TvShowModelItem* showItem, SeasonModelItem* seasonItem, EpisodeModelItem* episodeItem);
    void onShowChanged(TvShow* show);

private:
    TvShowRootModelItem m_rootItem;

    QMap<int, QMap<bool, QIcon>> m_icons;
    QIcon m_newIcon;
    QIcon m_syncIcon;
    QIcon m_missingIcon;
};
