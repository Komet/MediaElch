#ifndef TVSHOWMODEL_H
#define TVSHOWMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
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
    explicit TvShowModel(QObject *parent = nullptr);
    ~TvShowModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
    TvShowModelItem *appendChild(TvShow *show);
    void clear();
    TvShowModelItem *getItem(const QModelIndex &index) const;
    QList<TvShow *> tvShows();
    int hasNewShowOrEpisode();
    void removeShow(TvShow *show);

private slots:
    void onSigChanged(TvShowModelItem *showItem, TvShowModelItem *seasonItem, TvShowModelItem *episodeItem);
    void onShowChanged(TvShow *show);

private:
    TvShowModelItem *m_rootItem;
    QMap<int, QMap<bool, QIcon>> m_icons;
    QIcon m_newIcon;
    QIcon m_syncIcon;
    QIcon m_missingIcon;
};

#endif // TVSHOWMODEL_H
