#include "cli/info/ScraperFeatureTable.h"

#include "globals/Manager.h"

namespace mediaelch {
namespace cli {

void mediaelch::cli::MovieScraperFeatureTable::print()

{
    using namespace std::string_literals;

    m_out << "Supported movie scraper features:" << std::endl;

    TableWriter table(m_out, createTableLayout());
    table.writeHeading();

    QObject parent;
    const auto& scrapers = Manager::instance()->scrapers().movieScrapers();

    for (auto* scraper : scrapers) {
        table.writeCell(scraper->meta().name);
        for (MovieScraperInfo feature : m_featureMap.keys()) {
            table.writeCell(hasFeature(*scraper, feature) ? "yes"s : "no"s);
        }
    }

    parent.deleteLater();
}

bool MovieScraperFeatureTable::hasFeature(mediaelch::scraper::MovieScraper& scraper, MovieScraperInfo feature)
{
    for (MovieScraperInfo scraperFeature : scraper.meta().supportedDetails) {
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
