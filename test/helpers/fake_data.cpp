#include "test/helpers/fake_data.h"

#include <chrono>

using namespace std::chrono_literals;

namespace test {

std::unique_ptr<Movie> movieWithAllDetails()
{
    auto movie = std::make_unique<Movie>();
    movie->setTitle("Allegiant");
    movie->setOriginalTitle("AllegiantOriginal");
    movie->setSortTitle("TmovieFc10");

    {
        Rating rating;
        rating.rating = 5.8;
        rating.voteCount = 1641;
        rating.source = "imdb";
        rating.maxRating = 10;
        movie->ratings().setOrAddRating(rating);
    }
    {
        Rating rating;
        rating.rating = 1.2;
        rating.voteCount = 3400;
        rating.source = "themoviedb";
        rating.maxRating = 10;
        movie->ratings().setOrAddRating(rating);
    }
    {
        Rating rating;
        rating.rating = 4.2;
        rating.voteCount = 784;
        rating.source = "someOther";
        movie->ratings().setOrAddRating(rating);
    }

    movie->setTop250(240);
    movie->setOutline("TmovieFc02");
    movie->setOverview("Beatrice Prior and Tobias Eaton venture into the world outside of the fence and are taken "
                       "into protective custody by a mysterious agency known as the Bureau of Genetic Welfare.");
    movie->setTagline("Break the boundaries of your world");
    movie->setRuntime(88min);
    Poster poster;
    poster.originalUrl = "http://image.tmdb.org/t/p/original/tSFBh9Ayn5uiwbUK9HvD2lrRgaQ.jpg";
    poster.thumbUrl = "http://image.tmdb.org/t/p/w500/tSFBh9Ayn5uiwbUK9HvD2lrRgaQ.jpg";
    movie->images().addPoster(poster);
    movie->setCertification(Certification("Rated M"));
    movie->setPlayCount(1);
    movie->setLastPlayed(QDateTime::fromString("2017-09-06 12:44:12", Qt::ISODate));
    movie->setFiles(mediaelch::FileList({R"(F:\Movies- Test - Scraped\Allegiant (2016)\BDMV\index.bdmv)"}));
    // TODO: basepath
    movie->setImdbId(ImdbId("tt3410834"));
    movie->addGenre("Adventure");
    movie->addGenre("Science Fiction");
    movie->addCountry("United States of America");

    MovieSet set;
    set.tmdbId = TmdbId(283579);
    set.name = "Divergent Collection";
    set.overview =
        "A series of dystopian science fiction action films based on the Divergent novels by the American author "
        "Veronica Roth. Set in a dystopian and post-apocalyptic Chicago where people are divided into distinct "
        "factions based on human virtues. Beatrice Prior (Tris) is warned that she is Divergent and thus will "
        "never fit into any one of the factions. She along with Tobias Eaton (Four) soon learn that a sinister "
        "plot is brewing in the seemingly perfect society.";
    movie->setSet(set);

    movie->addTag("Best Tag");
    movie->setDirector("Robert Schwentke");
    movie->setWriter("Adam Cooper, Bill Collage, Stephen Chbosky");
    movie->setReleased(QDate::fromString("2016-03-09", Qt::ISODate));
    movie->addStudio("Summit Entertainment");
    movie->setTrailer(QUrl("TmovieFc19"));
    // requires that setFiles() was called
    movie->streamDetails()->setVideoDetail(StreamDetails::VideoDetails::Codec, "h264");
    movie->streamDetails()->setVideoDetail(StreamDetails::VideoDetails::Aspect, "1.777778");
    movie->streamDetails()->setVideoDetail(StreamDetails::VideoDetails::Width, "1920");
    movie->streamDetails()->setVideoDetail(StreamDetails::VideoDetails::Height, "1080");
    movie->streamDetails()->setVideoDetail(StreamDetails::VideoDetails::DurationInSeconds, "5311");
    movie->streamDetails()->setVideoDetail(StreamDetails::VideoDetails::StereoMode, "");
    movie->streamDetails()->setAudioDetail(1, StreamDetails::AudioDetails::Codec, "ac3");
    movie->streamDetails()->setAudioDetail(1, StreamDetails::AudioDetails::Language, "eng");
    movie->streamDetails()->setAudioDetail(1, StreamDetails::AudioDetails::Channels, "2");
    movie->streamDetails()->setAudioDetail(2, StreamDetails::AudioDetails::Codec, "ac3");
    movie->streamDetails()->setAudioDetail(2, StreamDetails::AudioDetails::Language, "");
    movie->streamDetails()->setAudioDetail(2, StreamDetails::AudioDetails::Channels, "2");
    movie->streamDetails()->setSubtitleDetail(1, StreamDetails::SubtitleDetails::Language, "eng");
    movie->streamDetails()->setSubtitleDetail(2, StreamDetails::SubtitleDetails::Language, "");
    Actor actor;
    actor.name = "Shailene Woodley";
    actor.role = R"(Beatrice "Tris" Prior)";
    actor.thumb = "http://image.tmdb.org/t/p/original/kkLbiTlBGNwJL9qHuVHeqCMNrEx.jpg";
    // TODO: order
    movie->addActor(actor);
    // 2nd actor
    actor.name = "Theo James";
    actor.role = R"(Tobias "Four" Eaton)";
    actor.thumb = "http://image.tmdb.org/t/p/original/hLNSoQ3gc52X5VVb172yO3CuUEq.jpg";
    // TODO: order
    movie->addActor(actor);
    return movie;
}
} // namespace test
