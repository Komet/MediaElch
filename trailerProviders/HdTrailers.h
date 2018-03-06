#ifndef HDTRAILERS_H
#define HDTRAILERS_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QQueue>
#include <QStringList>

#include "trailerProviders/TrailerProvider.h"

class HdTrailers : public TrailerProvider
{
    Q_OBJECT
public:
    explicit HdTrailers(QObject *parent = nullptr);
    QString name() override;

public slots:
    void searchMovie(QString searchStr) override;
    void loadMovieTrailers(QString id) override;

signals:
    void sigSearchDone(QList<ScraperSearchResult>) override;
    void sigLoadDone(QList<TrailerResult>) override;

private slots:
    void onSearchFinished();
    void onLoadFinished();

private:
    QNetworkAccessManager *m_qnam;
    QNetworkReply *m_searchReply;
    QNetworkReply *m_loadReply;
    QString m_currentSearch;
    QList<TrailerResult> parseTrailers(QString html);
    QMap<QString, QUrl> m_urls;
    QQueue<QString> m_libraryPages;
};

#endif // HDTRAILERS_H
