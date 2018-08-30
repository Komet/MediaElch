#define CATCH_CONFIG_RUNNER
#include "thirdParty/catch2/catch.hpp"

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    const int res = Catch::Session().run(argc, argv);
    return res;
}
