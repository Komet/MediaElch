#ifndef TVSHOWEPISODE_H
#define TVSHOWEPISODE_H

#include <QMetaType>
#include <QObject>
#include <QStringList>
#include "data/TvShow.h"

class TvShow;

class TvShowEpisode : public QObject
{
    Q_OBJECT
public:
    explicit TvShowEpisode(QStringList files = QStringList(), TvShow *parent = 0);
    QString name();

private:
    QStringList m_files;
    TvShow *m_parent;
};

Q_DECLARE_METATYPE(TvShowEpisode*)

#endif // TVSHOWEPISODE_H
