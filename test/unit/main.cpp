#define CATCH_CONFIG_RUNNER
#include "third_party/catch2/catch.hpp"

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    Catch::Session session; // NOLINT(clang-analyzer-core.uninitialized.UndefReturn)
    const int res = session.run(argc, argv);
    return res;
}
