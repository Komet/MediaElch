#include "test/test_helpers.h"

#include "data/concert/Concert.h"
#include "model/ConcertModel.h"

#include <QAbstractItemModelTester>
#include <memory>

TEST_CASE("ConcertModel", "[model][concert]")
{
    SECTION("passes Qt model checker with one item")
    {
        auto model = std::make_unique<ConcertModel>();
        auto concert = std::make_unique<Concert>();
        model->addConcert(concert.get());
        auto tester = std::make_unique<QAbstractItemModelTester>(
            model.get(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    }

    SECTION("empty model has zero rows")
    {
        ConcertModel model;
        CHECK(model.rowCount() == 0);
    }

    SECTION("rowCount increases after adding a concert")
    {
        ConcertModel model;
        Concert concert;
        model.addConcert(&concert);
        CHECK(model.rowCount() == 1);
    }

    SECTION("data() returns valid QVariant for first row")
    {
        ConcertModel model;
        Concert concert;
        model.addConcert(&concert);
        const QVariant v = model.data(model.index(0, 0, QModelIndex()), Qt::DisplayRole);
        CHECK(v.isValid());
    }
}
