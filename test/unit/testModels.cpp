#include "test/test_helpers.h"

#include "concerts/Concert.h"
#include "concerts/ConcertModel.h"
#include "globals/ActorModel.h"
#include "image/ImageModel.h"
#include "movies/Movie.h"
#include "movies/MovieModel.h"
#include "music/Artist.h"
#include "music/MusicModel.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowModel.h"

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

        model->setMovie(movie.get());

        auto tester = std::make_unique<QAbstractItemModelTester>(
            model.get(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    }
}
