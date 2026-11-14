#include "test/test_helpers.h"

#include "media/ImageUtils.h"

using namespace mediaelch;

TEST_CASE("scaledImage", "[media]")
{
    const QImage image(400, 200, QImage::Format_RGB32);
    SECTION("Preserves aspect ratio within bounds")
    {
        CHECK(scaledImage(image, 100, 100).size() == QSize(100, 50));
        CHECK(scaledImage(image, 120, 0).size() == QSize(120, 60));
        CHECK(scaledImage(image, 0, 80).size() == QSize(160, 80));
    }

    SECTION("Supports QSize overload and zero bounds")
    {
        CHECK(scaledImage(image, 0, 0).size() == image.size());
        CHECK(scaledImage(image, QSize(100, 100)).size() == scaledImage(image, 100, 100).size());
        CHECK(scaledImage(image, QSize(0, 80)).size() == scaledImage(image, 0, 80).size());
        CHECK(scaledImage(QImage{}, 100, 100).isNull());
    }
}

TEST_CASE("resizeBackdrop", "[media]")
{
    SECTION("Standard resolutions remain unchanged")
    {
        for (const QSize standard : {QSize(1920, 1080), QSize(1280, 720)}) {
            QImage image(standard, QImage::Format_RGB32);
            bool resized = true;
            resizeBackdrop(image, resized);
            CHECK_FALSE(resized);
            CHECK(image.size() == standard);
        }
    }

    SECTION("Nearby resolutions are normalized")
    {
        QImage fullHd(1921, 1080, QImage::Format_RGB32);
        bool resized = false;
        resizeBackdrop(fullHd, resized);
        CHECK(resized);
        CHECK(fullHd.size() == QSize(1920, 1080));

        QImage hd(1279, 721, QImage::Format_RGB32);
        resized = false;
        resizeBackdrop(hd, resized);
        CHECK(resized);
        CHECK(hd.size() == QSize(1280, 720));
    }

    SECTION("Outside the normalization range remains unchanged")
    {
        for (const QSize excluded : {
                 QSize(1915, 1080),
                 QSize(1925, 1080),
                 QSize(1920, 1075),
                 QSize(1920, 1085),
                 QSize(1275, 720),
                 QSize(1285, 720),
                 QSize(1280, 715),
                 QSize(1280, 725),
                 QSize(800, 450),
             }) {
            QImage image(excluded, QImage::Format_RGB32);
            bool resized = true;
            resizeBackdrop(image, resized);
            CHECK_FALSE(resized);
            CHECK(image.size() == excluded);
        }
    }
}
