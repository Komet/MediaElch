#pragma once

#include "data/MusicScraperInterface.h"

#include <QComboBox>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QObject>
#include <QWidget>

class UniversalMusicScraper : public MusicScraperInterface
{
    Q_OBJECT
public:
    explicit UniversalMusicScraper(QObject *parent = nullptr);

    QString name() const override;
    QString identifier() const override;
    void searchAlbum(QString artistName, QString searchStr) override;
    void searchArtist(QString searchStr) override;
    void loadData(QString mbId, Artist *artist, QList<MusicScraperInfos> infos) override;
    void loadData(QString mbAlbumId, QString mbReleaseGroupId, Album *album, QList<MusicScraperInfos> infos) override;
    bool hasSettings() const override;
    void loadSettings(const ScraperSettings &settings) override;
    void saveSettings(ScraperSettings &settings) override;
    QList<MusicScraperInfos> scraperSupports() override;
    QWidget *settingsWidget() override;

signals:
    void sigSearchDone(QList<ScraperSearchResult>) override;

private slots:
    void onSearchArtistFinished();
    void onSearchAlbumFinished();
    void onArtistRelsFinished();
    void onAlbumRelsFinished();
    void onArtistLoadFinished();
    void onAlbumLoadFinished();

private:
    struct DownloadElement
    {
        QString source;
        QString type;
        QUrl url;
        bool downloaded;
        QString contents;
    };

    QString m_tadbApiKey;
    QNetworkAccessManager m_qnam;
    QString m_language;
    QString m_prefer;
    QWidget *m_widget;
    QComboBox *m_box;
    QComboBox *m_preferBox;
    QMap<Artist *, QList<DownloadElement>> m_artistDownloads;
    QMap<Album *, QList<DownloadElement>> m_albumDownloads;
    QMutex m_artistMutex;
    QMutex m_albumMutex;

    QNetworkAccessManager *qnam();
    QString trim(QString text);
    bool shouldLoad(MusicScraperInfos info, QList<MusicScraperInfos> infos, Artist *artist);
    bool shouldLoad(MusicScraperInfos info, QList<MusicScraperInfos> infos, Album *album);
    bool infosLeft(QList<MusicScraperInfos> infos, Artist *artist);
    bool infosLeft(QList<MusicScraperInfos> infos, Album *album);
    void appendDownloadElement(Artist *artist, QString source, QString type, QUrl url);
    void appendDownloadElement(Album *album, QString source, QString type, QUrl url);
    void parseAndAssignMusicbrainzInfos(QString xml, Album *album, QList<MusicScraperInfos> infos);
    void parseAndAssignTadbInfos(QJsonObject document, Artist *artist, QList<MusicScraperInfos> infos);
    void parseAndAssignTadbInfos(QJsonObject document, Album *album, QList<MusicScraperInfos> infos);
    void parseAndAssignTadbDiscography(QJsonObject document, Artist *artist, QList<MusicScraperInfos> infos);
    void parseAndAssignAmInfos(QString html, Artist *artist, QList<MusicScraperInfos> infos);
    void parseAndAssignAmInfos(QString html, Album *album, QList<MusicScraperInfos> infos);
    void parseAndAssignAmBiography(QString html, Artist *artist, QList<MusicScraperInfos> infos);
    void parseAndAssignAmDiscography(QString html, Artist *artist, QList<MusicScraperInfos> infos);
    void parseAndAssignDiscogsInfos(QString html, Artist *artist, QList<MusicScraperInfos> infos);
    void parseAndAssignDiscogsInfos(QString html, Album *album, QList<MusicScraperInfos> infos);
    void processDownloadElement(DownloadElement elem, Artist *artist, QList<MusicScraperInfos> infos);
    void processDownloadElement(DownloadElement elem, Album *album, QList<MusicScraperInfos> infos);
};
