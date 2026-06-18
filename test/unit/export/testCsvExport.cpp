#include "test/test_helpers.h"

#include "export/CsvExport.h"

#include <QString>
#include <QTextStream>

using namespace mediaelch;

TEST_CASE("CsvExport", "[export]")
{
    SECTION("writeHeader")
    {
        SECTION("separator: tab")
        {
            QString output;
            QTextStream stream(&output);
            CsvExport csv(stream);
            csv.setFieldsInOrder({"Title", "Year", "Rating"});
            csv.setSeparator("\t");
            csv.writeHeader();
            stream.flush();
            CHECK(output == "Title\tYear\tRating\n");
        }

        SECTION("separator: semicolon")
        {
            QString output;
            QTextStream stream(&output);
            CsvExport csv(stream);
            csv.setFieldsInOrder({"Title", "Year"});
            csv.setSeparator(";");
            csv.writeHeader();
            stream.flush();
            CHECK(output == "Title;Year\n");
        }
    }

    SECTION("addRow")
    {
        SECTION("writes values in field order")
        {
            QString output;
            QTextStream stream(&output);
            CsvExport csv(stream);
            csv.setFieldsInOrder({"Title", "Year"});
            csv.setSeparator(" ");
            csv.addRow({{"Year", "1979"}, {"Title", "Alien"}});
            stream.flush();
            CHECK(output == "Alien 1979\n");
        }

        SECTION("missing key writes empty value")
        {
            QString output;
            QTextStream stream(&output);
            CsvExport csv(stream);
            csv.setFieldsInOrder({"Title", "Year"});
            csv.setSeparator("\t");
            csv.addRow({{"Title", "Alien"}});
            stream.flush();
            CHECK(output == "Alien\t\n");
        }

        SECTION("separator inside value is replaced")
        {
            QString output;
            QTextStream stream(&output);
            CsvExport csv(stream);
            csv.setFieldsInOrder({"Title"});
            csv.setSeparator("\t");
            csv.setReplacement("#");
            csv.addRow({{"Title", "Movie\tWith\tTabs"}});
            stream.flush();
            CHECK(output == "Movie#With#Tabs\n");
        }
    }
}
