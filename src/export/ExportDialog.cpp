#include "ExportDialog.h"
#include "ui_ExportDialog.h"

#include <QFileDialog>
#include <string>

#include "data/Concert.h"
#include "data/Movie.h"
#include "data/StreamDetails.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "export/ExportTemplateLoader.h"
#include "globals/Manager.h"

ExportDialog::ExportDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ExportDialog)
{
    ui->setupUi(this);
#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

int ExportDialog::exec()
{
    m_canceled = false;
    QVector<ExportTemplate *> templates = ExportTemplateLoader::instance()->installedTemplates();
    if (templates.isEmpty()) {
        ui->message->setErrorMessage(tr("You need to install at least one theme."));
        ui->btnExport->setEnabled(false);
    } else {
        ui->message->clear();
        ui->btnExport->setEnabled(true);
    }

    ui->progressBar->setValue(0);
    ui->comboTheme->clear();
    for (ExportTemplate *exportTemplate : templates) {
        ui->comboTheme->addItem(exportTemplate->name(), exportTemplate->identifier());
    }
    onThemeChanged();
    adjustSize();
    return QDialog::exec();
}

void ExportDialog::onBtnExport()
{
    ui->message->clear();

    int index = ui->comboTheme->currentIndex();
    if (index < 0 || index >= ui->comboTheme->count()) {
        return;
    }

    QVector<ExportTemplate::ExportSection> sections;
    if (ui->chkConcerts->isEnabled() && ui->chkConcerts->isChecked()) {
        sections << ExportTemplate::ExportSection::Concerts;
    }
    if (ui->chkMovies->isEnabled() && ui->chkMovies->isChecked()) {
        sections << ExportTemplate::ExportSection::Movies;
    }
    if (ui->chkTvShows->isEnabled() && ui->chkTvShows->isChecked()) {
        sections << ExportTemplate::ExportSection::TvShows;
    }

    if (sections.isEmpty()) {
        ui->message->setErrorMessage(tr("You need to select at least one entry to export."));
        return;
    }

    ExportTemplate *exportTemplate =
        ExportTemplateLoader::instance()->getTemplateByIdentifier(ui->comboTheme->itemData(index).toString());
    if (!exportTemplate) {
        return;
    }

    ui->progressBar->setValue(0);
    QString location = QFileDialog::getExistingDirectory(this, tr("Export directory"), QDir::homePath());
    if (location.isEmpty()) {
        return;
    }

    QDir dir(location);
    QString subDir =
        QStringLiteral("MediaElch Export %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm"));
    if (!dir.mkdir(subDir)) {
        ui->message->setErrorMessage(tr("Could not create export directory."));
        return;
    }
    dir.setCurrent(location + "/" + subDir);

    ui->btnExport->setEnabled(false);

    int itemsToExport = 0;
    if (sections.contains(ExportTemplate::ExportSection::Concerts)) {
        itemsToExport += Manager::instance()->concertModel()->concerts().count();
    }
    if (sections.contains(ExportTemplate::ExportSection::Movies)) {
        itemsToExport += Manager::instance()->movieModel()->movies().count();
    }
    if (sections.contains(ExportTemplate::ExportSection::TvShows)) {
        for (TvShow *show : Manager::instance()->tvShowModel()->tvShows()) {
            ++itemsToExport;
            itemsToExport += show->episodeCount();
        }
    }

    ui->progressBar->setRange(0, itemsToExport);

    // Create the base structure
    exportTemplate->copyTo(dir.currentPath());

    // Export movies
    if (sections.contains(ExportTemplate::ExportSection::Movies)) {
        if (m_canceled) {
            return;
        }
        parseAndSaveMovies(dir.currentPath(), exportTemplate, Manager::instance()->movieModel()->movies());
    }

    // Export TV Shows
    if (sections.contains(ExportTemplate::ExportSection::TvShows)) {
        if (m_canceled) {
            return;
        }
        parseAndSaveTvShows(dir.currentPath(), exportTemplate, Manager::instance()->tvShowModel()->tvShows());
    }

    // Export Concerts
    if (sections.contains(ExportTemplate::ExportSection::Concerts)) {
        if (m_canceled) {
            return;
        }
        parseAndSaveConcerts(dir.currentPath(), exportTemplate, Manager::instance()->concertModel()->concerts());
    }

    ui->progressBar->setValue(ui->progressBar->maximum());
    ui->message->setSuccessMessage(tr("Export completed."));
    ui->btnExport->setEnabled(true);
}

void ExportDialog::onThemeChanged()
{
    int index = ui->comboTheme->currentIndex();
    if (index < 0 || index >= ui->comboTheme->count()) {
        return;
    }

    ExportTemplate *exportTemplate =
        ExportTemplateLoader::instance()->getTemplateByIdentifier(ui->comboTheme->itemData(index).toString());
    if (!exportTemplate) {
        return;
    }

    ui->chkConcerts->setEnabled(exportTemplate->exportSections().contains(ExportTemplate::ExportSection::Concerts));
    ui->chkMovies->setEnabled(exportTemplate->exportSections().contains(ExportTemplate::ExportSection::Movies));
    ui->chkTvShows->setEnabled(exportTemplate->exportSections().contains(ExportTemplate::ExportSection::TvShows));
}

void ExportDialog::parseAndSaveMovies(QDir dir, ExportTemplate *exportTemplate, QVector<Movie *> movies)
{
    qSort(movies.begin(), movies.end(), Movie::lessThan);
    QString listContent = exportTemplate->getTemplate(ExportTemplate::ExportSection::Movies);
    QString itemContent = exportTemplate->getTemplate(ExportTemplate::ExportSection::Movie);

    QString listMovieItem;
    QString listMovieBlock;
    QStringList movieList;
    QRegExp rx(R"(\{\{ BEGIN_BLOCK_MOVIE \}\}(.*)\{\{ END_BLOCK_MOVIE \}\})");
    rx.setMinimal(true);

    int pos = 0;
    while ((pos = rx.indexIn(listContent, pos)) != -1) {
        pos += rx.matchedLength();

        listMovieBlock = rx.cap(0);
        listMovieItem = rx.cap(1).trimmed();
    }

    dir.mkdir("movies");
    dir.mkdir("movie_images");

    for (Movie *movie : movies) {
        if (m_canceled) {
            return;
        }

        QString movieTemplate = itemContent;
        replaceVars(movieTemplate, movie, dir, true);
        QFile file(dir.currentPath() + QString("/movies/%1.html").arg(movie->movieId()));
        if (file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(movieTemplate.toUtf8());
            file.close();
        }

        QString m = listMovieItem;
        replaceVars(m, movie, dir);
        movieList << m;
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    listContent.replace(listMovieBlock, movieList.join("\n"));

    QFile file(dir.currentPath() + "/movies.html");
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        file.write(listContent.toUtf8());
        file.close();
    }
}

void ExportDialog::replaceVars(QString &m, Movie *movie, QDir dir, bool subDir)
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
    m.replace("{{ MOVIE.SET }}", movie->set().toHtmlEscaped());
    m.replace("{{ MOVIE.TAGLINE }}", movie->tagline().toHtmlEscaped());
    m.replace("{{ MOVIE.GENRES }}", movie->genres().join(", ").toHtmlEscaped());
    m.replace("{{ MOVIE.COUNTRIES }}", movie->countries().join(", ").toHtmlEscaped());
    m.replace("{{ MOVIE.STUDIOS }}", movie->studios().join(", ").toHtmlEscaped());
    m.replace("{{ MOVIE.TAGS }}", movie->tags().join(", ").toHtmlEscaped());
    m.replace("{{ MOVIE.WRITER }}", movie->writer().toHtmlEscaped());
    m.replace("{{ MOVIE.DIRECTOR }}", movie->director().toHtmlEscaped());
    m.replace("{{ MOVIE.CERTIFICATION }}", movie->certification().toString().toHtmlEscaped());
    m.replace("{{ MOVIE.TRAILER }}", movie->trailer().toString());
    m.replace("{{ MOVIE.RATING }}", QString::number(movie->rating(), 'f', 1));
    m.replace("{{ MOVIE.VOTES }}", QString::number(movie->votes(), 'f', 0));
    m.replace("{{ MOVIE.RUNTIME }}", QString::number(movie->runtime().count(), 'f', 0));
    m.replace("{{ MOVIE.PLAY_COUNT }}", QString::number(movie->playcount(), 'f', 0));
    m.replace("{{ MOVIE.LAST_PLAYED }}",
        movie->lastPlayed().isValid() ? movie->lastPlayed().toString("yyyy-MM-dd hh:mm") : "");
    m.replace(
        "{{ MOVIE.DATE_ADDED }}", movie->dateAdded().isValid() ? movie->dateAdded().toString("yyyy-MM-dd hh:mm") : "");
    m.replace("{{ MOVIE.FILE_LAST_MODIFIED }}",
        movie->fileLastModified().isValid() ? movie->fileLastModified().toString("yyyy-MM-dd hh:mm") : "");
    m.replace("{{ MOVIE.FILENAME }}", (!movie->files().isEmpty()) ? movie->files().first() : "");
    if (!movie->files().isEmpty()) {
        QFileInfo fi(movie->files().first());
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
    for (const Actor &actor : movie->actors()) {
        actorNames << actor.name;
        actorRoles << actor.role;
    }
    replaceMultiBlock(m, "ACTORS", {"ACTOR.NAME", "ACTOR.ROLE"}, QVector<QStringList>() << actorNames << actorRoles);

    replaceStreamDetailsVars(m, movie->streamDetails());
    replaceImages(m, dir, subDir, movie);
}

void ExportDialog::parseAndSaveConcerts(QDir dir, ExportTemplate *exportTemplate, QVector<Concert *> concerts)
{
    qSort(concerts.begin(), concerts.end(), Concert::lessThan);
    QString listContent = exportTemplate->getTemplate(ExportTemplate::ExportSection::Concerts);
    QString itemContent = exportTemplate->getTemplate(ExportTemplate::ExportSection::Concert);

    QString listConcertItem;
    QString listConcertBlock;
    QStringList concertList;
    QRegExp rx(R"(\{\{ BEGIN_BLOCK_CONCERT \}\}(.*)\{\{ END_BLOCK_CONCERT \}\})");
    rx.setMinimal(true);

    int pos = 0;
    while ((pos = rx.indexIn(listContent, pos)) != -1) {
        pos += rx.matchedLength();

        listConcertBlock = rx.cap(0);
        listConcertItem = rx.cap(1).trimmed();
    }

    dir.mkdir("concerts");
    dir.mkdir("concert_images");

    for (const Concert *concert : concerts) {
        if (m_canceled) {
            return;
        }

        QString concertTemplate = itemContent;
        replaceVars(concertTemplate, concert, dir, true);
        QFile file(dir.currentPath() + QString("/concerts/%1.html").arg(concert->concertId()));
        if (file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(concertTemplate.toUtf8());
            file.close();
        }

        QString c = listConcertItem;
        replaceVars(c, concert, dir);
        concertList << c;
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    listContent.replace(listConcertBlock, concertList.join("\n"));

    QFile file(dir.currentPath() + "/concerts.html");
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        file.write(listContent.toUtf8());
        file.close();
    }
}

void ExportDialog::replaceVars(QString &m, const Concert *concert, QDir dir, bool subDir)
{
    m.replace("{{ CONCERT.ID }}", QString::number(concert->concertId(), 'f', 0));
    m.replace("{{ CONCERT.LINK }}", QString("concerts/%1.html").arg(concert->concertId()));
    m.replace("{{ CONCERT.TITLE }}", concert->name().toHtmlEscaped());
    m.replace("{{ CONCERT.ARTIST }}", concert->artist().toHtmlEscaped());
    m.replace("{{ CONCERT.ALBUM }}", concert->album().toHtmlEscaped());
    m.replace("{{ CONCERT.TAGLINE }}", concert->tagline().toHtmlEscaped());
    m.replace("{{ CONCERT.RATING }}", QString::number(concert->rating(), 'f', 1));
    m.replace("{{ CONCERT.YEAR }}", concert->released().isValid() ? concert->released().toString("yyyy") : "");
    m.replace("{{ CONCERT.RUNTIME }}", QString::number(concert->runtime().count(), 'f', 0));
    m.replace("{{ CONCERT.CERTIFICATION }}", concert->certification().toString().toHtmlEscaped());
    m.replace("{{ CONCERT.TRAILER }}", concert->trailer().toString());
    m.replace("{{ CONCERT.PLAY_COUNT }}", QString::number(concert->playcount(), 'f', 0));
    m.replace("{{ CONCERT.LAST_PLAYED }}",
        concert->lastPlayed().isValid() ? concert->lastPlayed().toString("yyyy-MM-dd hh:mm") : "");
    m.replace("{{ CONCERT.PLOT }}", concert->overview().toHtmlEscaped().replace("\n", "<br />"));
    m.replace("{{ CONCERT.TAGS }}", concert->tags().join(", ").toHtmlEscaped());
    m.replace("{{ CONCERT.GENRES }}", concert->genres().join(", ").toHtmlEscaped());

    replaceStreamDetailsVars(m, concert->streamDetails());
    replaceSingleBlock(m, "TAGS", "TAG.NAME", concert->tags());
    replaceSingleBlock(m, "GENRES", "GENRE.NAME", concert->genres());
    replaceImages(m, dir, subDir, nullptr, concert);
}

void ExportDialog::parseAndSaveTvShows(QDir dir, ExportTemplate *exportTemplate, QVector<TvShow *> shows)
{
    qSort(shows.begin(), shows.end(), TvShow::lessThan);
    QString listContent = exportTemplate->getTemplate(ExportTemplate::ExportSection::TvShows);
    QString itemContent = exportTemplate->getTemplate(ExportTemplate::ExportSection::TvShow);
    QString episodeContent = exportTemplate->getTemplate(ExportTemplate::ExportSection::Episode);

    QString listTvShowItem;
    QString listTvShowBlock;
    QStringList tvShowList;
    QRegExp rx(R"(\{\{ BEGIN_BLOCK_TVSHOW \}\}(.*)\{\{ END_BLOCK_TVSHOW \}\})");
    rx.setMinimal(true);

    int pos = 0;
    while ((pos = rx.indexIn(listContent, pos)) != -1) {
        pos += rx.matchedLength();

        listTvShowBlock = rx.cap(0);
        listTvShowItem = rx.cap(1).trimmed();
    }

    dir.mkdir("tvshows");
    dir.mkdir("tvshow_images");
    dir.mkdir("episodes");
    dir.mkdir("episode_images");

    for (TvShow *show : shows) {
        if (m_canceled) {
            return;
        }

        QString showTemplate = itemContent;
        replaceVars(showTemplate, show, dir, true);
        QFile file(dir.currentPath() + QString("/tvshows/%1.html").arg(show->showId()));
        if (file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(showTemplate.toUtf8());
            file.close();
        }

        QString s = listTvShowItem;
        replaceVars(s, show, dir);
        tvShowList << s;
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

        for (TvShowEpisode *episode : show->episodes()) {
            if (episode->isDummy()) {
                continue;
            }
            QString episodeTemplate = episodeContent;
            replaceVars(episodeTemplate, episode, dir, true);
            QFile file(dir.currentPath() + QString("/episodes/%1.html").arg(episode->episodeId()));
            if (file.open(QFile::WriteOnly | QFile::Text)) {
                file.write(episodeTemplate.toUtf8());
                file.close();
            }
            ui->progressBar->setValue(ui->progressBar->value() + 1);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
    }

    listContent.replace(listTvShowBlock, tvShowList.join("\n"));

    QFile file(dir.currentPath() + "/tvshows.html");
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        file.write(listContent.toUtf8());
        file.close();
    }
}

void ExportDialog::replaceVars(QString &m, const TvShow *show, QDir dir, bool subDir)
{
    m.replace("{{ TVSHOW.ID }}", QString::number(show->showId(), 'f', 0));
    m.replace("{{ TVSHOW.LINK }}", QString("tvshows/%1.html").arg(show->showId()));
    m.replace("{{ TVSHOW.IMDB_ID }}", show->imdbId());
    m.replace("{{ TVSHOW.TITLE }}", show->name().toHtmlEscaped());
    m.replace("{{ TVSHOW.RATING }}", QString::number(show->rating(), 'f', 1));
    m.replace("{{ TVSHOW.CERTIFICATION }}", show->certification().toString().toHtmlEscaped());
    m.replace(
        "{{ TVSHOW.FIRST_AIRED }}", show->firstAired().isValid() ? show->firstAired().toString("yyyy-MM-dd") : "");
    m.replace("{{ TVSHOW.STUDIO }}", show->network().toHtmlEscaped());
    m.replace("{{ TVSHOW.PLOT }}", show->overview().toHtmlEscaped().replace("\n", "<br />"));
    m.replace("{{ TVSHOW.TAGS }}", show->tags().join(", ").toHtmlEscaped());
    m.replace("{{ TVSHOW.GENRES }}", show->genres().join(", ").toHtmlEscaped());

    QStringList actorNames;
    QStringList actorRoles;
    for (Actor actor : show->actors()) {
        actorNames << actor.name;
        actorRoles << actor.role;
    }
    replaceMultiBlock(m,
        "ACTORS",
        QStringList() << "ACTOR.NAME"
                      << "ACTOR.ROLE",
        QVector<QStringList>() << actorNames << actorRoles);
    replaceSingleBlock(m, "TAGS", "TAG.NAME", show->tags());
    replaceSingleBlock(m, "GENRES", "GENRE.NAME", show->genres());
    replaceImages(m, dir, subDir, nullptr, nullptr, show);

    QString listSeasonItem;
    QString listSeasonBlock;
    QStringList seasonList;
    QRegExp rx;
    rx.setMinimal(true);
    rx.setPattern(R"(\{\{ BEGIN_BLOCK_SEASON \}\}(.*)\{\{ END_BLOCK_SEASON \}\})");

    int pos = 0;
    while ((pos = rx.indexIn(m, pos)) != -1) {
        pos += rx.matchedLength();

        listSeasonBlock = rx.cap(0);
        listSeasonItem = rx.cap(1).trimmed();
    }

    if (listSeasonBlock.isEmpty() || listSeasonItem.isEmpty()) {
        return;
    }

    QVector<SeasonNumber> seasons = show->seasons(false);
    qSort(seasons);
    for (const SeasonNumber &season : seasons) {
        QVector<TvShowEpisode *> episodes = show->episodes(season);
        qSort(episodes.begin(), episodes.end(), TvShowEpisode::lessThan);
        QString s = listSeasonItem;
        s.replace("{{ SEASON }}", season.toString());

        QString listEpisodeItem;
        QString listEpisodeBlock;
        QStringList episodeList;
        rx.setPattern(R"(\{\{ BEGIN_BLOCK_EPISODE \}\}(.*)\{\{ END_BLOCK_EPISODE \}\})");

        int pos = 0;
        while ((pos = rx.indexIn(s, pos)) != -1) {
            pos += rx.matchedLength();

            listEpisodeBlock = rx.cap(0);
            listEpisodeItem = rx.cap(1).trimmed();
        }

        for (TvShowEpisode *episode : episodes) {
            QString e = listEpisodeItem;
            replaceVars(e, episode, dir, true);
            episodeList << e;
        }
        s.replace(listEpisodeBlock, episodeList.join("\n"));
        seasonList << s;
    }

    m.replace(listSeasonBlock, seasonList.join("\n"));
}

void ExportDialog::replaceVars(QString &m, TvShowEpisode *episode, QDir dir, bool subDir)
{
    m.replace("{{ SHOW.TITLE }}", episode->tvShow()->name().toHtmlEscaped());
    m.replace("{{ SHOW.LINK }}", QString("../tvshows/%1.html").arg(episode->tvShow()->showId()));
    m.replace("{{ EPISODE.LINK }}", QString("../episodes/%1.html").arg(episode->episodeId()));
    m.replace("{{ EPISODE.TITLE }}", episode->name().toHtmlEscaped());
    m.replace("{{ EPISODE.SEASON }}", episode->seasonString().toHtmlEscaped());
    m.replace("{{ EPISODE.EPISODE }}", episode->episodeString().toHtmlEscaped());
    m.replace("{{ EPISODE.RATING }}", QString::number(episode->rating(), 'f', 1));
    m.replace("{{ EPISODE.CERTIFICATION }}", episode->certification().toString().toHtmlEscaped());
    m.replace("{{ EPISODE.FIRST_AIRED }}",
        episode->firstAired().isValid() ? episode->firstAired().toString("yyyy-MM-dd") : "");
    m.replace("{{ EPISODE.LAST_PLAYED }}",
        episode->lastPlayed().isValid() ? episode->lastPlayed().toString("yyyy-MM-dd hh:mm") : "");
    m.replace("{{ EPISODE.STUDIO }}", episode->network().toHtmlEscaped());
    m.replace("{{ EPISODE.PLOT }}", episode->overview().toHtmlEscaped().replace("\n", "<br />"));
    m.replace("{{ EPISODE.WRITERS }}", episode->writers().join(", ").toHtmlEscaped());
    m.replace("{{ EPISODE.DIRECTORS }}", episode->directors().join(", ").toHtmlEscaped());

    replaceStreamDetailsVars(m, episode->streamDetails());
    replaceSingleBlock(m, "WRITERS", "WRITER.NAME", episode->writers());
    replaceSingleBlock(m, "DIRECTORS", "DIRECTOR.NAME", episode->directors());
    replaceImages(m, dir, subDir, nullptr, nullptr, nullptr, episode);
}

void ExportDialog::replaceStreamDetailsVars(QString &m, const StreamDetails *streamDetails)
{
    const auto videoDetails = streamDetails->videoDetails();
    const auto audioDetails = streamDetails->audioDetails();

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
}

void ExportDialog::replaceSingleBlock(QString &m, QString blockName, QString itemName, QStringList replaces)
{
    replaceMultiBlock(m, blockName, QStringList() << itemName, QVector<QStringList>() << replaces);
}

void ExportDialog::replaceMultiBlock(QString &m,
    QString blockName,
    QStringList itemNames,
    QVector<QStringList> replaces)
{
    QRegExp rx;
    rx.setMinimal(true);
    rx.setPattern("\\{\\{ BEGIN_BLOCK_" + blockName + R"( \}\}(.*)\{\{ END_BLOCK_)" + blockName + " \\}\\}");
    if (rx.indexIn(m) != -1) {
        QString block = rx.cap(0);
        QString item = rx.cap(1).trimmed();
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

void ExportDialog::saveImage(QSize size, QString imageFile, QString destinationFile, const char *format, int quality)
{
    QImage img(imageFile);
    img = img.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    img.save(destinationFile, format, quality);
}

void ExportDialog::onBtnClose()
{
    m_canceled = true;
    QDialog::reject();
}

void ExportDialog::replaceImages(QString &m,
    const QDir &dir,
    const bool &subDir,
    const Movie *movie,
    const Concert *concert,
    const TvShow *tvShow,
    const TvShowEpisode *episode)
{
    QString item;
    QSize size;
    QRegExp rx(R"(\{\{ IMAGE.(.*)\[(\d*),(\d*)\] \}\})");
    rx.setMinimal(true);
    int pos = 0;
    while (rx.indexIn(m, pos) != -1) {
        item = rx.cap(0);
        QString type = rx.cap(1).toLower();
        size.setWidth(rx.cap(2).toInt());
        size.setHeight(rx.cap(3).toInt());

        if (!item.isEmpty() && !size.isEmpty()) {
            QString destFile;
            bool imageSaved = false;
            QString typeName;
            if (movie) {
                imageSaved = saveImageForType(type, size, dir, destFile, movie);
                typeName = "movie";
            } else if (concert) {
                imageSaved = saveImageForType(type, size, dir, destFile, concert);
                typeName = "concert";
            } else if (tvShow) {
                imageSaved = saveImageForType(type, size, dir, destFile, tvShow);
                typeName = "tvshow";
            } else if (episode) {
                imageSaved = saveImageForType(type, size, dir, destFile, episode);
                typeName = "episode";
            }

            if (imageSaved) {
                m.replace(item, (subDir ? "../" : "") + destFile);
            } else {
                m.replace(item,
                    (subDir ? "../" : "")
                        + QString("defaults/%1_%2_%3x%4.png")
                              .arg(typeName)
                              .arg(type)
                              .arg(size.width())
                              .arg(size.height()));
            }
        }
        pos += rx.matchedLength();
    }
}

bool ExportDialog::saveImageForType(const QString &type,
    const QSize &size,
    const QDir &dir,
    QString &destFile,
    const Movie *movie)
{
    destFile = "movie_images/"
               + QString("%1-%2_%3x%4.jpg").arg(movie->movieId()).arg(type).arg(size.width()).arg(size.height());

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
        return false;
    }

    QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(movie, imageType);
    if (filename.isEmpty()) {
        return false;
    }

    int imageQuality = (imageFormat == "jpg") ? 90 : -1;
    saveImage(size, filename, dir.currentPath() + "/" + destFile, imageFormat.c_str(), imageQuality);

    return true;
}

bool ExportDialog::saveImageForType(const QString &type,
    const QSize &size,
    const QDir &dir,
    QString &destFile,
    const Concert *concert)
{
    destFile =
        "concert_images/"
        + QStringLiteral("%1-%2_%3x%4.jpg").arg(concert->concertId()).arg(type).arg(size.width()).arg(size.height());

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
        return false;
    }

    QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(concert, imageType);
    if (filename.isEmpty()) {
        return false;
    }

    int imageQuality = (imageFormat == "jpg") ? 90 : -1;
    saveImage(size, filename, dir.currentPath() + "/" + destFile, imageFormat.c_str(), imageQuality);

    return true;
}

bool ExportDialog::saveImageForType(const QString &type,
    const QSize &size,
    const QDir &dir,
    QString &destFile,
    const TvShow *tvShow)
{
    destFile = "tvshow_images/"
               + QString("%1-%2_%3x%4.jpg").arg(tvShow->showId()).arg(type).arg(size.width()).arg(size.height());

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
        return false;
    }

    QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(tvShow, imageType);
    if (filename.isEmpty()) {
        return false;
    }

    int imageQuality = (imageFormat == "jpg") ? 90 : -1;
    saveImage(size, filename, dir.currentPath() + "/" + destFile, imageFormat.c_str(), imageQuality);

    return true;
}

bool ExportDialog::saveImageForType(const QString &type,
    const QSize &size,
    const QDir &dir,
    QString &destFile,
    const TvShowEpisode *episode)
{
    destFile = "episode_images/"
               + QString("%1-%2_%3x%4.jpg").arg(episode->episodeId()).arg(type).arg(size.width()).arg(size.height());

    if (type == "thumbnail") {
        QString filename =
            Manager::instance()->mediaCenterInterface()->imageFileName(episode, ImageType::TvShowEpisodeThumb);
        if (filename.isEmpty()) {
            return false;
        }
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "jpg", 90);
    } else {
        return false;
    }

    return true;
}
