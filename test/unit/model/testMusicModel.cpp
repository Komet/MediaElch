#include "test/test_helpers.h"

#include "data/music/Artist.h"
#include "model/music/MusicModel.h"

#include <QAbstractItemModelTester>
#include <memory>

TEST_CASE("MusicModel", "[model][music]")
{
    SECTION("passes Qt model checker with one item")
    {
        auto model = std::make_unique<MusicModel>();
        auto artist = std::make_unique<Artist>();
        model->appendChild(artist.get());
        auto tester = std::make_unique<QAbstractItemModelTester>(
            model.get(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    }

    SECTION("empty model has zero rows")
    {
        MusicModel model;
        CHECK(model.rowCount() == 0);
    }

    SECTION("rowCount increases after adding an artist")
    {
        MusicModel model;
        Artist artist;
        model.appendChild(&artist);
        CHECK(model.rowCount() == 1);
    }

    SECTION("data() returns valid QVariant for first row")
    {
        MusicModel model;
        Artist artist;
        model.appendChild(&artist);
        const QVariant v = model.data(model.index(0, 0), Qt::DisplayRole);
        CHECK(v.isValid());
    }
}
