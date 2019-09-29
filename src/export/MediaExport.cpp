#include "export/MediaExport.h"

#include "concerts/Concert.h"
#include "data/StreamDetails.h"
#include "export/SimpleEngine.h"
#include "globals/Manager.h"
#include "movies/Movie.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"

#include <QApplication>
#include <QDebug>
#include <QEventLoop>

namespace mediaelch {

MediaExport::MediaExport(std::atomic_bool& cancelFlag, QObject* parent) : QObject(parent), m_canceled{cancelFlag}
{
}

void MediaExport::doExport(ExportTemplate& exportTemplate,
    QDir directory,
    const QVector<ExportTemplate::ExportSection>& sections)
{
    QDir::setCurrent(directory.path());

    switch (exportTemplate.templateEngine()) {
    case ExportEngine::Simple:
        SimpleEngine engine(exportTemplate, directory, m_canceled);
        connect(&engine, &SimpleEngine::sigItemExported, [&]() { emit sigItemExported(); });

        if (!m_canceled && sections.contains(ExportTemplate::ExportSection::Movies)) {
            engine.exportMovies(Manager::instance()->movieModel()->movies());
        }

        if (!m_canceled && sections.contains(ExportTemplate::ExportSection::TvShows)) {
            engine.exportTvShows(Manager::instance()->tvShowModel()->tvShows());
        }

        if (!m_canceled && sections.contains(ExportTemplate::ExportSection::Concerts)) {
            engine.exportConcerts(Manager::instance()->concertModel()->concerts());
        }
        return;
    }
    qCritical() << "[MediaExport] Unknown template engine!";
}

} // namespace mediaelch
