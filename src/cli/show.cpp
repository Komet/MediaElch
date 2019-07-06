#include "cli/show.h"

#include "Version.h"
#include "cli/common.h"
#include "globals/Manager.h"

#include <iomanip>
#include <iostream>

namespace mediaelch {
namespace cli {

int show(QApplication& app, QCommandLineParser& parser)
{
    parser.clearPositionalArguments();
    // re-add this command so that it appears when help is printed
    parser.addPositionalArgument("show", "Show a given media item", "show <id>");
    parser.addPositionalArgument("id", "media item id");
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    const QString command = args.size() < 2 ? QString() : args.at(1);

    if (command.isEmpty()) {
        std::cerr << "Missing media identifier." << std::endl;
        return 1;
    }

    ShowConfig config;
    config.id = command;

    std::cerr << "We don't support this command, yet!" << std::endl;

    //    Manager::instance()->movieFileSearcher()->setMovieDirectories(
    //        Settings::instance()->directorySettings().movieDirectories());
    //    Manager::instance()->movieFileSearcher()->reload(false);
    //    MovieModel* movieModel = Manager::instance()->movieModel();

    //    for (Movie* movie : movieModel->movies()) {
    //        if (movie->imdbId() == ImdbId(config.id)) {
    //            std::cerr << movie->name().toStdString() << std::endl;
    //        }
    //    }

    return 1;
}

} // namespace cli
} // namespace mediaelch
