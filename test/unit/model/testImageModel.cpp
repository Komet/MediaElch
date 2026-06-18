#include "test/test_helpers.h"

#include "data/Image.h"
#include "model/ImageModel.h"

#include <QAbstractItemModelTester>
#include <memory>

TEST_CASE("ImageModel", "[model]")
{
    SECTION("passes Qt model checker (empty)")
    {
        auto model = std::make_unique<ImageModel>();
        auto tester = std::make_unique<QAbstractItemModelTester>(
            model.get(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    }

    SECTION("empty model has zero rows")
    {
        ImageModel model;
        CHECK(model.rowCount() == 0);
    }

    SECTION("rowCount increases after adding an image")
    {
        // ImageModel takes Qt ownership via setParent(this) and deletes images in its destructor.
        ImageModel model;
        model.addImage(new Image);
        CHECK(model.rowCount() == 1);
    }
}
