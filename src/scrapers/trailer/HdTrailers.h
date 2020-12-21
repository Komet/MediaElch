#pragma once

#include "network/NetworkManager.h"
#include "scrapers/trailer/TrailerProvider.h"

#include <QMultiMap>
#include <QNetworkReply>
#include <QObject>
#include <QQueue>
#include <QStringList>

namespace mediaelch {
namespace scraper {

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
    mediaelch::network::NetworkManager* m_network;
    QNetworkReply* m_searchReply;
    QNetworkReply* m_loadReply;
    QString m_currentSearch;
    QVector<TrailerResult> parseTrailers(QString html);
    QMultiMap<QString, QUrl> m_urls;
    QQueue<char> m_libraryPages;

    QUrl getLibraryUrl(char library);
};

} // namespace scraper
} // namespace mediaelch
