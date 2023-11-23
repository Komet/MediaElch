#define CATCH_CONFIG_RUNNER
#include "third_party/catch2/catch.hpp"

#include "test/helpers/resource_dir.h"
#include "utils/Meta.h"

#include <QApplication>
#include <QDir>
#include <QtGlobal>

// TODO: Combine main() with integration_tests

static void usage()
{
    std::cerr << R"cerr(
Usage:
  export PROJECT_ROOT="$(pwd)/../..";
  ./test/scrapers/mediaelch_test_scrapers \
    --use-colour yes \
    --resource-dir "${PROJECT_ROOT}/test/resources" \
    --temp-dir test/resources

Environment Variables:
  MEDIAELCH_UPDATE_REF_FILES  Set to true, and reference files will be updated.
)cerr" << std::endl;
}

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
#ifdef MEDIAELCH_SOURCE_DIR
    resourceDirString = QDir(MEDIAELCH_SOURCE_DIR).absoluteFilePath("test/resources").toStdString();
    tempDirString = QDir(MEDIAELCH_SOURCE_DIR).absoluteFilePath("tmp").toStdString();
#endif

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
        usage();
        return returnCode;
    }

    // if we don't want to execute tests then don't expect resource arguments
    if (session.config().listTests() || session.config().listTestNamesOnly() || session.config().listTags()
        || session.config().listReporters()) {
        return session.run();
    }

    try {
        test::setResourceDir(resourceDirString);
        test::setTempRootDir(tempDirString);

    } catch (const std::runtime_error& error) {
        std::cerr << "An exception was thrown:\n";
        std::cerr << error.what() << std::endl;
        std::cerr.flush();
        return 1;
    }

    std::cerr << R"(
###########################################################
#
# Scraper tests are flaky.  You most likely want to run
#
#   export MEDIAELCH_UPDATE_REF_FILES=1
#   ninja # or make
#   ./test/scrapers/mediaelch_test_scrapers
#
###########################################################

)";

    std::cerr << "Using resource directory: "
              << QDir::toNativeSeparators(test::resourceDir().absolutePath()).toStdString() << "\n";
    std::cerr << "Using temp directory:     " //
              << QDir::toNativeSeparators(test::tempRootDir().absolutePath()).toStdString() << "\n";
    std::cerr.flush();

    const int result = session.run();
    if (result != 0) {
        std::cerr << "An error occurred!" << std::endl;
        if (!test::shouldUpdateResourceFiles()) {
            std::cerr << "Try updating ref files with MEDIAELCH_UPDATE_REF_FILES=1" << std::endl;
        }
    }

    return result;
}
