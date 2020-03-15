#pragma once

#include "data/Storage.h"
#include "globals/ScraperInfos.h"
#include "movies/Movie.h"
#include "network/NetworkReplyWatcher.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

class IMDB;

class ImdbMovieLoader : public QObject
{
    Q_OBJECT
public:
    ImdbMovieLoader(IMDB& scraper,
        QString imdbId,
        Movie& movie,
        QVector<MovieScraperInfos> infos,
        bool loadAllTags,
        QObject* parent = nullptr) :
        QObject(parent),
        m_scraper{scraper},
        m_imdbId{std::move(imdbId)},
        m_movie{movie},
        m_infos{std::move(infos)},
        m_loadAllTags{loadAllTags}
    {
    }

    void load();

signals:
    void sigLoadDone(Movie& movie, ImdbMovieLoader* loader);

private slots:
    void onLoadFinished();
    void onPosterLoadFinished();
    void onTagsFinished();

    void onActorImageUrlLoadDone();

private:
    void loadPoster(const QUrl& posterViewerUrl);
    void loadTags();
    void loadActorImageUrls();

    void parseAndAssignInfos(const QString& html);
    void parseAndAssignPoster(const QString& html, QString posterId);
    void parseAndStoreActors(const QString& html);
    QUrl parsePoster(const QString& html);
    void parseAndAssignTags(const QString& html);
    QString parseActorImageUrl(const QString& html);

    void mergeActors();
    void decreaseDownloadCount();

private:
    QMutex m_mutex;
    int m_itemsLeftToDownloads = 0;

    IMDB& m_scraper;
    QString m_imdbId;
    Movie& m_movie;
    QVector<MovieScraperInfos> m_infos;
    QNetworkAccessManager m_qnam;
    bool m_loadAllTags = false;

    QVector<QPair<Actor, QUrl>> m_actorUrls;
};
