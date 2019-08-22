#define CATCH_CONFIG_RUNNER
#include "third_party/catch2/catch.hpp"

#include "globals/Meta.h"
#include "test/integration/resource_dir.h"

#include <QApplication>
#include <QDir>
#include <QtGlobal>

int main(int argc, char** argv)
{
    // QDOmNode attributes are stored in a hash map and therefore
    // the order depends on a random seed; we need to set the seed
    // to get a deterministic output string
    // see: https://stackoverflow.com/questions/21535707/incorrect-order-of-attributes-in-qt-xml/39953337#39953337
    qSetGlobalQHashSeed(0);

    QApplication app(argc, argv);
    registerAllMetaTypes();
    Catch::Session session; // NOLINT(clang-analyzer-core.uninitialized.UndefReturn)

    std::string resourceDirString;
    std::string tempDirString;

    // Build a new parser on top of Catch's
    using namespace Catch::clara;
    auto cli = session.cli() // Get Catch's composite command line parser
               | Opt(resourceDirString, "directory")["--resource-dir"](
                   "The test directory which contains reference NFO files, etc.")
               | Opt(tempDirString, "directory")["--temp-dir"](
                   "The temporary directory to which result files can be written.");

    session.cli(cli);

    const int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0) {
        return returnCode;
    }

    // if we don't want to execute tests then don't expect resource arguments
    if (session.config().listTests() || session.config().listTestNamesOnly() || session.config().listTags()
        || session.config().listReporters()) {
        return session.run();
    }

    if (resourceDirString.empty()) {
        std::cerr << "Missing resource directory argument!" << std::endl;
        return 1;
    }
    QDir resourceDir(resourceDirString.c_str());
    if (!resourceDir.exists()) {
        std::cerr << "Resource directory does not exist!" << std::endl;
        return 1;
    }
    if (!resourceDir.isReadable()) {
        std::cerr << "Resource directory is not readable!" << std::endl;
        return 1;
    }

    if (tempDirString.empty()) {
        std::cerr << "Missing temporary directory argument!" << std::endl;
        return 1;
    }
    QDir tempDir(tempDirString.c_str());
    if (!tempDir.exists()) {
        std::cerr << "Temporary directory does not exist!" << std::endl;
        return 1;
    }
    if (!tempDir.isReadable()) {
        std::cerr << "Temporary directory is not readable!" << std::endl;
        return 1;
    }

    try {
        setResourceDir(resourceDir);
        setTempDir(tempDir);

    } catch (const std::runtime_error& error) {
        std::cerr << "An exception was thrown:" << std::endl;
        std::cerr << error.what();
        std::cerr.flush();
        return 1;
    }

    return session.run();
}
