#include "test/test_helpers.h"

#include "data/tv_show/TvShow.h"
#include "model/TvShowModel.h"

#include <QAbstractItemModelTester>
#include <memory>

TEST_CASE("TvShowModel", "[model][tv_show]")
{
    SECTION("passes Qt model checker with one item")
    {
        auto model = std::make_unique<TvShowModel>();
        auto show = std::make_unique<TvShow>();
        model->appendShow(show.get());
        auto tester = std::make_unique<QAbstractItemModelTester>(
            model.get(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    }

    SECTION("empty model has zero rows")
    {
        TvShowModel model;
        CHECK(model.rowCount() == 0);
    }

    SECTION("rowCount increases after appending a show")
    {
        TvShowModel model;
        TvShow show;
        model.appendShow(&show);
        CHECK(model.rowCount() == 1);
    }

    SECTION("data() returns valid QVariant for first row")
    {
        TvShowModel model;
        TvShow show;
        model.appendShow(&show);
        const QVariant v = model.data(model.index(0, 0), Qt::DisplayRole);
        CHECK(v.isValid());
    }
}
