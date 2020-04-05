#include "test/test_helpers.h"

#include "file/Path.h"

#include "test/integration/resource_dir.h"

using namespace mediaelch;

TEST_CASE("DirectoryPath", "[path]")
{
    const QString rootPath = resourceDir().absolutePath();
    const QString absolutePath = rootPath + "/export/simple";
    const QString relativePath = "./export/simple";

    // Set the current directory to one whose structure we know.
    QDir::setCurrent(rootPath);

    // Some predefined directories for testing
    const auto parentDir = DirectoryPath(".");
    const auto childDir = DirectoryPath("./export/simple");
    CHECK(parentDir.isValid());
    CHECK(childDir.isValid());

    SECTION("default constructed path is invalid")
    {
        CHECK_FALSE(DirectoryPath().isValid());
        CHECK_FALSE(DirectoryPath("").isValid());
    }

    SECTION("toString() returns a normalized absolute path")
    {
        CHECK(DirectoryPath(absolutePath).toString() == absolutePath);
        CHECK(DirectoryPath(relativePath).toString() == absolutePath);
        CHECK_THAT(DirectoryPath(relativePath).toString(), StartsWith("/"));
    }

    SECTION("dir() returns a valid QDir")
    {
        CHECK(DirectoryPath(rootPath).dir().exists());
        CHECK(parentDir.dir().exists());
        CHECK(childDir.dir().exists());
        CHECK_FALSE(childDir.subDir("does-not-exist").dir().exists());
    }

    SECTION("isParentFolderOf() works")
    {
        CHECK(parentDir.isParentFolderOf(childDir));
        CHECK(parentDir.isParentFolderOf(parentDir.subDir("sub-dirs-should-work")));
    }

    SECTION("dirName() returns the folder name")
    {
        CHECK(parentDir.dirName() == "resources");
        CHECK(childDir.dirName() == "simple");
    }

    SECTION("filePath() returns correct path regardless of whether the file exists")
    {
        CHECK(parentDir.filePath("README.md") == rootPath + "/README.md");
        CHECK(parentDir.filePath("DoesNotExist.md") == rootPath + "/DoesNotExist.md");
        CHECK(childDir.filePath("README.md") == absolutePath + "/README.md");
        CHECK(childDir.filePath("DoesNotExist.md") == absolutePath + "/DoesNotExist.md");
    }

    SECTION("subDir() returns correct path regardless of whether the dir exists")
    {
        CHECK(parentDir.subDir("export").toString() == rootPath + "/export");
        CHECK(parentDir.subDir("DoesNotExist").toString() == rootPath + "/DoesNotExist");
        CHECK(childDir.subDir("export").toString() == absolutePath + "/export");
        CHECK(childDir.subDir("DoesNotExist").toString() == absolutePath + "/DoesNotExist");
    }
}

TEST_CASE("FilePath", "[path]")
{
    const QString rootPath = resourceDir().absolutePath();
    const QString absolutePath = rootPath + "/export/simple";
    const QString relativePath = "./export/simple";

    // Set the current directory to one whose structure we know.
    QDir::setCurrent(rootPath);

    // Predefined test file
    const auto htmlFile = FilePath(relativePath + "/movies.html");
    CHECK(htmlFile.isValid());

    SECTION("default constructed path is invalid")
    {
        CHECK_FALSE(FilePath().isValid());
        CHECK_FALSE(FilePath("").isValid());
    }

    SECTION("toString() returns a normalized absolute path")
    {
        CHECK(htmlFile.toString() == absolutePath + "/movies.html");
    }

    SECTION("dir() returns a valid DirectoryPath")
    { //
        CHECK(htmlFile.dir().toString() == absolutePath);
    }

    SECTION("fileName() works")
    {
        CHECK(htmlFile.fileName() == "movies.html");
        CHECK(FilePath("dir/does.not.exist.txt").fileName() == "does.not.exist.txt");
        CHECK(FilePath("dir/.hiddeOnUnix").fileName() == ".hiddeOnUnix");
    }

    SECTION("fileSuffix() works")
    {
        CHECK(htmlFile.fileSuffix() == "html");
        CHECK(FilePath("dir/does.not.exist.txt").fileSuffix() == "txt");
        CHECK(FilePath("dir/.hiddeOnUnix").fileSuffix() == "hiddeOnUnix");
    }
}
