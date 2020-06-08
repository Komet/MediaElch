#pragma once

#include "globals/ScraperInfos.h"
#include "scrapers/music/MusicScraperInterface.h"

#include <QComboBox>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QObject>
#include <QWidget>

class UniversalMusicScraper : public MusicScraperInterface
{
    Q_OBJECT
public:
    explicit UniversalMusicScraper(QObject* parent = nullptr);
    static constexpr const char* scraperIdentifier = "UniversalMusicScraper";

    QString name() const override;
    QString identifier() const override;
    void searchAlbum(QString artistName, QString searchStr) override;
    void searchArtist(QString searchStr) override;
    void loadData(QString mbId, Artist* artist, QSet<MusicScraperInfo> infos) override;
    void loadData(QString mbAlbumId, QString mbReleaseGroupId, Album* album, QSet<MusicScraperInfo> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QSet<MusicScraperInfo> scraperSupports() override;
    QWidget* settingsWidget() override;

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
    QWidget* m_widget;
    QComboBox* m_box;
    QComboBox* m_preferBox;
    QMap<Artist*, QVector<DownloadElement>> m_artistDownloads;
    QMap<Album*, QVector<DownloadElement>> m_albumDownloads;
    QMutex m_artistMutex;
    QMutex m_albumMutex;

    QNetworkAccessManager* qnam();
    QString trim(QString text);
    bool shouldLoad(MusicScraperInfo info, QSet<MusicScraperInfo> infos, Artist* artist);
    bool shouldLoad(MusicScraperInfo info, QSet<MusicScraperInfo> infos, Album* album);
    bool infosLeft(QSet<MusicScraperInfo> infos, Artist* artist);
    bool infosLeft(QSet<MusicScraperInfo> infos, Album* album);
    void appendDownloadElement(Artist* artist, QString source, QString type, QUrl url);
    void appendDownloadElement(Album* album, QString source, QString type, QUrl url);
    void parseAndAssignMusicbrainzInfos(QString xml, Album* album, QSet<MusicScraperInfo> infos);
    void parseAndAssignTadbInfos(QJsonObject document, Artist* artist, QSet<MusicScraperInfo> infos);
    void parseAndAssignTadbInfos(QJsonObject document, Album* album, QSet<MusicScraperInfo> infos);
    void parseAndAssignTadbDiscography(QJsonObject document, Artist* artist, QSet<MusicScraperInfo> infos);
    void parseAndAssignAmInfos(QString html, Artist* artist, QSet<MusicScraperInfo> infos);
    void parseAndAssignAmInfos(QString html, Album* album, QSet<MusicScraperInfo> infos);
    void parseAndAssignAmBiography(QString html, Artist* artist, QSet<MusicScraperInfo> infos);
    void parseAndAssignAmDiscography(QString html, Artist* artist, QSet<MusicScraperInfo> infos);
    void parseAndAssignDiscogsInfos(QString html, Artist* artist, QSet<MusicScraperInfo> infos);
    void parseAndAssignDiscogsInfos(QString html, Album* album, QSet<MusicScraperInfo> infos);
    void processDownloadElement(DownloadElement elem, Artist* artist, QSet<MusicScraperInfo> infos);
    void processDownloadElement(DownloadElement elem, Album* album, QSet<MusicScraperInfo> infos);
};
