#ifndef TVSHOWMODEL_H
#define TVSHOWMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"

class TvShowModelItem;

/**
 * @brief The TvShowModel class
 */
class TvShowModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TvShowModel(QObject *parent = 0);
    ~TvShowModel();

    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());
    TvShowModelItem *appendChild(TvShow *show);
    void clear();
    TvShowModelItem *getItem(const QModelIndex &index) const;
    QList<TvShow*> tvShows();
    bool hasNewShowOrEpisode();

private slots:
    void onSigChanged(TvShowModelItem *showItem, TvShowModelItem *seasonItem, TvShowModelItem *episodeItem);
    void onShowChanged(TvShow *show);

private:
    TvShowModelItem *m_rootItem;
};

#endif // TVSHOWMODEL_H
