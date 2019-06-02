#include "cli/info.h"

#include "Version.h"
#include "cli/common.h"
#include "cli/info/ScraperFeatureTable.h"
#include "export/TableWriter.h"
#include "globals/Manager.h"

#include <iomanip>
#include <iostream>

namespace mediaelch {
namespace cli {

enum class InfoObjectType
{
    MovieScrapers,
    Unknown
};

static InfoObjectType infoTypeFromString(QString str)
{
    if ("movie_scrapers" == str) {
        return InfoObjectType::MovieScrapers;
    }
    return InfoObjectType::Unknown;
}

int info(QApplication& app, QCommandLineParser& parser)
{
    parser.clearPositionalArguments();
    // re-add this command so that it appears when help is printed
    parser.addPositionalArgument("info", "Query information about MediaElch.", "info [list_options]");
    parser.addPositionalArgument("details", "What details to show. Possible values:\n - movie_scrapers", "<details>");

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    const QString command = args.size() < 2 ? QString() : args.at(1);

    switch (infoTypeFromString(command)) {
    case InfoObjectType::MovieScrapers: {
        MovieScraperFeatureTable printer(std::cout);
        printer.print();
        return 0;
    }
    case InfoObjectType::Unknown:
        if (command.isEmpty()) {
            std::cout << "Missing info <details>" << std::endl;
        } else {
            std::cout << "Unknown info <details>: " << command.toStdString() << std::endl;
        }
        return 1;
    }

    return 1;
}

} // namespace cli
} // namespace mediaelch
