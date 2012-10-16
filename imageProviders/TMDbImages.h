#ifndef TMDBIMAGES_H
#define TMDBIMAGES_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "data/ImageProviderInterface.h"

/**
 * @brief The TMDbImages class
 */
class TMDbImages : public ImageProviderInterface
{
    Q_OBJECT
public:
    explicit TMDbImages(QObject *parent = 0);
    static TMDbImages *instance(QObject *parent = 0);
    QString name();
    void moviePosters(QString tmdbId);
    void movieBackdrops(QString tmdbId);
    void movieLogos(QString tmdbId);
    void movieClearArts(QString tmdbId);
    void movieCdArts(QString tmdbId);
    void concertPosters(QString tmdbId);
    void concertBackdrops(QString tmdbId);
    void concertLogos(QString tmdbId);
    void concertClearArts(QString tmdbId);
    void concertCdArts(QString tmdbId);
    QList<int> provides();

public slots:
    void searchMovie(QString searchStr, int limit = 0);
    void searchConcert(QString searchStr, int limit = 0);

signals:
    void sigSearchDone(QList<ScraperSearchResult>);
    void sigImagesLoaded(QList<Poster>);

private slots:
    void onSetupFinished();
    void onSearchMovieFinished();
    void onLoadPostersFinished();
    void onLoadBackdropsFinished();

private:
    QList<int> m_provides;
    QString m_apiKey;
    QNetworkAccessManager m_qnam;
    QNetworkReply *m_searchReply;
    QNetworkReply *m_setupReply;
    QNetworkReply *m_loadReply;
    QString m_language;
    QString m_baseUrl;
    QList<ScraperSearchResult> m_results;
    QString m_searchString;
    int m_searchResultLimit;

    void setup();
    QNetworkAccessManager *qnam();
    QList<Poster> parsePosters(QString json);
    QList<Poster> parseBackdrops(QString json);
};

#endif // TMDBIMAGES_H
