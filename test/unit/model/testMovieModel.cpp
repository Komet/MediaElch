#include "test/test_helpers.h"

#include "data/movie/Movie.h"
#include "model/MovieModel.h"

#include <QAbstractItemModelTester>
#include <memory>

TEST_CASE("MovieModel", "[model][movie]")
{
    SECTION("passes Qt model checker with one item")
    {
        auto model = std::make_unique<MovieModel>();
        auto movie = std::make_unique<Movie>();
        model->addMovie(movie.get());
        auto tester = std::make_unique<QAbstractItemModelTester>(
            model.get(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    }

    SECTION("empty model has zero rows")
    {
        MovieModel model;
        CHECK(model.rowCount() == 0);
    }

    SECTION("rowCount increases after adding a movie")
    {
        MovieModel model;
        Movie movie;
        model.addMovie(&movie);
        CHECK(model.rowCount() == 1);
    }

    SECTION("data() returns valid QVariant for first row")
    {
        MovieModel model;
        Movie movie;
        model.addMovie(&movie);
        const QVariant v = model.data(model.index(0, 0, QModelIndex()), Qt::DisplayRole);
        CHECK(v.isValid());
    }
}
