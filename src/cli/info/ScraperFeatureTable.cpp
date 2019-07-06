#include "cli/info/ScraperFeatureTable.h"


namespace mediaelch {
namespace cli {

void mediaelch::cli::MovieScraperFeatureTable::print()

{
    using namespace std::string_literals;

    m_out << "Supported movie scraper features:" << std::endl;

    TableWriter table(m_out, createTableLayout());
    table.writeHeading();

    QObject parent;
    auto scrapers = Manager::constructMovieScrapers(&parent);

    for (auto* scraper : scrapers) {
        table.writeCell(scraper->name());
        for (MovieScraperInfos feature : m_featureMap.keys()) {
            table.writeCell(hasFeature(*scraper, feature) ? "yes"s : "no"s);
        }
    }

    parent.deleteLater();
}

bool MovieScraperFeatureTable::hasFeature(MovieScraperInterface& scraper, MovieScraperInfos feature)

{
    for (MovieScraperInfos scraperFeature : scraper.scraperSupports()) {
        if (scraperFeature == feature) {
            return true;
        }
    }
    return false;
}

TableLayout MovieScraperFeatureTable::createTableLayout()

{
    TableLayout layout;
    layout.addColumn(TableColumn("Scraper", 20));
    for (const QString& feature : m_featureMap.values()) {
        layout.addColumn(TableColumn(feature, static_cast<unsigned>(feature.size())));
    }
    return layout;
}

} // namespace cli
} // namespace mediaelch
