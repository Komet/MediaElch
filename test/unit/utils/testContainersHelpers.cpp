#include "test/test_helpers.h"

#include "utils/Containers.h"

#include <algorithm>

using namespace mediaelch;

TEST_CASE("container utilities", "[utils]")
{
    SECTION("List to set")
    {
        CHECK(listToSet(QList<int>{}).isEmpty());
        const QSet<int> numbers = listToSet(QList<int>{1, 2, 2, 3});
        CHECK(numbers.size() == 3);
        CHECK(numbers.contains(1));
        CHECK(numbers.contains(2));
        CHECK(numbers.contains(3));
    }

    SECTION("Set to vector")
    {
        CHECK(setToVector(QSet<int>{}).isEmpty());
        QVector<int> vector = setToVector(QSet<int>{1, 2, 3});
        std::sort(vector.begin(), vector.end());
        CHECK(vector == QVector<int>{1, 2, 3});
    }

    SECTION("Set to string list")
    {
        CHECK(setToStringList(QSet<QString>{}).isEmpty());
        const QStringList strings = setToStringList(QSet<QString>{"one", "two"});
        CHECK(strings.size() == 2);
        CHECK(strings.contains("one"));
        CHECK(strings.contains("two"));
    }
}
