#include "Version.h"
#include "cli/common.h"
#include "cli/info.h"
#include "cli/list.h"
#include "cli/reload.h"
#include "settings/Settings.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <iostream>
#include <string>

// MediaElch's command line tool
// This tool is currently only used for testing. If it's mature enough,
// these features will be merged into the main executable.

using namespace std::literals::string_literals;

enum class Command
{
    Unknown,
    List,
    Reload,
    Add,
    Show,
    Sync,
    Settings,
    Info,
    Help,
    Version
};

static Command commandFromString(const QString& command)
{
    if ("list" == command) {
        return Command::List;
    } else if ("reload" == command) {
        return Command::Reload;
    } else if ("add" == command) {
        return Command::Add;
    } else if ("show" == command) {
        return Command::Show;
    } else if ("sync" == command) {
        return Command::Sync;
    } else if ("info" == command) {
        return Command::Info;
    } else if ("settings" == command) {
        return Command::Settings;
    } else if ("help" == command) {
        return Command::Help;
    } else if ("version" == command) {
        return Command::Version;
    }
    return Command::Unknown;
}

const char* const helpMessage = R"(Usage: mediaelch [options] <command> [command_options]

Options:
 -h, --help        Print this help notice. For help on commands, use
                   `mediaelch <command> --help` to the command.
 -v, --version     Print Mediaelch's version.
 --verbose=<level> Verbosity level (0: only errors, 4: everything)

commands:
   list        List all media entries.
   reload      Reload all media files.
   add <path>  Add given path to MediaElch's directory settings.
   show <id>   Show an entry with the identifier <id>. <id> can be either
               MediaElch's media id, IMDb id or TheTvDb id for tv shows.
   sync        Sync MediaElch with Kodi. Uses parameters set in settings.
   settings    Get or set MediaElch's settings.
   info        Get various details about MediaElch.
   help        Same as `--help`.
   version     Same as `--version`.
)";

static void printHelp()
{
    std::cout << helpMessage << std::endl;
}

static void printUnsupported()
{
    std::cout << "Command not supported, yet." << std::endl;
}

static int parseArguments(QApplication& app)
{
    QCommandLineParser parser;
    parser.addVersionOption();
    // custom help option that lists all commands
    parser.addOption({{"h", "?", "help"}, "Print help"});
    parser.addOption({"verbose", "Verbosity level (0: only errors, 4: everything)", "level"});
    parser.addHelpOption();
    parser.addPositionalArgument("command", "The command to execute.");

    // Call parse() to find out the positional arguments.
    // Does not process the options.
    parser.parse(QCoreApplication::arguments());

    if (parser.isSet("verbose")) {
        const int verbosity = QString(parser.value("verbose")).toInt();
        mediaelch::cli::setVerbosity(verbosity);
    }

    const QStringList args = parser.positionalArguments();
    const QString command = args.isEmpty() ? QString() : args.first();

    switch (commandFromString(command)) {
    case Command::Help: printHelp(); return 0;
    case Command::Version: parser.showVersion();
    case Command::List: return mediaelch::cli::list(app, parser);
    case Command::Reload: return mediaelch::cli::reload(app, parser);
    case Command::Add: printUnsupported(); return 1;
    case Command::Show: printUnsupported(); return 1;
    case Command::Settings: printUnsupported(); return 1;
    case Command::Sync: printUnsupported(); return 1;
    case Command::Info: return mediaelch::cli::info(app, parser);
    case Command::Unknown:
        // do not process arguments so that we can show our custom help command
        if (command.isEmpty() && parser.isSet("help")) {
            printHelp();
            return 0;
        }

        parser.process(app);
        if (command.isEmpty()) {
            std::cout << "Missing command" << std::endl;
            return 1;
        }

        std::cout << "Unknown command: " << command.toStdString() << std::endl;

        return 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(mediaelch::constants::OrganizationName);
    QCoreApplication::setApplicationName(mediaelch::constants::AppName);
    QCoreApplication::setApplicationVersion(mediaelch::constants::AppVersionStr);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    qInstallMessageHandler(mediaelch::cli::messageHandler);

    Settings::instance(QCoreApplication::instance())->loadSettings();

    return parseArguments(app);
}
