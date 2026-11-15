#include "test/test_helpers.h"

#include "media/FileFilter.h"

using namespace mediaelch;

TEST_CASE("FileFilter exclusion rules", "[media]")
{
    SECTION("Default and empty filters")
    {
        FileFilter defaults;
        CHECK(defaults.hasValidFilters());
        CHECK_FALSE(defaults.isFileExcluded("movie.mkv"));
        CHECK_FALSE(defaults.isFolderExcluded("Extras"));

        FileFilter empty{QStringList{}};
        CHECK_FALSE(empty.hasValidFilters());
        CHECK_FALSE(empty.isFileExcluded("movie.mkv"));
        CHECK_FALSE(empty.isFolderExcluded("Extras"));
    }

    SECTION("File and folder exclusion rules are independent")
    {
        FileFilter filter;
        filter.fileExcludes = {QRegularExpression("\\.sample$")};
        filter.folderExcludes = {QRegularExpression("^ignored$")};
        CHECK(filter.isFileExcluded("movie.sample"));
        CHECK_FALSE(filter.isFileExcluded("movie.mkv"));
        CHECK(filter.isFolderExcluded("ignored"));
        CHECK_FALSE(filter.isFolderExcluded("Extras"));
        CHECK_FALSE(filter.isFileExcluded("ignored"));
        CHECK_FALSE(filter.isFolderExcluded("movie.sample"));
    }

    SECTION("Invalid regular expressions are ignored")
    {
        FileFilter filter;
        filter.fileExcludes = {QRegularExpression("\\.sample$")};
        filter.folderExcludes = {QRegularExpression("^ignored$")};
        filter.fileExcludes.append(QRegularExpression("["));
        filter.folderExcludes.append(QRegularExpression("["));
        CHECK(filter.isFileExcluded("movie.sample"));
        CHECK_FALSE(filter.isFileExcluded("movie.mkv"));
        CHECK_FALSE(filter.isFolderExcluded("Extras"));
    }
}
