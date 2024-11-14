#include "database/TvShowPersistence.h"

#include "data/tv_show/TvShow.h"
#include "database/Database.h"
#include "globals/Manager.h"
#include "log/Log.h"
#include "media_center/kodi/EpisodeXmlWriter.h"

#include <QSqlQuery>
#include <QSqlRecord>

namespace mediaelch {

TvShowPersistence::TvShowPersistence(Database& db) : m_db(db)
{
}

QSqlDatabase TvShowPersistence::db()
{
    return m_db.db();
}

void TvShowPersistence::add(TvShow* show, DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO shows(dir, content, path) "
                  "VALUES(:dir, :content, :path)");
    query.bindValue(":dir", show->dir().toString().toUtf8());
    query.bindValue(":content", show->nfoContent().isEmpty() ? "" : show->nfoContent().toUtf8());
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();

    show->setDatabaseId(query.lastInsertId().toInt());

    query.prepare("SELECT showMissingEpisodes, hideSpecialsInMissingEpisodes FROM showsSettings WHERE dir=:dir");
    query.bindValue(":dir", show->dir().toString().toUtf8());
    query.exec();
    if (query.next()) {
        show->setShowMissingEpisodes(query.value(query.record().indexOf("showMissingEpisodes")).toInt() == 1);
        show->setHideSpecialsInMissingEpisodes(
            query.value(query.record().indexOf("hideSpecialsInMissingEpisodes")).toInt() == 1);
    } else {
        query.prepare("INSERT INTO showsSettings(showMissingEpisodes, hideSpecialsInMissingEpisodes, dir, tmdbid, url) "
                      "VALUES(0, 0, :dir, :tmdbid, :url)");
        query.bindValue(":dir", show->dir().toString().toUtf8());
        query.bindValue(":tmdbid", show->tmdbId().toString());
        query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
        query.exec();
        show->setShowMissingEpisodes(false);
        show->setHideSpecialsInMissingEpisodes(false);
    }
}

void TvShowPersistence::setShowMissingEpisodes(TvShow* show, bool showMissing)
{
    QSqlQuery query(db());

    query.prepare("SELECT showMissingEpisodes FROM showsSettings WHERE dir=:dir");
    query.bindValue(":dir", show->dir().toString().toUtf8());
    query.exec();
    if (query.next()) {
        query.prepare("UPDATE showsSettings SET showMissingEpisodes=:show, url=:url, tmdbid=:tmdbid WHERE dir=:dir");
        query.bindValue(":show", showMissing ? 1 : 0);
        query.bindValue(":dir", show->dir().toString().toUtf8());
        query.bindValue(":tmdbid", show->tmdbId().toString());
        query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
        query.exec();
    } else {
        query.prepare(
            "INSERT INTO showsSettings(showMissingEpisodes, dir, tmdbid, url) VALUES(:show, :dir, :tmdbid, :url)");
        query.bindValue(":dir", show->dir().toString().toUtf8());
        query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
        query.bindValue(":tmdbid", show->tmdbId().toString());
        query.bindValue(":show", showMissing ? 1 : 0);
        query.exec();
    }
}

void TvShowPersistence::setHideSpecialsInMissingEpisodes(TvShow* show, bool hideSpecials)
{
    QSqlQuery query(db());

    query.prepare("SELECT hideSpecialsInMissingEpisodes FROM showsSettings WHERE dir=:dir");
    query.bindValue(":dir", show->dir().toString().toUtf8());
    query.exec();
    if (query.next()) {
        query.prepare(
            "UPDATE showsSettings SET hideSpecialsInMissingEpisodes=:hide, url=:url, tmdbid=:tmdbid WHERE dir=:dir");
        query.bindValue(":show", hideSpecials ? 1 : 0);
        query.bindValue(":dir", show->dir().toString().toUtf8());
        query.bindValue(":tmdbid", show->tmdbId().toString());
        query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
        query.exec();
    } else {
        query.prepare("INSERT INTO showsSettings(hideSpecialsInMissingEpisodes, dir, tmdbid, url) VALUES(:hide, :dir, "
                      ":tmdbid, :url)");
        query.bindValue(":dir", show->dir().toString().toUtf8());
        query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
        query.bindValue(":tmdbid", show->tmdbId().toString());
        query.bindValue(":hide", hideSpecials ? 1 : 0);
        query.exec();
    }
}

void TvShowPersistence::add(TvShowEpisode* episode, DirectoryPath path, mediaelch::DatabaseId idShow)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO episodes(content, idShow, path, seasonNumber, episodeNumber) "
                  "VALUES(:content, :idShow, :path, :seasonNumber, :episodeNumber)");
    query.bindValue(":content", episode->nfoContent().isEmpty() ? "" : episode->nfoContent().toUtf8());
    query.bindValue(":idShow", idShow.toInt());
    query.bindValue(":path", path.toString().toUtf8());
    query.bindValue(":seasonNumber", episode->seasonNumber().toInt());
    query.bindValue(":episodeNumber", episode->episodeNumber().toInt());
    query.exec();
    int insertId = query.lastInsertId().toInt();
    for (const FilePath& file : episode->files()) {
        query.prepare("INSERT INTO episodeFiles(idEpisode, file) VALUES(:idEpisode, :file)");
        query.bindValue(":idEpisode", insertId);
        query.bindValue(":file", file.toString().toUtf8());
        query.exec();
    }
    episode->setDatabaseId(insertId);
}

void TvShowPersistence::update(TvShow* show)
{
    QSqlQuery query(db());
    query.prepare("UPDATE shows SET content=:content, dir=:dir WHERE idShow=:id");
    query.bindValue(":content", show->nfoContent().isEmpty() ? "" : show->nfoContent());
    query.bindValue(":dir", show->dir().toString().toUtf8());
    query.bindValue(":id", show->databaseId().toInt());
    query.exec();

    DatabaseId id = showsSettingsId(show);
    query.prepare("UPDATE showsSettings SET showMissingEpisodes=:show, hideSpecialsInMissingEpisodes=:hide, url=:url, "
                  "tmdbid=:tmdbid WHERE idShow=:idShow");
    query.bindValue(":show", show->showMissingEpisodes());
    query.bindValue(":hide", show->hideSpecialsInMissingEpisodes());
    query.bindValue(":idShow", id.toInt());
    query.bindValue(":tmdbid", show->tmdbId().toString());
    query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
    query.exec();
}

void TvShowPersistence::update(TvShowEpisode* episode)
{
    QSqlQuery query(db());
    query.prepare("UPDATE episodes SET content=:content WHERE idEpisode=:id");
    query.bindValue(":content", episode->nfoContent().isEmpty() ? "" : episode->nfoContent());
    query.bindValue(":id", episode->databaseId().toInt());
    query.exec();

    query.prepare("DELETE FROM episodeFiles WHERE idEpisode=:idEpisode");
    query.bindValue(":idEpisode", episode->databaseId().toInt());
    query.exec();

    for (const FilePath& file : episode->files()) {
        query.prepare("INSERT INTO episodeFiles(idEpisode, file) VALUES(:idEpisode, :file)");
        query.bindValue(":idEpisode", episode->databaseId().toInt());
        query.bindValue(":file", file.toString().toUtf8());
        query.exec();
    }
}

int TvShowPersistence::showCount(DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("SELECT COUNT(*) FROM shows WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    if (!query.next()) {
        return 0;
    }
    bool ok = false;
    int numberOfShows = query.value(0).toInt(&ok);
    return ok ? numberOfShows : 0;
}

QVector<TvShow*> TvShowPersistence::showsInDirectory(DirectoryPath path)
{
    QVector<TvShow*> shows;
    QSqlQuery query(db());
    query.prepare("SELECT idShow, dir, content, path FROM shows WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    while (query.next()) {
        mediaelch::DirectoryPath dir(QString::fromUtf8(query.value(query.record().indexOf("dir")).toByteArray()));
        auto* show = new TvShow(dir, Manager::instance()->tvShowFileSearcher());
        show->setDatabaseId(query.value(query.record().indexOf("idShow")).toInt());
        show->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
        shows.append(show);
    }

    for (TvShow* show : shows) {
        query.prepare("SELECT showMissingEpisodes, hideSpecialsInMissingEpisodes FROM showsSettings WHERE dir=:dir");
        query.bindValue(":dir", show->dir().toString().toUtf8());
        query.exec();
        if (query.next()) {
            show->setShowMissingEpisodes(
                query.value(query.record().indexOf("showMissingEpisodes")).toInt() == 1, false);
            show->setHideSpecialsInMissingEpisodes(
                query.value(query.record().indexOf("hideSpecialsInMissingEpisodes")).toInt() == 1, false);
        }
    }

    return shows;
}

QVector<TvShowEpisode*> TvShowPersistence::episodes(mediaelch::DatabaseId idShow)
{
    QVector<TvShowEpisode*> episodes;
    QSqlQuery query(db());
    QSqlQuery queryFiles(db());
    query.prepare("SELECT idEpisode, content, seasonNumber, episodeNumber FROM episodes WHERE idShow=:idShow");
    query.bindValue(":idShow", idShow.toInt());
    query.exec();
    while (query.next()) {
        QStringList files;
        queryFiles.prepare("SELECT file FROM episodeFiles WHERE idEpisode=:idEpisode");
        queryFiles.bindValue(":idEpisode", query.value(query.record().indexOf("idEpisode")).toInt());
        queryFiles.exec();
        while (queryFiles.next()) {
            files << QString::fromUtf8(queryFiles.value(queryFiles.record().indexOf("file")).toByteArray());
        }

        auto* episode = new TvShowEpisode(files);
        episode->setSeason(SeasonNumber(query.value(query.record().indexOf("seasonNumber")).toInt()));
        episode->setEpisode(EpisodeNumber(query.value(query.record().indexOf("episodeNumber")).toInt()));
        episode->setDatabaseId(query.value(query.record().indexOf("idEpisode")).toInt());
        episode->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
        episodes.append(episode);
    }
    return episodes;
}

void TvShowPersistence::clearAllTvShows()
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM shows");
    query.exec();
    query.prepare("DELETE FROM episodes");
    query.exec();
    query.prepare("DELETE FROM episodeFiles");
    query.exec();
    query.prepare("DELETE FROM sqlite_sequence WHERE name='shows'");
    query.exec();
    query.prepare("DELETE FROM sqlite_sequence WHERE name='episodes'");
    query.exec();
    query.prepare("DELETE FROM sqlite_sequence WHERE name='episodeFiles'");
    query.exec();
}

void TvShowPersistence::clearTvShowsInDirectory(DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM shows WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    query.prepare("DELETE FROM episodeFiles WHERE idEpisode IN (SELECT idEpisode FROM episodes WHERE path=:path)");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    query.prepare("DELETE FROM episodes WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
}

void TvShowPersistence::clearTvShowInDirectory(DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("SELECT idShow FROM shows WHERE dir=:dir");
    query.bindValue(":dir", path.toString().toUtf8());
    query.exec();
    if (!query.next()) {
        return;
    }
    int idShow = query.value(0).toInt();

    query.prepare("DELETE FROM episodeFiles WHERE idEpisode IN (SELECT idEpisode FROM episodes WHERE idShow=:idShow)");
    query.bindValue(":idShow", idShow);
    query.exec();

    query.prepare("DELETE FROM shows WHERE idShow=:idShow");
    query.bindValue(":idShow", idShow);
    query.exec();

    query.prepare("DELETE FROM episodes WHERE idShow=:idShow");
    query.bindValue(":idShow", idShow);
    query.exec();
}

int TvShowPersistence::episodeCount()
{
    QSqlQuery query(db());
    query.prepare("SELECT COUNT(*) FROM episodes");
    query.exec();
    if (query.next()) {
        return query.value(0).toInt();
    } else {
        qCWarning(generic) << "[Database] Query was not successful: Can't retrieve number of episodes";
        return 0;
    }
}

mediaelch::DatabaseId TvShowPersistence::showsSettingsId(TvShow* show)
{
    QSqlQuery query(db());
    query.prepare("SELECT idShow FROM showsSettings WHERE dir=:dir");
    query.bindValue(":dir", show->dir().toString().toUtf8());
    query.exec();
    if (query.next()) {
        return query.value(0).toInt();
    }

    query.prepare("INSERT INTO showsSettings(showMissingEpisodes, hideSpecialsInMissingEpisodes, dir) VALUES(:show, "
                  ":hide, :dir)");
    query.bindValue(":dir", show->dir().toString().toUtf8());
    query.bindValue(":show", 0);
    query.bindValue(":hide", 0);
    query.exec();
    return query.lastInsertId().toInt();
}

void TvShowPersistence::clearEpisodeList(mediaelch::DatabaseId showsSettingsId)
{
    QSqlQuery query(db());
    query.prepare("UPDATE showsEpisodes SET updated=0 WHERE idShow=:idShow");
    query.bindValue(":idShow", showsSettingsId.toInt());
    query.exec();
}

void TvShowPersistence::addEpisodeToShowList(TvShowEpisode* episode,
    mediaelch::DatabaseId showsSettingsId,
    TmdbId tmdbId)
{
    kodi::EpisodeXmlWriterGeneric xmlWriter(KodiVersion::latest(), {episode});
    const QByteArray xmlContent = xmlWriter.getEpisodeXml();

    QSqlQuery query(db());
    query.prepare("SELECT idEpisode FROM showsEpisodes WHERE tmdbid=:tmdbId");
    query.bindValue(":tmdbId", tmdbId.toString());
    query.exec();
    if (query.next()) {
        const int idEpisode = query.value(0).toInt();
        query.prepare("UPDATE showsEpisodes SET seasonNumber=:seasonNumber, episodeNumber=:episodeNumber, updated=1, "
                      "content=:content WHERE idEpisode=:idEpisode");
        query.bindValue(":content", xmlContent.isEmpty() ? "" : xmlContent);
        query.bindValue(":idEpisode", idEpisode);
        query.bindValue(":seasonNumber", episode->seasonNumber().toInt());
        query.bindValue(":episodeNumber", episode->episodeNumber().toInt());
        query.exec();
    } else {
        query.prepare("INSERT INTO showsEpisodes(content, idShow, seasonNumber, episodeNumber, tmdbid, updated) "
                      "VALUES(:content, :idShow, :seasonNumber, :episodeNumber, :tmdbId, 1)");
        query.bindValue(":content", xmlContent.isEmpty() ? "" : xmlContent);
        query.bindValue(":idShow", showsSettingsId.toInt());
        query.bindValue(":seasonNumber", episode->seasonNumber().toInt());
        query.bindValue(":episodeNumber", episode->episodeNumber().toInt());
        query.bindValue(":tmdbId", tmdbId.toString());
        query.exec();
    }
}

void TvShowPersistence::cleanUpEpisodeList(mediaelch::DatabaseId showsSettingsId)
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM showsEpisodes WHERE idShow=:idShow AND updated=0");
    query.bindValue(":idShow", showsSettingsId.toInt());
    query.exec();
}

QVector<TvShowEpisode*> TvShowPersistence::showsEpisodes(TvShow* show)
{
    DatabaseId id = showsSettingsId(show);
    QVector<TvShowEpisode*> episodes;
    QSqlQuery query(db());
    query.prepare("SELECT idEpisode, content, seasonNumber, episodeNumber FROM showsEpisodes WHERE idShow=:idShow");
    query.bindValue(":idShow", id.toInt());
    query.exec();
    while (query.next()) {
        auto* episode = new TvShowEpisode(QStringList(), show);
        episode->setSeason(SeasonNumber(query.value(query.record().indexOf("seasonNumber")).toInt()));
        episode->setEpisode(EpisodeNumber(query.value(query.record().indexOf("episodeNumber")).toInt()));
        episode->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
        episodes.append(episode);
    }
    return episodes;
}


} // namespace mediaelch
