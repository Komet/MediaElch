#pragma once

#include <QAbstractItemModel>
#include <QIcon>

class Concert;

class ConcertModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum ConcertRoles
    {
        InfoLoadedRole = Qt::UserRole + 1,
        HasChangedRole = Qt::UserRole + 2,
        SyncNeededRole = Qt::UserRole + 3,
        FileRole = Qt::UserRole + 4,
        FileNameRole,
    };
    explicit ConcertModel(QObject* parent = nullptr);
    void addConcert(Concert* concert);
    void clear();
    QVector<Concert*> concerts();
    Concert* concert(int row);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int countNewConcerts() const;
    void update();

private slots:
    void onConcertChanged(Concert* concert);

private:
    QVector<Concert*> m_concerts;
    QIcon m_newIcon;
    QIcon m_syncIcon;
};
