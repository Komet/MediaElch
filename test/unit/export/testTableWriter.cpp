#include "test/test_helpers.h"

#include "export/TableWriter.h"

#include <sstream>
#include <string>

using namespace mediaelch;

namespace {
int countNewlines(const std::string& str)
{
    size_t count = 0;
    size_t pos = 0;
    while ((pos = str.find('\n', pos)) != std::string::npos) {
        ++count;
        ++pos;
    }
    return count;
}
} // namespace

TEST_CASE("TableWriter", "[export]")
{
    SECTION("writeHeading")
    {
        SECTION("writes a markdown-style header and separator row")
        {
            std::ostringstream out;
            TableLayout layout;
            layout.addColumn(TableColumn("Name", 10, ColumnAlignment::Left));
            layout.addColumn(TableColumn("Year", 4, ColumnAlignment::Right));

            TableWriter writer(out, layout);
            writer.writeHeading();

            const std::string result = out.str();
            CHECK(result.find("Name") != std::string::npos);
            CHECK(result.find("Year") != std::string::npos);
            CHECK(result.find("---") != std::string::npos);
        }
    }

    SECTION("writeCell")
    {
        SECTION("writes cell content between pipe characters")
        {
            std::ostringstream out;
            TableLayout layout;
            layout.addColumn(TableColumn("Title", 10, ColumnAlignment::Left));
            layout.addColumn(TableColumn("Year", 4, ColumnAlignment::Right));

            TableWriter writer(out, layout);
            writer.writeCell(QString("Alien"));
            writer.writeCell(QString("1979"));

            const std::string result = out.str();
            CHECK(result.find("Alien") != std::string::npos);
            CHECK(result.find("1979") != std::string::npos);
            CHECK(result.find("|\n") != std::string::npos);
        }

        SECTION("two rows produce two newlines")
        {
            std::ostringstream out;
            TableLayout layout;
            layout.addColumn(TableColumn("A", 5));
            layout.addColumn(TableColumn("B", 5));

            TableWriter writer(out, layout);
            writer.writeCell(QString("r1c1"));
            writer.writeCell(QString("r1c2"));
            writer.writeCell(QString("r2c1"));
            writer.writeCell(QString("r2c2"));

            const std::string result = out.str();
            const int count = countNewlines(result);
            CHECK(count == 2);
        }
    }
}
