#include "test/test_helpers.h"

#include "data/Rating.h"
#include "model/RatingModel.h"

#include <QAbstractItemModelTester>
#include <memory>

TEST_CASE("RatingModel", "[model]")
{
    SECTION("passes Qt model checker with one item")
    {
        Ratings ratings;
        Rating r;
        r.source = "themoviedb";
        r.rating = 8.8;
        ratings.addRating(r);

        auto model = std::make_unique<RatingModel>();
        model->setRatings(&ratings);
        auto tester = std::make_unique<QAbstractItemModelTester>(
            model.get(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    }

    SECTION("empty model has zero rows")
    {
        RatingModel model;
        CHECK(model.rowCount() == 0);
    }

    SECTION("rowCount reflects rating count after setRatings")
    {
        Ratings ratings;
        Rating r;
        r.source = "imdb";
        r.rating = 7.5;
        ratings.addRating(r);

        RatingModel model;
        model.setRatings(&ratings);
        CHECK(model.rowCount() == 1);
    }

    SECTION("data() returns valid QVariant for first row")
    {
        Ratings ratings;
        Rating r;
        r.source = "imdb";
        r.rating = 7.5;
        ratings.addRating(r);

        RatingModel model;
        model.setRatings(&ratings);
        const QVariant v = model.data(model.index(0, 0), Qt::DisplayRole);
        CHECK(v.isValid());
    }
}
