#include "checkVersionUpdate.h"

#include "settings/UpdateCheck.h"
#include "common.h"

#include <iostream>

namespace mediaelch {
namespace cli {

int checkVersionUpdate(QApplication& app, QCommandLineParser& parser)
{
    parser.clearPositionalArguments();
    // re-add this command so that it appears when help is printed
    parser.addPositionalArgument("check-update", "Check for version updates.", "check-version [check_version_options]");
    parser.process(app);

    UpdateCheck updateCheck;
    UpdateCheck::Result result;

    QEventLoop loop;

    QEventLoop::connect(
        &updateCheck, &mediaelch::UpdateCheck::updateCheckFinished, &loop, [&](UpdateCheck::Result updateResult) {
            result = updateResult;
            loop.quit();
        });
    updateCheck.checkForUpdate();
    loop.exec();

    if (result.isNewVersionAvailable) {
        std::cout << "New version available: \n";
        std::cout << " Name: " << result.versionName << "\n";
        std::cout << " URL: " << result.downloadUrl.toString(QUrl::FullyEncoded) << "\n";
        std::cout << " Date: " << result.releaseDate << "\n";

    } else {
        std::cout << "Already on latest version!\n";
    }

    std::cout.flush();

    return 0;
}

} // namespace cli
} // namespace mediaelch
