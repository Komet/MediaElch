#include "test/test_helpers.h"

#include "data/concert/Concert.h"
#include "data/movie/Movie.h"
#include "data/music/Artist.h"
#include "data/tv_show/TvShow.h"
#include "model/ActorModel.h"
#include "model/ConcertModel.h"
#include "model/ImageModel.h"
#include "model/MovieModel.h"
#include "model/RatingModel.h"
#include "model/TvShowModel.h"
#include "model/music/MusicModel.h"

#include <QAbstractItemModelTester>
#include <memory>

// Not an in-depth test for our models.
// This test only does some common checks, e.g. checks whether rowCount() returns a valid value, etc.
TEST_CASE("Models pass Qt's builtin model checker", "[movie][model]")
{
    SECTION("MovieModel")
    {
        auto model = std::make_unique<MovieModel>();

        // The model tester also accesses data if there are rows.
        // That's why we insert one row here.
        auto movie = std::make_unique<Movie>();
        model->addMovie(movie.get());

        auto tester = std::make_unique<QAbstractItemModelTester>(
            model.get(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    }

    SECTION("TvShowModel")
    {
        auto model = std::make_unique<TvShowModel>();

        // The model tester also accesses data if there are rows.
        // That's why we insert one row here.
        auto tvShow = std::make_unique<TvShow>();
        model->appendShow(tvShow.get());

        auto tester = std::make_unique<QAbstractItemModelTester>(
            model.get(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    }

    SECTION("MusicModel")
    {
        auto model = std::make_unique<MusicModel>();

        // The model tester also accesses data if there are rows.
        // That's why we insert one row here.
        auto artist = std::make_unique<Artist>();
        model->appendChild(artist.get());

        auto tester = std::make_unique<QAbstractItemModelTester>(
            model.get(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    }

    SECTION("ConcertModel")
    {
        auto model = std::make_unique<ConcertModel>();

        // The model tester also accesses data if there are rows.
        // That's why we insert one row here.
        auto concert = std::make_unique<Concert>();
        model->addConcert(concert.get());

        auto tester = std::make_unique<QAbstractItemModelTester>(
            model.get(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    }

    SECTION("ImageModel")
    {
        auto model = std::make_unique<ImageModel>();
        auto tester = std::make_unique<QAbstractItemModelTester>(
            model.get(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    }

    SECTION("ActorModel")
    {
        auto model = std::make_unique<ActorModel>();

        // The model tester also accesses data if there are rows.
        // That's why we insert one row here.
        auto movie = std::make_unique<Movie>();

        Actor a;
        a.name = "Test Name";
        a.role = "Test Role";
        movie->addActor(std::move(a));

        model->setActors(&movie->actors());

        auto tester = std::make_unique<QAbstractItemModelTester>(
            model.get(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    }

    SECTION("RatingModel")
    {
        auto model = std::make_unique<RatingModel>();

        // The model tester also accesses data if there are rows.
        // That's why we insert one row here.
        Rating r;
        r.source = "themoviedb";
        r.rating = 8.8;
        model->addRating(r);

        auto tester = std::make_unique<QAbstractItemModelTester>(
            model.get(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    }
}
