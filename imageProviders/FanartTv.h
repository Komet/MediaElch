#ifndef FANARTTV_H
#define FANARTTV_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include "globals/Globals.h"
#include "data/ImageProviderInterface.h"

/**
 * @brief The FanartTv class
 */
class FanartTv : public ImageProviderInterface
{
    Q_OBJECT
public:
    explicit FanartTv(QObject *parent = 0);
    QString name();
    void moviePosters(QString tmdbId);
    void movieBackdrops(QString tmdbId);
    QList<int> provides();

public slots:
    void searchMovie(QString searchStr);

signals:
    void sigSearchDone(QList<ScraperSearchResult>);
    void sigImagesLoaded(QList<Poster>);

private slots:
    void onSetupFinished();
    void onSearchMovieFinished();
    void onLoadBackdropsFinished();

private:
    QList<int> m_provides;
    QString m_apiKey;
    QString m_tmdbApiKey;
    QNetworkAccessManager m_qnam;
    QNetworkReply *m_searchReply;
    QNetworkReply *m_setupReply;
    QNetworkReply *m_loadReply;
    QString m_tmdbLanguage;
    QString m_tmdbBaseUrl;
    QList<ScraperSearchResult> m_results;
    QString m_searchString;

    void setup();
    QNetworkAccessManager *qnam();
    QList<Poster> parseBackdrops(QString json);
};

#endif // FANARTTV_H
