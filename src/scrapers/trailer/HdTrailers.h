#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QQueue>
#include <QStringList>

#include "scrapers/trailer/TrailerProvider.h"

class HdTrailers : public TrailerProvider
{
    Q_OBJECT
public:
    explicit HdTrailers(QObject* parent = nullptr);
    QString name() override;

public slots:
    void searchMovie(QString searchStr) override;
    void loadMovieTrailers(QString id) override;

private slots:
    void onSearchFinished();
    void onLoadFinished();

private:
    QNetworkAccessManager* m_qnam;
    QNetworkReply* m_searchReply;
    QNetworkReply* m_loadReply;
    QString m_currentSearch;
    QVector<TrailerResult> parseTrailers(QString html);
    QMap<QString, QUrl> m_urls;
    QQueue<char> m_libraryPages;

    QUrl getLibraryUrl(char library);
};
