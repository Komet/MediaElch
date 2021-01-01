#pragma once

#include "globals/ScraperInfos.h"

#include <QObject>

class Artist;
class Album;

namespace mediaelch {
namespace scraper {

class Discogs : public QObject
{
    Q_OBJECT
public:
    explicit Discogs(QObject* parent = nullptr);
    ~Discogs() override = default;

public:
    void parseAndAssignArtist(const QString& html, Artist* artist, QSet<MusicScraperInfo> infos);
    void parseAndAssignAlbum(const QString& html, Album* album, QSet<MusicScraperInfo> infos);

private:
    QString trim(QString text);
};

} // namespace scraper
} // namespace mediaelch
