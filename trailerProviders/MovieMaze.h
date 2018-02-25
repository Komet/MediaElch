#ifndef MOVIEMAZE_H
#define MOVIEMAZE_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>
#include <QStringList>
#include "trailerProviders/TrailerProvider.h"

class MovieMaze : public TrailerProvider
{
    Q_OBJECT
public:
    explicit MovieMaze(QObject *parent = nullptr);
    QString name();

public slots:
    void searchMovie(QString searchStr);
    void loadMovieTrailers(QString id);

signals:
    void sigSearchDone(QList<ScraperSearchResult>);
    void sigLoadDone(QList<TrailerResult>);

private slots:
    void onSearchFinished();
    void onLoadFinished();
    void onSubLoadFinished();
    void onLoadPreviewImageFinished();

private:
    QNetworkAccessManager *m_qnam;
    QNetworkReply *m_searchReply;
    QPointer<QNetworkReply> m_loadReply;
    QNetworkReply *m_previewLoadReply;
    QList<TrailerResult> m_currentTrailers;
    QStringList m_trailerSites;
    QString m_currentSearch;
    int m_currentPreviewLoad;
    QList<TrailerResult> parseTrailers(QString html);
    void loadPreviewImages();
};

#endif // MOVIEMAZE_H
