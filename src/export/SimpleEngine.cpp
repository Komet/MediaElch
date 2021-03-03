#include "export/SimpleEngine.h"

#include "concerts/Concert.h"
#include "data/StreamDetails.h"
#include "globals/Manager.h"
#include "movies/Movie.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"

#include <QApplication>
#include <QEventLoop>

static QString colorLabelToString(ColorLabel label)
{
    switch (label) {
    case ColorLabel::NoLabel: return "white";
    case ColorLabel::Red: return "red";
    case ColorLabel::Orange: return "orange";
    case ColorLabel::Yellow: return "yellow";
    case ColorLabel::Green: return "green";
    case ColorLabel::Blue: return "blue";
    case ColorLabel::Purple: return "purple";
    case ColorLabel::Grey: return "grey";
    }
    return "white";
}

namespace mediaelch {

SimpleEngine::SimpleEngine(ExportTemplate& exportTemplate,
    QDir directory,
    std::atomic_bool& cancelFlag,
    QObject* parent) :
    QObject(parent), m_cancelFlag{cancelFlag}, m_template{&exportTemplate}, m_dir{directory}
{
    // Create the base structure
    m_template->copyTo(m_dir.path());
}

void SimpleEngine::exportMovies(QVector<Movie*> movies)
{
    std::sort(movies.begin(), movies.end(), Movie::lessThan);
    QString listContent = m_template->getTemplate(ExportTemplate::ExportSection::Movies);
    QString itemContent = m_template->getTemplate(ExportTemplate::ExportSection::Movie);

    QString listMovieItem;
    QString listMovieBlock;
    QStringList movieList;
    QRegularExpression rx(R"(\{\{ BEGIN_BLOCK_MOVIE \}\}(.*)\{\{ END_BLOCK_MOVIE \}\})", //
        QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);

    QRegularExpressionMatchIterator matches = rx.globalMatch(listContent);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        listMovieBlock = match.captured(0);
        listMovieItem = match.captured(1).trimmed();
    }

    m_dir.mkdir("movies");
    m_dir.mkdir("movie_images");

    for (Movie* movie : movies) {
        if (m_cancelFlag.load()) {
            return;
        }

        // We can't replace an empty block...
        if (!listMovieItem.isEmpty()) {
            QString m = listMovieItem;
            replaceVars(m, movie);
            movieList << m;
        }

        if (!itemContent.isEmpty()) {
            QString movieTemplate = itemContent;
            replaceVars(movieTemplate, movie, true);
            QFile file(m_dir.path() + QStringLiteral("/movies/%1.html").arg(movie->movieId()));
            if (file.open(QFile::WriteOnly | QFile::Text)) {
                file.write(movieTemplate.toUtf8());
                file.close();
            }
        }

        emit sigItemExported();
        QApplication::processEvents();
    }

    // If the movie block is empty, replacing it would result in add a line break after each character
    if (!listMovieBlock.isEmpty()) {
        listContent.replace(listMovieBlock, movieList.join("\n"));
    }

    QFile file(m_dir.path() + "/movies.html");
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        file.write(listContent.toUtf8());
        file.close();
    }
}

void SimpleEngine::replaceVars(QString& m, Movie* movie, bool subDir)
{
    m.replace("{{ MOVIE.ID }}", QString::number(movie->movieId(), 'f', 0));
    m.replace("{{ MOVIE.LINK }}", QString("movies/%1.html").arg(movie->movieId()));
    m.replace("{{ MOVIE.IMDB_ID }}", movie->imdbId().toString());
    m.replace("{{ MOVIE.TMDB_ID }}", movie->tmdbId().toString());
    m.replace("{{ MOVIE.TITLE }}", movie->name().toHtmlEscaped());
    m.replace("{{ MOVIE.YEAR }}", movie->released().isValid() ? movie->released().toString("yyyy") : "");
    m.replace("{{ MOVIE.ORIGINAL_TITLE }}", movie->originalName().toHtmlEscaped());
    m.replace("{{ MOVIE.PLOT }}", movie->overview().toHtmlEscaped().replace("\n", "<br />"));
    m.replace("{{ MOVIE.PLOT_SIMPLE }}", movie->outline().toHtmlEscaped().replace("\n", "<br />"));
    m.replace("{{ MOVIE.SET }}", movie->set().name.toHtmlEscaped());
    m.replace("{{ MOVIE.TAGLINE }}", movie->tagline().toHtmlEscaped());
    m.replace("{{ MOVIE.GENRES }}", movie->genres().join(", ").toHtmlEscaped());
    m.replace("{{ MOVIE.COUNTRIES }}", movie->countries().join(", ").toHtmlEscaped());
    m.replace("{{ MOVIE.STUDIOS }}", movie->studios().join(", ").toHtmlEscaped());
    m.replace("{{ MOVIE.TAGS }}", movie->tags().join(", ").toHtmlEscaped());
    m.replace("{{ MOVIE.WRITER }}", movie->writer().toHtmlEscaped());
    m.replace("{{ MOVIE.DIRECTOR }}", movie->director().toHtmlEscaped());
    m.replace("{{ MOVIE.CERTIFICATION }}", movie->certification().toString().toHtmlEscaped());
    m.replace("{{ MOVIE.TRAILER }}", movie->trailer().toString());
    m.replace("{{ MOVIE.LABEL }}", colorLabelToString(movie->label()));

    // \todo multiple ratings
    if (!movie->ratings().isEmpty()) {
        double rating = movie->ratings().front().rating;
        int voteCount = movie->ratings().front().voteCount;
        m.replace("{{ MOVIE.RATING }}", QString::number(rating, 'f', 1));
        m.replace("{{ MOVIE.VOTES }}", QString::number(voteCount, 'f', 0));
    } else {
        m.replace("{{ MOVIE.RATING }}", "n/a");
        m.replace("{{ MOVIE.VOTES }}", "n/a");
    }

    m.replace("{{ MOVIE.RUNTIME }}", QString::number(movie->runtime().count(), 'f', 0));
    m.replace("{{ MOVIE.PLAY_COUNT }}", QString::number(movie->playcount(), 'f', 0));
    m.replace("{{ MOVIE.LAST_PLAYED }}",
        movie->lastPlayed().isValid() ? movie->lastPlayed().toString("yyyy-MM-dd hh:mm") : "");
    m.replace(
        "{{ MOVIE.DATE_ADDED }}", movie->dateAdded().isValid() ? movie->dateAdded().toString("yyyy-MM-dd hh:mm") : "");
    m.replace("{{ MOVIE.FILE_LAST_MODIFIED }}",
        movie->fileLastModified().isValid() ? movie->fileLastModified().toString("yyyy-MM-dd hh:mm") : "");
    m.replace("{{ MOVIE.FILENAME }}", (!movie->files().isEmpty()) ? movie->files().first().toString() : "");
    if (!movie->files().isEmpty()) {
        QFileInfo fi(movie->files().first().toString());
        m.replace("{{ MOVIE.DIR }}", fi.absolutePath());
    } else {
        m.replace("{{ MOVIE.DIR }}", "");
    }

    replaceSingleBlock(m, "TAGS", "TAG.NAME", movie->tags());
    replaceSingleBlock(m, "GENRES", "GENRE.NAME", movie->genres());
    replaceSingleBlock(m, "COUNTRIES", "COUNTRY.NAME", movie->countries());
    replaceSingleBlock(m, "STUDIOS", "STUDIO.NAME", movie->studios());

    QStringList actorNames;
    QStringList actorRoles;
    for (const Actor* actor : movie->actors()) {
        actorNames << actor->name;
        actorRoles << actor->role;
    }
    replaceMultiBlock(m, "ACTORS", {"ACTOR.NAME", "ACTOR.ROLE"}, QVector<QStringList>() << actorNames << actorRoles);

    replaceStreamDetailsVars(m, movie->streamDetails());
    replaceImages(m, subDir, movie, nullptr, nullptr, nullptr);
}

void SimpleEngine::exportConcerts(QVector<Concert*> concerts)
{
    std::sort(concerts.begin(), concerts.end(), Concert::lessThan);
    QString listContent = m_template->getTemplate(ExportTemplate::ExportSection::Concerts);
    QString itemContent = m_template->getTemplate(ExportTemplate::ExportSection::Concert);

    QString listConcertItem;
    QString listConcertBlock;
    QStringList concertList;
    QRegularExpression rx(R"(\{\{ BEGIN_BLOCK_CONCERT \}\}(.*)\{\{ END_BLOCK_CONCERT \}\})", //
        QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);

    QRegularExpressionMatchIterator matches = rx.globalMatch(listContent);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        listConcertBlock = match.captured(0);
        listConcertItem = match.captured(1).trimmed();
    }

    m_dir.mkdir("concerts");
    m_dir.mkdir("concert_images");

    for (const Concert* concert : concerts) {
        if (m_cancelFlag.load()) {
            return;
        }

        QString concertTemplate = itemContent;
        replaceVars(concertTemplate, concert, true);
        QFile file(m_dir.path() + QString("/concerts/%1.html").arg(concert->concertId()));
        if (file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(concertTemplate.toUtf8());
            file.close();
        }

        QString c = listConcertItem;
        replaceVars(c, concert);
        concertList << c;
        emit sigItemExported();
        QApplication::processEvents();
    }

    listContent.replace(listConcertBlock, concertList.join("\n"));

    QFile file(m_dir.path() + "/concerts.html");
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        file.write(listContent.toUtf8());
        file.close();
    }
}

void SimpleEngine::replaceVars(QString& m, const Concert* concert, bool subDir)
{
    m.replace("{{ CONCERT.ID }}", QString::number(concert->concertId(), 'f', 0));
    m.replace("{{ CONCERT.LINK }}", QString("concerts/%1.html").arg(concert->concertId()));
    m.replace("{{ CONCERT.TITLE }}", concert->title().toHtmlEscaped());
    m.replace("{{ CONCERT.ARTIST }}", concert->artist().toHtmlEscaped());
    m.replace("{{ CONCERT.ALBUM }}", concert->album().toHtmlEscaped());
    m.replace("{{ CONCERT.TAGLINE }}", concert->tagline().toHtmlEscaped());

    if (concert->ratings().isEmpty()) {
        m.replace("{{ CONCERT.RATING }}", "n/a");
    } else {
        m.replace("{{ CONCERT.RATING }}", QString::number(concert->ratings().first().rating, 'f', 1));
    }

    m.replace("{{ CONCERT.YEAR }}", concert->released().isValid() ? concert->released().toString("yyyy") : "");
    m.replace("{{ CONCERT.RUNTIME }}", QString::number(concert->runtime().count(), 'f', 0));
    m.replace("{{ CONCERT.CERTIFICATION }}", concert->certification().toString().toHtmlEscaped());
    m.replace("{{ CONCERT.TRAILER }}", concert->trailer().toString());
    m.replace("{{ CONCERT.PLAY_COUNT }}", QString::number(concert->playcount(), 'f', 0));
    m.replace("{{ CONCERT.LAST_PLAYED }}",
        concert->lastPlayed().isValid() ? concert->lastPlayed().toString("yyyy-MM-dd hh:mm") : "");

    m.replace("{{ CONCERT.FILENAME }}", (!concert->files().isEmpty()) ? concert->files().first().toString() : "");
    if (!concert->files().isEmpty()) {
        QFileInfo fi(concert->files().first().toString());
        m.replace("{{ CONCERT.DIR }}", fi.absolutePath());
    } else {
        m.replace("{{ CONCERT.DIR }}", "");
    }

    m.replace("{{ CONCERT.PLOT }}", concert->overview().toHtmlEscaped().replace("\n", "<br />"));
    m.replace("{{ CONCERT.TAGS }}", concert->tags().join(", ").toHtmlEscaped());
    m.replace("{{ CONCERT.GENRES }}", concert->genres().join(", ").toHtmlEscaped());

    replaceStreamDetailsVars(m, concert->streamDetails());
    replaceSingleBlock(m, "TAGS", "TAG.NAME", concert->tags());
    replaceSingleBlock(m, "GENRES", "GENRE.NAME", concert->genres());
    replaceImages(m, subDir, nullptr, concert, nullptr, nullptr);
}

void SimpleEngine::exportTvShows(QVector<TvShow*> shows)
{
    std::sort(shows.begin(), shows.end(), TvShow::lessThan);
    QString listContent = m_template->getTemplate(ExportTemplate::ExportSection::TvShows);
    QString itemContent = m_template->getTemplate(ExportTemplate::ExportSection::TvShow);
    QString episodeContent = m_template->getTemplate(ExportTemplate::ExportSection::Episode);

    QString listTvShowItem;
    QString listTvShowBlock;
    QStringList tvShowList;
    QRegularExpression rx(R"(\{\{ BEGIN_BLOCK_TVSHOW \}\}(.*)\{\{ END_BLOCK_TVSHOW \}\})", //
        QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);

    QRegularExpressionMatchIterator matches = rx.globalMatch(listContent);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        listTvShowBlock = match.captured(0);
        listTvShowItem = match.captured(1).trimmed();
    }

    m_dir.mkdir("tvshows");
    m_dir.mkdir("tvshow_images");
    m_dir.mkdir("episodes");
    m_dir.mkdir("episode_images");

    for (TvShow* show : shows) {
        if (m_cancelFlag.load()) {
            return;
        }

        // tvshow.html - Single TV show
        {
            QString showTemplate = itemContent;
            replaceVars(showTemplate, show, true);
            QFile file(m_dir.path() + QString("/tvshows/%1.html").arg(show->showId()));
            if (file.open(QFile::WriteOnly | QFile::Text)) {
                file.write(showTemplate.toUtf8());
                file.close();
            }
            QApplication::processEvents();
        }

        // tvshows.html - All TV shows listed
        {
            QString showBlock = listTvShowItem;
            replaceVars(showBlock, show, false);
            tvShowList << showBlock;
            emit sigItemExported();
            QApplication::processEvents();
        }

        // episode.html - Single episode
        for (TvShowEpisode* episode : show->episodes()) {
            if (episode->isDummy()) {
                continue;
            }
            QString episodeTemplate = episodeContent;
            replaceVars(episodeTemplate, episode, true);
            QFile file(m_dir.path() + QString("/episodes/%1.html").arg(episode->episodeId()));
            if (file.open(QFile::WriteOnly | QFile::Text)) {
                file.write(episodeTemplate.toUtf8());
                file.close();
            }
            emit sigItemExported();
            QApplication::processEvents();
        }
    }

    listContent.replace(listTvShowBlock, tvShowList.join("\n"));

    QFile file(m_dir.path() + "/tvshows.html");
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        file.write(listContent.toUtf8());
        file.close();
    }
}

void SimpleEngine::replaceVars(QString& m, const TvShow* show, bool subDir)
{
    m.replace("{{ TVSHOW.ID }}", QString::number(show->showId(), 'f', 0));
    m.replace("{{ TVSHOW.LINK }}", QString("tvshows/%1.html").arg(show->showId()));
    m.replace("{{ TVSHOW.IMDB_ID }}", show->imdbId().toString());
    m.replace("{{ TVSHOW.TITLE }}", show->title().toHtmlEscaped());
    m.replace("{{ TVSHOW.SORTTITLE }}", show->sortTitle().toHtmlEscaped());
    m.replace("{{ TVSHOW.ORIGINALTITLE }}", show->originalTitle().toHtmlEscaped());

    // \todo multiple ratings
    if (!show->ratings().isEmpty()) {
        double rating = show->ratings().front().rating;
        int voteCount = show->ratings().front().voteCount;
        m.replace("{{ TVSHOW.RATING }}", QString::number(rating, 'f', 1));
        m.replace("{{ TVSHOW.VOTES }}", QString::number(voteCount, 'f', 0));
    } else {
        m.replace("{{ TVSHOW.RATING }}", "n/a");
        m.replace("{{ TVSHOW.VOTES }}", "n/a");
    }

    m.replace("{{ TVSHOW.CERTIFICATION }}", show->certification().toString().toHtmlEscaped());
    m.replace(
        "{{ TVSHOW.FIRST_AIRED }}", show->firstAired().isValid() ? show->firstAired().toString("yyyy-MM-dd") : "");
    m.replace("{{ TVSHOW.STUDIO }}", show->network().toHtmlEscaped());
    m.replace("{{ TVSHOW.PLOT }}", show->overview().toHtmlEscaped().replace("\n", "<br />"));
    m.replace("{{ TVSHOW.TAGS }}", show->tags().join(", ").toHtmlEscaped());
    m.replace("{{ TVSHOW.GENRES }}", show->genres().join(", ").toHtmlEscaped());
    m.replace("{{ TVSHOW.SEASONS_AMOUNT }}", QString::number(show->seasons(false).size()));

    QStringList actorNames;
    QStringList actorRoles;
    for (const Actor* actor : show->actors()) {
        actorNames << actor->name;
        actorRoles << actor->role;
    }
    replaceMultiBlock(m, "ACTORS", {"ACTOR.NAME", "ACTOR.ROLE"}, {actorNames, actorRoles});
    replaceSingleBlock(m, "TAGS", "TAG.NAME", show->tags());
    replaceSingleBlock(m, "GENRES", "GENRE.NAME", show->genres());

    QString listSeasonItem;
    QString listSeasonBlock;
    QStringList seasonList;

    QRegularExpression rx(R"(\{\{ BEGIN_BLOCK_SEASON \}\}(.*)\{\{ END_BLOCK_SEASON \}\})", //
        QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);

    {
        QRegularExpressionMatchIterator matches = rx.globalMatch(m);
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            listSeasonBlock = match.captured(0);
            listSeasonItem = match.captured(1).trimmed();
        }
    }

    if (listSeasonBlock.isEmpty() || listSeasonItem.isEmpty()) {
        replaceImages(m, subDir, nullptr, nullptr, show, nullptr);
        return;
    }

    QVector<SeasonNumber> seasons = show->seasons(false);
    std::sort(seasons.begin(), seasons.end());
    for (const SeasonNumber& season : asConst(seasons)) {
        QVector<TvShowEpisode*> episodes = show->episodes(season);
        std::sort(episodes.begin(), episodes.end(), TvShowEpisode::lessThan);
        QString s = listSeasonItem;
        s.replace("{{ SEASON }}", season.toString());

        QString listEpisodeItem;
        QString listEpisodeBlock;
        QStringList episodeList;

        rx.setPattern(R"(\{\{ BEGIN_BLOCK_EPISODE \}\}(.*)\{\{ END_BLOCK_EPISODE \}\})");
        QRegularExpressionMatchIterator matches = rx.globalMatch(s);
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            listEpisodeBlock = match.captured(0);
            listEpisodeItem = match.captured(1).trimmed();
        }

        if (!listEpisodeItem.isEmpty()) {
            for (TvShowEpisode* episode : asConst(episodes)) {
                QString e = listEpisodeItem;
                replaceVars(e, episode, subDir);
                episodeList << e;
            }
        }
        s.replace(listEpisodeBlock, episodeList.join("\n"));
        seasonList << s;
    }

    m.replace(listSeasonBlock, seasonList.join("\n"));
    replaceImages(m, subDir, nullptr, nullptr, show, nullptr);
}

void SimpleEngine::replaceVars(QString& m, const TvShowEpisode* episode, bool subDir)
{
    m.replace("{{ SHOW.TITLE }}", episode->tvShow()->title().toHtmlEscaped());
    m.replace("{{ SHOW.LINK }}", QString("../tvshows/%1.html").arg(episode->tvShow()->showId()));
    m.replace("{{ EPISODE.LINK }}", QString("../episodes/%1.html").arg(episode->episodeId()));
    m.replace("{{ EPISODE.TITLE }}", episode->title().toHtmlEscaped());
    m.replace("{{ EPISODE.SEASON }}", episode->seasonString().toHtmlEscaped());
    m.replace("{{ EPISODE.EPISODE }}", episode->episodeString().toHtmlEscaped());
    if (episode->ratings().isEmpty()) {
        m.replace("{{ EPISODE.RATING }}", "n/a");
    } else {
        m.replace("{{ EPISODE.RATING }}", QString::number(episode->ratings().first().rating, 'f', 1));
    }
    m.replace("{{ EPISODE.CERTIFICATION }}", episode->certification().toString().toHtmlEscaped());
    m.replace("{{ EPISODE.FIRST_AIRED }}",
        episode->firstAired().isValid() ? episode->firstAired().toString("yyyy-MM-dd") : "");
    m.replace("{{ EPISODE.LAST_PLAYED }}",
        episode->lastPlayed().isValid() ? episode->lastPlayed().toString("yyyy-MM-dd hh:mm") : "");
    m.replace("{{ EPISODE.STUDIO }}", episode->network().toHtmlEscaped());
    m.replace("{{ EPISODE.PLOT }}", episode->overview().toHtmlEscaped().replace("\n", "<br />"));
    m.replace("{{ EPISODE.WRITERS }}", episode->writers().join(", ").toHtmlEscaped());
    m.replace("{{ EPISODE.DIRECTORS }}", episode->directors().join(", ").toHtmlEscaped());

    if (!episode->files().isEmpty()) {
        QFileInfo fi(episode->files().first().toString());
        m.replace("{{ EPISODE.DIR }}", fi.absolutePath());
    } else {
        m.replace("{{ EPISODE.DIR }}", "");
    }
    m.replace("{{ EPISODE.FILENAME }}", (!episode->files().isEmpty()) ? episode->files().first().toString() : "");

    replaceStreamDetailsVars(m, episode->streamDetails());
    replaceSingleBlock(m, "WRITERS", "WRITER.NAME", episode->writers());
    replaceSingleBlock(m, "DIRECTORS", "DIRECTOR.NAME", episode->directors());
    replaceImages(m, subDir, nullptr, nullptr, nullptr, episode);
}

void SimpleEngine::replaceStreamDetailsVars(QString& m, const StreamDetails* details)
{
    const auto videoDetails = (details != nullptr) ? details->videoDetails() : decltype(details->videoDetails()){};
    const auto audioDetails = (details != nullptr) ? details->audioDetails() : decltype(details->audioDetails()){};

    m.replace("{{ FILEINFO.WIDTH }}", videoDetails.value(StreamDetails::VideoDetails::Width, "0"));
    m.replace("{{ FILEINFO.HEIGHT }}", videoDetails.value(StreamDetails::VideoDetails::Height, "0"));
    m.replace("{{ FILEINFO.ASPECT }}", videoDetails.value(StreamDetails::VideoDetails::Aspect, "0"));
    m.replace("{{ FILEINFO.CODEC }}", videoDetails.value(StreamDetails::VideoDetails::Codec, ""));
    m.replace("{{ FILEINFO.DURATION }}", videoDetails.value(StreamDetails::VideoDetails::DurationInSeconds, "0"));

    QStringList audioCodecs;
    QStringList audioChannels;
    QStringList audioLanguages;
    for (int i = 0, n = audioDetails.count(); i < n; ++i) {
        audioCodecs << audioDetails.at(i).value(StreamDetails::AudioDetails::Codec);
        audioChannels << audioDetails.at(i).value(StreamDetails::AudioDetails::Channels);
        audioLanguages << audioDetails.at(i).value(StreamDetails::AudioDetails::Language);
    }
    m.replace("{{ FILEINFO.AUDIO.CODEC }}", audioCodecs.join("|"));
    m.replace("{{ FILEINFO.AUDIO.CHANNELS }}", audioChannels.join("|"));
    m.replace("{{ FILEINFO.AUDIO.LANGUAGE }}", audioLanguages.join("|"));

    QStringList subtitleLanguages;
    if (details != nullptr) {
        for (auto& subtitle : details->subtitleDetails()) {
            subtitleLanguages << subtitle.value(StreamDetails::SubtitleDetails::Language);
        }
    }
    m.replace("{{ FILEINFO.SUBTITLES.LANGUAGE }}", subtitleLanguages.join("|"));
}

void SimpleEngine::replaceSingleBlock(QString& m, QString blockName, QString itemName, QStringList replaces)
{
    replaceMultiBlock(m, blockName, QStringList() << itemName, QVector<QStringList>() << replaces);
}

void SimpleEngine::replaceMultiBlock(QString& m,
    QString blockName,
    QStringList itemNames,
    QVector<QStringList> replaces)
{
    QRegularExpression rx("\\{\\{ BEGIN_BLOCK_" + blockName + R"( \}\}(.*)\{\{ END_BLOCK_)" + blockName + " \\}\\}", //
        QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = rx.match(m);
    if (match.hasMatch()) {
        QString block = match.captured(0);
        QString item = match.captured(1).trimmed();
        QStringList list;
        for (int i = 0, n = replaces.at(0).count(); i < n; ++i) {
            QString subItem = item;
            for (int x = 0, y = itemNames.count(); x < y; ++x) {
                subItem.replace("{{ " + itemNames.at(x) + " }}", replaces.at(x).at(i).toHtmlEscaped());
            }
            list << subItem;
        }
        m.replace(block, list.join(" "));
    }
}

void SimpleEngine::saveImage(QSize size, QString imageFile, QString destinationFile, const char* format, int quality)
{
    Q_UNUSED(format)
    Q_UNUSED(quality)

    QImage img(imageFile);
    if (img.isNull()) {
        qWarning() << "[Export][SimpleEngine] Cannot load image:" << imageFile;
        return;
    }

    img = img.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (!img.isNull()) {
        img.save(destinationFile);
    } else {
        qWarning() << "[Export][SimpleEngine] Could not scale (result was empty):" << imageFile;
    }
}

void SimpleEngine::replaceImages(QString& m,
    const bool& subDir,
    const Movie* movie,
    const Concert* concert,
    const TvShow* tvShow,
    const TvShowEpisode* episode)
{
    QString item;
    QSize size;
    QRegularExpression rx(R"(\{\{ IMAGE.(.*)\[(\d*), ?(\d*)\] \}\})", //
        QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator matches = rx.globalMatch(m);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        item = match.captured(0);
        QString type = match.captured(1).toLower();
        size.setWidth(match.captured(2).toInt());
        size.setHeight(match.captured(3).toInt());

        if (item.isEmpty() || size.isEmpty()) {
            continue;
        }

        QString destFile;
        bool imageSaved = false;
        bool isPlaceholderUsed = true;
        QString typeName;
        if (movie != nullptr) {
            imageSaved = saveImageForType(type, size, destFile, movie, &isPlaceholderUsed);
            typeName = "movie";

        } else if (concert != nullptr) {
            imageSaved = saveImageForType(type, size, destFile, concert, &isPlaceholderUsed);
            typeName = "concert";

        } else if (tvShow != nullptr) {
            imageSaved = saveImageForType(type, size, destFile, tvShow, &isPlaceholderUsed);
            typeName = "tvshow";

        } else if (episode != nullptr) {
            imageSaved = saveImageForType(type, size, destFile, episode, &isPlaceholderUsed);
            typeName = "episode";
        }

        if (!isPlaceholderUsed) {
            continue;
        }

        if (imageSaved) {
            m.replace(item, (subDir ? "../" : "") + destFile);
        } else {
            m.replace(item,
                (subDir ? "../" : "")
                    + QString("defaults/%1_%2_%3x%4.png").arg(typeName).arg(type).arg(size.width()).arg(size.height()));
        }
    }
}

bool SimpleEngine::saveImageForType(const QString& type,
    const QSize& size,
    QString& destFile,
    const Movie* movie,
    bool* isPlaceHolderUsed)
{
    std::string imageFormat = "png";
    ImageType imageType;

    if (type == "poster") {
        imageType = ImageType::MoviePoster;
        imageFormat = "jpg";
    } else if (type == "fanart") {
        imageType = ImageType::MovieBackdrop;
        imageFormat = "jpg";
    } else if (type == "logo") {
        imageType = ImageType::MovieLogo;
    } else if (type == "clearart") {
        imageType = ImageType::MovieClearArt;
    } else if (type == "disc") {
        imageType = ImageType::MovieCdArt;
    } else {
        *isPlaceHolderUsed = false;
        return false;
    }

    *isPlaceHolderUsed = true;

    QString file_ending = QString::fromStdString(imageFormat);
    destFile = "movie_images/"
               + QString("%1-%2_%3x%4.%5")
                     .arg(movie->movieId())
                     .arg(type)
                     .arg(size.width())
                     .arg(size.height())
                     .arg(file_ending);

    QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(movie, imageType);
    if (filename.isEmpty()) {
        return false;
    }

    int imageQuality = (imageFormat == "jpg") ? 90 : -1;
    saveImage(size, filename, m_dir.path() + "/" + destFile, imageFormat.c_str(), imageQuality);

    return true;
}

bool SimpleEngine::saveImageForType(const QString& type,
    const QSize& size,
    QString& destFile,
    const Concert* concert,
    bool* isPlaceHolderUsed)
{
    std::string imageFormat = "png";
    ImageType imageType;

    if (type == "poster") {
        imageType = ImageType::ConcertPoster;
        imageFormat = "jpg";
    } else if (type == "fanart") {
        imageType = ImageType::ConcertBackdrop;
        imageFormat = "jpg";
    } else if (type == "logo") {
        imageType = ImageType::ConcertLogo;
    } else if (type == "clearart") {
        imageType = ImageType::ConcertClearArt;
    } else if (type == "disc") {
        imageType = ImageType::ConcertCdArt;
    } else {
        *isPlaceHolderUsed = false;
        return false;
    }

    *isPlaceHolderUsed = true;

    QString file_ending = QString::fromStdString(imageFormat);
    destFile = "movie_images/"
               + QString("%1-%2_%3x%4.%5")
                     .arg(concert->concertId())
                     .arg(type)
                     .arg(size.width())
                     .arg(size.height())
                     .arg(file_ending);

    QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(concert, imageType);
    if (filename.isEmpty()) {
        return false;
    }

    int imageQuality = (imageFormat == "jpg") ? 90 : -1;
    saveImage(size, filename, m_dir.path() + "/" + destFile, imageFormat.c_str(), imageQuality);

    return true;
}

bool SimpleEngine::saveImageForType(const QString& type,
    const QSize& size,
    QString& destFile,
    const TvShow* tvShow,
    bool* isPlaceHolderUsed)
{
    std::string imageFormat = "png";
    ImageType imageType;

    if (type == "poster") {
        imageType = ImageType::TvShowPoster;
        imageFormat = "jpg";
    } else if (type == "fanart") {
        imageType = ImageType::TvShowBackdrop;
        imageFormat = "jpg";
    } else if (type == "banner") {
        imageType = ImageType::TvShowBanner;
        imageFormat = "jpg";
    } else if (type == "logo") {
        imageType = ImageType::TvShowLogos;
    } else if (type == "clearart") {
        imageType = ImageType::TvShowClearArt;
    } else if (type == "characterart") {
        imageType = ImageType::TvShowCharacterArt;
    } else {
        *isPlaceHolderUsed = false;
        return false;
    }
    *isPlaceHolderUsed = true;

    QString file_ending = QString::fromStdString(imageFormat);
    destFile = "tvshow_images/"
               + QString("%1-%2_%3x%4.%5")
                     .arg(tvShow->showId())
                     .arg(type)
                     .arg(size.width())
                     .arg(size.height())
                     .arg(file_ending);

    QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(tvShow, imageType);
    if (filename.isEmpty()) {
        return false;
    }

    int imageQuality = (imageFormat == "jpg") ? 90 : -1;
    saveImage(size, filename, m_dir.path() + "/" + destFile, imageFormat.c_str(), imageQuality);

    return true;
}

bool SimpleEngine::saveImageForType(const QString& type,
    const QSize& size,
    QString& destFile,
    const TvShowEpisode* episode,
    bool* isPlaceHolderUsed)
{
    destFile = "episode_images/"
               + QString("%1-%2_%3x%4.jpg").arg(episode->episodeId()).arg(type).arg(size.width()).arg(size.height());

    if (type == "thumbnail") {
        *isPlaceHolderUsed = true;
        QString filename =
            Manager::instance()->mediaCenterInterface()->imageFileName(episode, ImageType::TvShowEpisodeThumb);
        if (filename.isEmpty()) {
            return false;
        }
        saveImage(size, filename, m_dir.path() + "/" + destFile, "jpg", 90);
    } else {
        *isPlaceHolderUsed = false;
        return false;
    }

    return true;
}

} // namespace mediaelch
