#include "ExportDialog.h"
#include "ui_ExportDialog.h"

#include <QFileDialog>

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
    QList<ExportTemplate *> templates = ExportTemplateLoader::instance()->installedTemplates();
    if (templates.isEmpty()) {
        ui->message->setErrorMessage(tr("You need to install at least one theme."));
        ui->btnExport->setEnabled(false);
    } else {
        ui->message->clear();
        ui->btnExport->setEnabled(true);
    }

    ui->progressBar->setValue(0);
    ui->comboTheme->clear();
    foreach (ExportTemplate *exportTemplate, templates)
        ui->comboTheme->addItem(exportTemplate->name(), exportTemplate->identifier());
    onThemeChanged();
    adjustSize();
    return QDialog::exec();
}

void ExportDialog::onBtnExport()
{
    ui->message->clear();

    int index = ui->comboTheme->currentIndex();
    if (index < 0 || index >= ui->comboTheme->count())
        return;

    QList<ExportTemplate::ExportSection> sections;
    if (ui->chkConcerts->isEnabled() && ui->chkConcerts->isChecked())
        sections << ExportTemplate::SectionConcerts;
    if (ui->chkMovies->isEnabled() && ui->chkMovies->isChecked())
        sections << ExportTemplate::SectionMovies;
    if (ui->chkTvShows->isEnabled() && ui->chkTvShows->isChecked())
        sections << ExportTemplate::SectionTvShows;

    if (sections.isEmpty()) {
        ui->message->setErrorMessage(tr("You need to select at least one entry to export."));
        return;
    }

    ExportTemplate *exportTemplate =
        ExportTemplateLoader::instance()->getTemplateByIdentifier(ui->comboTheme->itemData(index).toString());
    if (!exportTemplate)
        return;

    ui->progressBar->setValue(0);
    QString location = QFileDialog::getExistingDirectory(this, tr("Export directory"), QDir::homePath());
    if (location.isEmpty())
        return;


    QDir dir(location);
    QString subDir = QString("MediaElch Export %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm"));
    if (!dir.mkdir(subDir)) {
        ui->message->setErrorMessage(tr("Could not create export directory."));
        return;
    }
    dir.setCurrent(location + "/" + subDir);

    ui->btnExport->setEnabled(false);

    int itemsToExport = 0;
    if (sections.contains(ExportTemplate::SectionConcerts))
        itemsToExport += Manager::instance()->concertModel()->concerts().count();
    if (sections.contains(ExportTemplate::SectionMovies))
        itemsToExport += Manager::instance()->movieModel()->movies().count();
    if (sections.contains(ExportTemplate::SectionTvShows)) {
        foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows()) {
            itemsToExport++;
            itemsToExport += show->episodeCount();
        }
    }

    ui->progressBar->setRange(0, itemsToExport);

    // Create the base structure
    exportTemplate->copyTo(dir.currentPath());

    // Export movies
    if (sections.contains(ExportTemplate::SectionMovies)) {
        if (m_canceled)
            return;
        parseAndSaveMovies(dir.currentPath(), exportTemplate, Manager::instance()->movieModel()->movies());
    }

    // Export TV Shows
    if (sections.contains(ExportTemplate::SectionTvShows)) {
        if (m_canceled)
            return;
        parseAndSaveTvShows(dir.currentPath(), exportTemplate, Manager::instance()->tvShowModel()->tvShows());
    }

    // Export Concerts
    if (sections.contains(ExportTemplate::SectionConcerts)) {
        if (m_canceled)
            return;
        parseAndSaveConcerts(dir.currentPath(), exportTemplate, Manager::instance()->concertModel()->concerts());
    }

    ui->progressBar->setValue(ui->progressBar->maximum());
    ui->message->setSuccessMessage(tr("Export completed."));
    ui->btnExport->setEnabled(true);
}

void ExportDialog::onThemeChanged()
{
    int index = ui->comboTheme->currentIndex();
    if (index < 0 || index >= ui->comboTheme->count())
        return;

    ExportTemplate *exportTemplate =
        ExportTemplateLoader::instance()->getTemplateByIdentifier(ui->comboTheme->itemData(index).toString());
    if (!exportTemplate)
        return;

    ui->chkConcerts->setEnabled(exportTemplate->exportSections().contains(ExportTemplate::SectionConcerts));
    ui->chkMovies->setEnabled(exportTemplate->exportSections().contains(ExportTemplate::SectionMovies));
    ui->chkTvShows->setEnabled(exportTemplate->exportSections().contains(ExportTemplate::SectionTvShows));
}

void ExportDialog::parseAndSaveMovies(QDir dir, ExportTemplate *exportTemplate, QList<Movie *> movies)
{
    qSort(movies.begin(), movies.end(), Movie::lessThan);
    QString listContent = exportTemplate->getTemplate(ExportTemplate::SectionMovies);
    QString itemContent = exportTemplate->getTemplate(ExportTemplate::SectionMovie);

    QString listMovieItem;
    QString listMovieBlock;
    QStringList movieList;
    QRegExp rx("\\{\\{ BEGIN_BLOCK_MOVIE \\}\\}(.*)\\{\\{ END_BLOCK_MOVIE \\}\\}");
    rx.setMinimal(true);

    int pos = 0;
    while ((pos = rx.indexIn(listContent, pos)) != -1) {
        pos += rx.matchedLength();

        listMovieBlock = rx.cap(0);
        listMovieItem = rx.cap(1).trimmed();
    }

    dir.mkdir("movies");
    dir.mkdir("movie_images");

    foreach (Movie *movie, movies) {
        if (m_canceled)
            return;

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
    m.replace("{{ MOVIE.IMDB_ID }}", movie->id());
    m.replace("{{ MOVIE.TMDB_ID }}", movie->tmdbId());
    m.replace("{{ MOVIE.TITLE }}", movie->name());
    m.replace("{{ MOVIE.YEAR }}", movie->released().isValid() ? movie->released().toString("yyyy") : "");
    m.replace("{{ MOVIE.ORIGINAL_TITLE }}", movie->originalName());
    m.replace("{{ MOVIE.PLOT }}", movie->overview().replace("\n", "<br />"));
    m.replace("{{ MOVIE.PLOT_SIMPLE }}", movie->outline().replace("\n", "<br />"));
    m.replace("{{ MOVIE.SET }}", movie->set());
    m.replace("{{ MOVIE.TAGLINE }}", movie->tagline());
    m.replace("{{ MOVIE.GENRES }}", movie->genres().join(", "));
    m.replace("{{ MOVIE.COUNTRIES }}", movie->countries().join(", "));
    m.replace("{{ MOVIE.STUDIOS }}", movie->studios().join(", "));
    m.replace("{{ MOVIE.TAGS }}", movie->tags().join(", "));
    m.replace("{{ MOVIE.WRITER }}", movie->writer());
    m.replace("{{ MOVIE.DIRECTOR }}", movie->director());
    m.replace("{{ MOVIE.CERTIFICATION }}", movie->certification());
    m.replace("{{ MOVIE.TRAILER }}", movie->trailer().toString());
    m.replace("{{ MOVIE.RATING }}", QString::number(movie->rating(), 'f', 1));
    m.replace("{{ MOVIE.VOTES }}", QString::number(movie->votes(), 'f', 0));
    m.replace("{{ MOVIE.RUNTIME }}", QString::number(movie->runtime(), 'f', 0));
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
    replaceSingleBlock(m, "STUDIOS", "STUDIO.NAME", movie->countries());

    QStringList actorNames;
    QStringList actorRoles;
    foreach (Actor actor, movie->actors()) {
        actorNames << actor.name;
        actorRoles << actor.role;
    }
    replaceMultiBlock(m,
        "ACTORS",
        QStringList() << "ACTOR.NAME"
                      << "ACTOR.ROLE",
        QList<QStringList>() << actorNames << actorRoles);

    replaceStreamDetailsVars(m, movie->streamDetails());
    replaceImages(m, dir, subDir, movie);
}

void ExportDialog::parseAndSaveConcerts(QDir dir, ExportTemplate *exportTemplate, QList<Concert *> concerts)
{
    qSort(concerts.begin(), concerts.end(), Concert::lessThan);
    QString listContent = exportTemplate->getTemplate(ExportTemplate::SectionConcerts);
    QString itemContent = exportTemplate->getTemplate(ExportTemplate::SectionConcert);

    QString listConcertItem;
    QString listConcertBlock;
    QStringList concertList;
    QRegExp rx("\\{\\{ BEGIN_BLOCK_CONCERT \\}\\}(.*)\\{\\{ END_BLOCK_CONCERT \\}\\}");
    rx.setMinimal(true);

    int pos = 0;
    while ((pos = rx.indexIn(listContent, pos)) != -1) {
        pos += rx.matchedLength();

        listConcertBlock = rx.cap(0);
        listConcertItem = rx.cap(1).trimmed();
    }

    dir.mkdir("concerts");
    dir.mkdir("concert_images");

    foreach (Concert *concert, concerts) {
        if (m_canceled)
            return;

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

void ExportDialog::replaceVars(QString &m, Concert *concert, QDir dir, bool subDir)
{
    m.replace("{{ CONCERT.ID }}", QString::number(concert->concertId(), 'f', 0));
    m.replace("{{ CONCERT.LINK }}", QString("concerts/%1.html").arg(concert->concertId()));
    m.replace("{{ CONCERT.TITLE }}", concert->name());
    m.replace("{{ CONCERT.ARTIST }}", concert->artist());
    m.replace("{{ CONCERT.ALBUM }}", concert->album());
    m.replace("{{ CONCERT.TAGLINE }}", concert->tagline());
    m.replace("{{ CONCERT.RATING }}", QString::number(concert->rating(), 'f', 1));
    m.replace("{{ CONCERT.YEAR }}", concert->released().isValid() ? concert->released().toString("yyyy") : "");
    m.replace("{{ CONCERT.RUNTIME }}", QString::number(concert->runtime(), 'f', 0));
    m.replace("{{ CONCERT.CERTIFICATION }}", concert->certification());
    m.replace("{{ CONCERT.TRAILER }}", concert->trailer().toString());
    m.replace("{{ CONCERT.PLAY_COUNT }}", QString::number(concert->playcount(), 'f', 0));
    m.replace("{{ CONCERT.LAST_PLAYED }}",
        concert->lastPlayed().isValid() ? concert->lastPlayed().toString("yyyy-MM-dd hh:mm") : "");
    m.replace("{{ CONCERT.PLOT }}", concert->overview().replace("\n", "<br />"));
    m.replace("{{ CONCERT.TAGS }}", concert->tags().join(", "));
    m.replace("{{ CONCERT.GENRES }}", concert->genres().join(", "));

    replaceStreamDetailsVars(m, concert->streamDetails());
    replaceSingleBlock(m, "TAGS", "TAG.NAME", concert->tags());
    replaceSingleBlock(m, "GENRES", "GENRE.NAME", concert->genres());
    replaceImages(m, dir, subDir, nullptr, concert);
}

void ExportDialog::parseAndSaveTvShows(QDir dir, ExportTemplate *exportTemplate, QList<TvShow *> shows)
{
    qSort(shows.begin(), shows.end(), TvShow::lessThan);
    QString listContent = exportTemplate->getTemplate(ExportTemplate::SectionTvShows);
    QString itemContent = exportTemplate->getTemplate(ExportTemplate::SectionTvShow);
    QString episodeContent = exportTemplate->getTemplate(ExportTemplate::SectionEpisode);

    QString listTvShowItem;
    QString listTvShowBlock;
    QStringList tvShowList;
    QRegExp rx("\\{\\{ BEGIN_BLOCK_TVSHOW \\}\\}(.*)\\{\\{ END_BLOCK_TVSHOW \\}\\}");
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

    foreach (TvShow *show, shows) {
        if (m_canceled)
            return;

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

        foreach (TvShowEpisode *episode, show->episodes()) {
            if (episode->isDummy())
                continue;
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

void ExportDialog::replaceVars(QString &m, TvShow *show, QDir dir, bool subDir)
{
    m.replace("{{ TVSHOW.ID }}", QString::number(show->showId(), 'f', 0));
    m.replace("{{ TVSHOW.LINK }}", QString("tvshows/%1.html").arg(show->showId()));
    m.replace("{{ TVSHOW.IMDB_ID }}", show->imdbId());
    m.replace("{{ TVSHOW.TITLE }}", show->name());
    m.replace("{{ TVSHOW.RATING }}", QString::number(show->rating(), 'f', 1));
    m.replace("{{ TVSHOW.CERTIFICATION }}", show->certification());
    m.replace(
        "{{ TVSHOW.FIRST_AIRED }}", show->firstAired().isValid() ? show->firstAired().toString("yyyy-MM-dd") : "");
    m.replace("{{ TVSHOW.STUDIO }}", show->network());
    m.replace("{{ TVSHOW.PLOT }}", show->overview().replace("\n", "<br />"));
    m.replace("{{ TVSHOW.TAGS }}", show->tags().join(", "));
    m.replace("{{ TVSHOW.GENRES }}", show->genres().join(", "));

    QStringList actorNames;
    QStringList actorRoles;
    foreach (Actor actor, show->actors()) {
        actorNames << actor.name;
        actorRoles << actor.role;
    }
    replaceMultiBlock(m,
        "ACTORS",
        QStringList() << "ACTOR.NAME"
                      << "ACTOR.ROLE",
        QList<QStringList>() << actorNames << actorRoles);
    replaceSingleBlock(m, "TAGS", "TAG.NAME", show->tags());
    replaceSingleBlock(m, "GENRES", "GENRE.NAME", show->genres());
    replaceImages(m, dir, subDir, nullptr, nullptr, show);

    QString listSeasonItem;
    QString listSeasonBlock;
    QStringList seasonList;
    QRegExp rx;
    rx.setMinimal(true);
    rx.setPattern("\\{\\{ BEGIN_BLOCK_SEASON \\}\\}(.*)\\{\\{ END_BLOCK_SEASON \\}\\}");

    int pos = 0;
    while ((pos = rx.indexIn(m, pos)) != -1) {
        pos += rx.matchedLength();

        listSeasonBlock = rx.cap(0);
        listSeasonItem = rx.cap(1).trimmed();
    }

    if (listSeasonBlock.isEmpty() || listSeasonItem.isEmpty())
        return;

    QList<int> seasons = show->seasons(false);
    qSort(seasons);
    foreach (const int &season, seasons) {
        QList<TvShowEpisode *> episodes = show->episodes(season);
        qSort(episodes.begin(), episodes.end(), TvShowEpisode::lessThan);
        QString s = listSeasonItem;
        s.replace("{{ SEASON }}", QString::number(season));

        QString listEpisodeItem;
        QString listEpisodeBlock;
        QStringList episodeList;
        rx.setPattern("\\{\\{ BEGIN_BLOCK_EPISODE \\}\\}(.*)\\{\\{ END_BLOCK_EPISODE \\}\\}");

        int pos = 0;
        while ((pos = rx.indexIn(s, pos)) != -1) {
            pos += rx.matchedLength();

            listEpisodeBlock = rx.cap(0);
            listEpisodeItem = rx.cap(1).trimmed();
        }

        foreach (TvShowEpisode *episode, episodes) {
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
    m.replace("{{ SHOW.TITLE }}", episode->tvShow()->name());
    m.replace("{{ SHOW.LINK }}", QString("../tvshows/%1.html").arg(episode->tvShow()->showId()));
    m.replace("{{ EPISODE.LINK }}", QString("../episodes/%1.html").arg(episode->episodeId()));
    m.replace("{{ EPISODE.TITLE }}", episode->name());
    m.replace("{{ EPISODE.SEASON }}", episode->seasonString());
    m.replace("{{ EPISODE.EPISODE }}", episode->episodeString());
    m.replace("{{ EPISODE.RATING }}", QString::number(episode->rating(), 'f', 1));
    m.replace("{{ EPISODE.CERTIFICATION }}", episode->certification());
    m.replace("{{ EPISODE.FIRST_AIRED }}",
        episode->firstAired().isValid() ? episode->firstAired().toString("yyyy-MM-dd") : "");
    m.replace("{{ EPISODE.LAST_PLAYED }}",
        episode->lastPlayed().isValid() ? episode->lastPlayed().toString("yyyy-MM-dd hh:mm") : "");
    m.replace("{{ EPISODE.STUDIO }}", episode->network());
    m.replace("{{ EPISODE.PLOT }}", episode->overview().replace("\n", "<br />"));
    m.replace("{{ EPISODE.WRITERS }}", episode->writers().join(", "));
    m.replace("{{ EPISODE.DIRECTORS }}", episode->directors().join(", "));

    replaceStreamDetailsVars(m, episode->streamDetails());
    replaceSingleBlock(m, "WRITERS", "WRITER.NAME", episode->writers());
    replaceSingleBlock(m, "DIRECTORS", "DIRECTOR.NAME", episode->directors());
    replaceImages(m, dir, subDir, nullptr, nullptr, nullptr, episode);
}

void ExportDialog::replaceStreamDetailsVars(QString &m, StreamDetails *streamDetails)
{
    m.replace("{{ FILEINFO.WIDTH }}", streamDetails->videoDetails().value("width", "0"));
    m.replace("{{ FILEINFO.HEIGHT }}", streamDetails->videoDetails().value("height", "0"));
    m.replace("{{ FILEINFO.ASPECT }}", streamDetails->videoDetails().value("aspect", "0"));
    m.replace("{{ FILEINFO.CODEC }}", streamDetails->videoDetails().value("codec", ""));
    m.replace("{{ FILEINFO.DURATION }}", streamDetails->videoDetails().value("durationinseconds", "0"));

    QStringList audioCodecs;
    QStringList audioChannels;
    QStringList audioLanguages;
    for (int i = 0, n = streamDetails->audioDetails().count(); i < n; ++i) {
        audioCodecs << streamDetails->audioDetails().at(i).value("codec");
        audioChannels << streamDetails->audioDetails().at(i).value("channels");
        audioLanguages << streamDetails->audioDetails().at(i).value("language");
    }
    m.replace("{{ FILEINFO.AUDIO.CODEC }}", audioCodecs.join("|"));
    m.replace("{{ FILEINFO.AUDIO.CHANNELS }}", audioChannels.join("|"));
    m.replace("{{ FILEINFO.AUDIO.LANGUAGE }}", audioLanguages.join("|"));
}

void ExportDialog::replaceSingleBlock(QString &m, QString blockName, QString itemName, QStringList replaces)
{
    replaceMultiBlock(m, blockName, QStringList() << itemName, QList<QStringList>() << replaces);
}

void ExportDialog::replaceMultiBlock(QString &m, QString blockName, QStringList itemNames, QList<QStringList> replaces)
{
    QRegExp rx;
    rx.setMinimal(true);
    rx.setPattern("\\{\\{ BEGIN_BLOCK_" + blockName + " \\}\\}(.*)\\{\\{ END_BLOCK_" + blockName + " \\}\\}");
    if (rx.indexIn(m) != -1) {
        QString block = rx.cap(0);
        QString item = rx.cap(1).trimmed();
        QStringList list;
        for (int i = 0, n = replaces.at(0).count(); i < n; ++i) {
            QString subItem = item;
            for (int x = 0, y = itemNames.count(); x < y; ++x) {
                subItem.replace("{{ " + itemNames.at(x) + " }}", replaces.at(x).at(i));
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
    Movie *movie,
    Concert *concert,
    TvShow *tvShow,
    TvShowEpisode *episode)
{
    QString item;
    QSize size;
    QRegExp rx("\\{\\{ IMAGE.(.*)\\[(\\d*),(\\d*)\\] \\}\\}");
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
    Movie *movie)
{
    destFile = "movie_images/"
               + QString("%1-%2_%3x%4.jpg").arg(movie->movieId()).arg(type).arg(size.width()).arg(size.height());

    if (type == "poster") {
        QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(movie, ImageType::MoviePoster);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "jpg", 90);
    } else if (type == "fanart") {
        QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(movie, ImageType::MovieBackdrop);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "jpg", 90);
    } else if (type == "logo") {
        QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(movie, ImageType::MovieLogo);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "png", -1);
    } else if (type == "clearart") {
        QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(movie, ImageType::MovieClearArt);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "png", -1);
    } else if (type == "disc") {
        QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(movie, ImageType::MovieCdArt);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "png", -1);
    } else {
        return false;
    }

    return true;
}

bool ExportDialog::saveImageForType(const QString &type,
    const QSize &size,
    const QDir &dir,
    QString &destFile,
    Concert *concert)
{
    destFile = "concert_images/"
               + QString("%1-%2_%3x%4.jpg").arg(concert->concertId()).arg(type).arg(size.width()).arg(size.height());

    if (type == "poster") {
        QString filename =
            Manager::instance()->mediaCenterInterface()->imageFileName(concert, ImageType::ConcertPoster);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "jpg", 90);
    } else if (type == "fanart") {
        QString filename =
            Manager::instance()->mediaCenterInterface()->imageFileName(concert, ImageType::ConcertBackdrop);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "jpg", 90);
    } else if (type == "logo") {
        QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(concert, ImageType::ConcertLogo);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "png", -1);
    } else if (type == "clearart") {
        QString filename =
            Manager::instance()->mediaCenterInterface()->imageFileName(concert, ImageType::ConcertClearArt);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "png", -1);
    } else if (type == "disc") {
        QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(concert, ImageType::ConcertCdArt);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "png", -1);
    } else {
        return false;
    }

    return true;
}

bool ExportDialog::saveImageForType(const QString &type,
    const QSize &size,
    const QDir &dir,
    QString &destFile,
    TvShow *tvShow)
{
    destFile = "tvshow_images/"
               + QString("%1-%2_%3x%4.jpg").arg(tvShow->showId()).arg(type).arg(size.width()).arg(size.height());

    if (type == "poster") {
        QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(tvShow, ImageType::TvShowPoster);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "jpg", 90);
    } else if (type == "fanart") {
        QString filename =
            Manager::instance()->mediaCenterInterface()->imageFileName(tvShow, ImageType::TvShowBackdrop);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "jpg", 90);
    } else if (type == "banner") {
        QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(tvShow, ImageType::TvShowBanner);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "jpg", 90);
    } else if (type == "logo") {
        QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(tvShow, ImageType::TvShowLogos);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "png", -1);
    } else if (type == "clearart") {
        QString filename =
            Manager::instance()->mediaCenterInterface()->imageFileName(tvShow, ImageType::TvShowClearArt);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "png", -1);
    } else if (type == "characterart") {
        QString filename =
            Manager::instance()->mediaCenterInterface()->imageFileName(tvShow, ImageType::TvShowCharacterArt);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "png", -1);
    } else {
        return false;
    }

    return true;
}

bool ExportDialog::saveImageForType(const QString &type,
    const QSize &size,
    const QDir &dir,
    QString &destFile,
    TvShowEpisode *episode)
{
    destFile = "episode_images/"
               + QString("%1-%2_%3x%4.jpg").arg(episode->episodeId()).arg(type).arg(size.width()).arg(size.height());

    if (type == "thumbnail") {
        QString filename =
            Manager::instance()->mediaCenterInterface()->imageFileName(episode, ImageType::TvShowEpisodeThumb);
        if (filename.isEmpty())
            return false;
        saveImage(size, filename, dir.currentPath() + "/" + destFile, "jpg", 90);
    } else {
        return false;
    }

    return true;
}
