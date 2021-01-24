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

/// \brief The TvShowModel is responsible for handling *all* TV shows and episodes. A single
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
    bool removeRows(int row, int count, const QModelIndex& parent = {}) override;

    /// Append a TV show and its seasons and episodes to the tree view.
    void appendShow(TvShow* show);
    /// Remove a show from the TreeView
    /// \return true if the show was found and removed, false otherwise
    bool removeShow(TvShow* show);
    /// Update a show, i.e. remove it and add it again.
    /// \todo Maybe add more specific functions like adding a season, etc.
    /// \return true if the show was found and updated, false otherwise
    bool updateShow(TvShow* show);

    const TvShowBaseModelItem& getItem(const QModelIndex& index) const;
    TvShowBaseModelItem& getItem(const QModelIndex& index);
    void clear();

    QVector<TvShow*> tvShows();
    int hasNewShowOrEpisode();

private slots:
    void onSigChanged(TvShowModelItem* showItem, SeasonModelItem* seasonItem, EpisodeModelItem* episodeItem);
    void onShowChanged(TvShow* show);

private:
    TvShowModelItem* findModelForShow(TvShow* show);

private:
    TvShowRootModelItem m_rootItem;

    QMap<int, QMap<bool, QIcon>> m_icons;
    QIcon m_newIcon;
    QIcon m_syncIcon;
    QIcon m_missingIcon;
};
