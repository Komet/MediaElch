#include "test/test_helpers.h"

#include "media_center/KodiXml.h"
#include "media_center/kodi/MovieXmlReader.h"
#include "media_center/kodi/MovieXmlWriter.h"
#include "test/helpers/resource_dir.h"

#include <QDateTime>
#include <QDomDocument>
#include <chrono>

using namespace std::chrono_literals;

/// Reads a file, parses it, executes callback (you can add further checks), then
/// writes the file to a temporary file and compares the created file with the
/// reference file.
template<class Callback>
static void createAndCompareMovie(const QString& filename, Callback callback)
{
    CAPTURE(filename);

    Movie movie;

    mediaelch::kodi::MovieXmlReader reader(movie);
    QDomDocument doc;
    doc.setContent(test::readResourceFile(filename));
    CHECK(reader.parseNfoDom(doc));

    mediaelch::kodi::MovieXmlWriterGeneric writer(mediaelch::KodiVersion(18), movie);
    QString actual = writer.getMovieXml(true).trimmed();
    test::compareXmlAgainstResourceFile(actual, filename);

    callback(movie);
}

/// Parses a minimal <movie> document built from a <title> and the given child elements.
static void parseMovieChildren(Movie& movie, const QString& children)
{
    mediaelch::kodi::MovieXmlReader reader(movie);
    QDomDocument doc;
    doc.setContent(QStringLiteral("<movie><title>Test</title>%1</movie>").arg(children));
    REQUIRE(reader.parseNfoDom(doc));
}

static MovieSet parseMovieSet(const QString& setXml)
{
    Movie movie;
    parseMovieChildren(movie, setXml);
    return movie.set();
}

TEST_CASE("Movie XML writer for Kodi v18", "[data][movie][kodi][nfo]")
{
    SECTION("Empty movie")
    {
        Movie movie;
        const QString filename = "movie/kodi_v18_movie_empty.nfo";
        CAPTURE(filename);

        mediaelch::kodi::MovieXmlWriterGeneric writer(mediaelch::KodiVersion(18), movie);
        const QString actual = writer.getMovieXml(true).trimmed();
        test::compareXmlAgainstResourceFile(actual, filename);
    }

    SECTION("read / write details: Alien 1979")
    {
        createAndCompareMovie("movie/kodi_v18_Alien_1979.nfo", [](Movie& movie) {
            // check some details
            CHECK(movie.title() == "Alien");
            REQUIRE(!movie.ratings().isEmpty());
            CHECK(movie.ratings().first().voteCount == 7653);
            CHECK(movie.images().posters().size() == 176);  // TODO: currently every thumb is a poster...
            CHECK(movie.images().backdrops().size() == 57); // <fanart>
            CHECK(movie.certification() == Certification("Rated R"));
            CHECK(movie.set().name == "Alien Collection");
            CHECK(movie.set().tmdbId == TmdbId(8091));
            // the collection's id must not be picked up as the movie's id
            CHECK(movie.tmdbId() == TmdbId(348));
            CHECK(movie.actors().size() == 10);
        });
    }

    SECTION("read / write details: Toy Story 3 2010")
    {
        createAndCompareMovie("movie/kodi_v18_Toy_Story_3_2010.nfo", [](Movie& movie) {
            // check some details
            CHECK(movie.title() == "Toy Story 3");
            CHECK(movie.ratings().first().voteCount == 8542);
            CHECK(movie.images().posters().size() == 101);  // TODO: currently every thumb is a poster...
            CHECK(movie.images().backdrops().size() == 29); // <fanart>
            CHECK(movie.certification() == Certification("Rated G"));
            CHECK(movie.set().name == "Toy Story Collection");
            CHECK(movie.set().tmdbId == TmdbId::NoId);
            CHECK(movie.actors().size() == 59);
        });
    }

    SECTION("set id: <uniqueid> takes precedence over the migration fallbacks")
    {
        CHECK(parseMovieSet(R"(<set><name>S</name><uniqueid type="tmdb">1</uniqueid></set>)").tmdbId == TmdbId(1));
        CHECK(parseMovieSet("<set><name>S</name><tmdbid>2</tmdbid></set>").tmdbId == TmdbId(2));
        // Ember Media Manager's spelling
        CHECK(parseMovieSet("<set><name>S</name><tmdb>3</tmdb></set>").tmdbId == TmdbId(3));
        CHECK(parseMovieSet(
                  R"(<set><name>S</name><uniqueid type="tmdb">1</uniqueid><tmdbid>2</tmdbid><tmdb>3</tmdb></set>)")
                  .tmdbId
              == TmdbId(1));
        CHECK(parseMovieSet(R"(<set><name>S</name><uniqueid type="imdb">tt1</uniqueid></set>)").tmdbId == TmdbId::NoId);
        // an empty candidate is skipped instead of hiding the next one
        CHECK(
            parseMovieSet(R"(<set><name>S</name><uniqueid type="tmdb"/><tmdbid>2</tmdbid></set>)").tmdbId == TmdbId(2));
        CHECK(parseMovieSet(R"(<set><name>S</name><tmdbid/><tmdb>3</tmdb></set>)").tmdbId == TmdbId(3));
    }

    SECTION("set id: Ember Media Manager's movie-level <tmdbcolid>")
    {
        Movie ember;
        parseMovieChildren(ember, "<tmdbcolid>2344</tmdbcolid><set>Matrix Filmreihe</set>");
        CHECK(ember.set().name == "Matrix Filmreihe");
        CHECK(ember.set().tmdbId == TmdbId(2344));
        // the collection's id must not be picked up as the movie's id
        CHECK(ember.tmdbId() == TmdbId::NoId);

        Movie inSetWins;
        parseMovieChildren(
            inSetWins, R"(<tmdbcolid>2</tmdbcolid><set><name>S</name><uniqueid type="tmdb">1</uniqueid></set>)");
        CHECK(inSetWins.set().tmdbId == TmdbId(1));

        // a collection id without a set name says nothing we could use
        Movie noSet;
        parseMovieChildren(noSet, "<tmdbcolid>2344</tmdbcolid>");
        CHECK(noSet.set().tmdbId == TmdbId::NoId);
    }

    SECTION("set id is not mistaken for the movie's id")
    {
        Movie movie;
        parseMovieChildren(
            movie, R"(<set><name>S</name><uniqueid type="tmdb">8091</uniqueid><tmdbid>7</tmdbid></set>)");
        CHECK(movie.set().tmdbId == TmdbId(8091));
        CHECK(movie.tmdbId() == TmdbId::NoId);
    }

    SECTION("movie ids come from direct children of <movie>")
    {
        Movie movie;
        parseMovieChildren(movie, "<actor><name>A</name><id>tt99</id></actor><id>tt1</id>");
        CHECK(movie.imdbId() == ImdbId("tt1"));
    }

    SECTION("Full movie details")
    {
        // Taken from https://kodi.wiki/view/NFO_files/Movies#Sample_Movie_nfo_File
        Movie movie;

        // What's missing?
        // - userrating
        // - path
        // - filenameandpath
        // - basepath
        // - uniqueId
        // - set details^(overview)
        // - premiered
        // - status
        // - aired
        // - showlink
        // - resume
        // - dateadded
        // - art

        movie.setTitle("Allegiant");
        movie.setOriginalTitle("AllegiantOriginal");
        movie.setSortTitle("TmovieFc10");

        {
            Rating rating;
            rating.rating = 5.8;
            rating.voteCount = 1641;
            rating.source = "imdb";
            rating.maxRating = 10;
            movie.ratings().setOrAddRating(rating);
        }
        {
            Rating rating;
            rating.rating = 1.2;
            rating.voteCount = 3400;
            rating.source = "themoviedb";
            rating.maxRating = 10;
            movie.ratings().setOrAddRating(rating);
        }
        {
            Rating rating;
            rating.rating = 4.2;
            rating.voteCount = 784;
            rating.source = "someOther";
            movie.ratings().setOrAddRating(rating);
        }

        movie.setTop250(240);
        movie.setOutline("TmovieFc02");
        movie.setOverview("Beatrice Prior and Tobias Eaton venture into the world outside of the fence and are taken "
                          "into protective custody by a mysterious agency known as the Bureau of Genetic Welfare.");
        movie.setTagline("Break the boundaries of your world");
        movie.setRuntime(88min);
        Poster poster;
        poster.originalUrl = "http://image.tmdb.org/t/p/original/tSFBh9Ayn5uiwbUK9HvD2lrRgaQ.jpg";
        poster.thumbUrl = "http://image.tmdb.org/t/p/w500/tSFBh9Ayn5uiwbUK9HvD2lrRgaQ.jpg";
        movie.images().addPoster(poster);
        movie.setCertification(Certification("Rated M"));
        movie.setPlayCount(1);
        movie.setLastPlayed(QDateTime::fromString("2017-09-06 12:44:12", Qt::ISODate));
        movie.setFiles(mediaelch::FileList({R"(F:\Movies- Test - Scraped\Allegiant (2016)\BDMV\index.bdmv)"}));
        // TODO: basepath
        movie.setImdbId(ImdbId("tt3410834"));
        movie.addGenre("Adventure");
        movie.addGenre("Science Fiction");
        movie.addCountry("United States of America");

        MovieSet set;
        set.tmdbId = TmdbId(283579);
        set.name = "Divergent Collection";
        set.overview =
            "A series of dystopian science fiction action films based on the Divergent novels by the American author "
            "Veronica Roth. Set in a dystopian and post-apocalyptic Chicago where people are divided into distinct "
            "factions based on human virtues. Beatrice Prior (Tris) is warned that she is Divergent and thus will "
            "never fit into any one of the factions. She along with Tobias Eaton (Four) soon learn that a sinister "
            "plot is brewing in the seemingly perfect society.";
        movie.setSet(set);

        movie.addTag("Best Tag");
        movie.setDirector("Robert Schwentke");
        movie.setWriter("Adam Cooper, Bill Collage, Stephen Chbosky");
        movie.setReleased(QDate::fromString("2016-03-09", Qt::ISODate));
        movie.addStudio("Summit Entertainment");
        movie.setTrailer(QUrl("TmovieFc19"));
        movie.setTvShowLinks({"Some Allegiant show 1", "Some Allegiant show 2"});
        // requires that setFiles() was called
        movie.streamDetails()->setVideoDetail(StreamDetails::VideoDetails::Codec, "h264");
        movie.streamDetails()->setVideoDetail(StreamDetails::VideoDetails::Aspect, "1.777778");
        movie.streamDetails()->setVideoDetail(StreamDetails::VideoDetails::Width, "1920");
        movie.streamDetails()->setVideoDetail(StreamDetails::VideoDetails::Height, "1080");
        movie.streamDetails()->setVideoDetail(StreamDetails::VideoDetails::DurationInSeconds, "5311");
        movie.streamDetails()->setVideoDetail(StreamDetails::VideoDetails::StereoMode, "");
        movie.streamDetails()->setAudioDetail(1, StreamDetails::AudioDetails::Codec, "ac3");
        movie.streamDetails()->setAudioDetail(1, StreamDetails::AudioDetails::Language, "eng");
        movie.streamDetails()->setAudioDetail(1, StreamDetails::AudioDetails::Channels, "2");
        movie.streamDetails()->setAudioDetail(2, StreamDetails::AudioDetails::Codec, "ac3");
        movie.streamDetails()->setAudioDetail(2, StreamDetails::AudioDetails::Language, "");
        movie.streamDetails()->setAudioDetail(2, StreamDetails::AudioDetails::Channels, "2");
        movie.streamDetails()->setSubtitleDetail(1, StreamDetails::SubtitleDetails::Language, "eng");
        movie.streamDetails()->setSubtitleDetail(2, StreamDetails::SubtitleDetails::Language, "");
        Actor actor;
        actor.name = "Shailene Woodley";
        actor.role = R"(Beatrice "Tris" Prior)";
        actor.thumb = "http://image.tmdb.org/t/p/original/kkLbiTlBGNwJL9qHuVHeqCMNrEx.jpg";
        // TODO: order
        movie.addActor(actor);
        // 2nd actor
        actor.name = "Theo James";
        actor.role = R"(Tobias "Four" Eaton)";
        actor.thumb = "http://image.tmdb.org/t/p/original/hLNSoQ3gc52X5VVb172yO3CuUEq.jpg";
        // TODO: order
        movie.addActor(actor);

        mediaelch::kodi::MovieXmlWriterGeneric writer(mediaelch::KodiVersion(18), movie);
        const QString actual = writer.getMovieXml(true);
        test::compareXmlAgainstResourceFile(actual, "movie/kodi_v18_movie_all.nfo");
    }
}
