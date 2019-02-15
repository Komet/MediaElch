#pragma once

#include <QNetworkAccessManager>
#include <QObject>

class TvShow;
class TheTvDb;

class TvShowUpdater : public QObject
{
    Q_OBJECT
public:
    explicit TvShowUpdater(QObject* parent = nullptr);
    static TvShowUpdater* instance(QObject* parent = nullptr);
    void updateShow(TvShow* show, bool force = false);

private slots:
    void onLoadFinished();

private:
    QNetworkAccessManager m_qnam;
    TheTvDb* m_tvdb;
    QVector<TvShow*> m_updatedShows;
    QString unzipContent(QByteArray content);
};
