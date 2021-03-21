#include "Rating.h"

#include <QMap>

Rating::Rating() = default;

QString Rating::sourceToName(const QString& source)
{
    static QMap<QString, QString> r({
        {"thetvdb", "TheTvDb"},
        {"tvdb", "TheTvDb"},
        {"themoviedb", "TMDb"},
        {"tmdb", "TMDb"},
        {"imdb", "IMDb"},
        {"metacritic", "Metacritic"},
        {"ofdb", "OFDb"},
        {"default", "Default"},
        {"thetvdb", "TheTvDb"} //
    });

    if (r.contains(source)) {
        return r[source];
    }
    return source;
}

QStringList Rating::commonSources()
{
    // See https://kodi.wiki/view/NFO_files/Movies
    return {"themoviedb", "imdb", "ofdb", "tvdb", "metacritic", "default"};
}
