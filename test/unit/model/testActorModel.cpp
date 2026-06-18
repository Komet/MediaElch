#include "test/test_helpers.h"

#include "data/Actor.h"
#include "data/movie/Movie.h"
#include "model/ActorModel.h"

#include <QAbstractItemModelTester>
#include <memory>

TEST_CASE("ActorModel", "[model]")
{
    SECTION("passes Qt model checker with one item")
    {
        auto model = std::make_unique<ActorModel>();
        auto movie = std::make_unique<Movie>();
        Actor a;
        a.name = "Test Name";
        a.role = "Test Role";
        movie->addActor(std::move(a));
        model->setActors(&movie->actors());
        auto tester = std::make_unique<QAbstractItemModelTester>(
            model.get(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    }

    SECTION("empty model has zero rows")
    {
        ActorModel model;
        CHECK(model.rowCount() == 0);
    }

    SECTION("rowCount reflects actor count")
    {
        ActorModel model;
        Movie movie;
        Actor a;
        a.name = "Actor";
        movie.addActor(std::move(a));
        model.setActors(&movie.actors());
        CHECK(model.rowCount() == 1);
    }

    SECTION("data() returns valid QVariant for first row")
    {
        ActorModel model;
        Movie movie;
        Actor a;
        a.name = "Actor";
        movie.addActor(std::move(a));
        model.setActors(&movie.actors());
        const QVariant v = model.data(model.index(0, 0), Qt::DisplayRole);
        CHECK(v.isValid());
    }
}
