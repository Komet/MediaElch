#include "test/test_helpers.h"

#include "media_centers/kodi/MovieXmlReader.h"
#include "media_centers/kodi/v16/MovieXmlWriterV16.h"
#include "test/integration/resource_dir.h"

#include <QDateTime>
#include <QDomDocument>
#include <chrono>

using namespace std::chrono_literals;

TEST_CASE("Movie XML writer for Kodi v16", "[data][movie][kodi][nfo]")
{
    SECTION("empty movie")
    {
        Movie movie;
        QString filename = "movie/kodi_v16_movie_empty.nfo";
        QString expected = getFileContent(filename);
        CAPTURE(filename);

        mediaelch::kodi::MovieXmlWriterV16 writer(movie);
        QString actual = writer.getMovieXml().trimmed();

        writeTempFile(filename, actual);
        checkSameXml(expected, actual);
    }

    SECTION("read / write details: empty tvshow")
    {
        Movie movie;
        QString filename = "movie/kodi_v16_movie_empty.nfo";
        QString showContent = getFileContent(filename);
        CAPTURE(filename);

        mediaelch::kodi::MovieXmlReader reader(movie);

        QDomDocument doc;
        doc.setContent(showContent);
        reader.parseNfoDom(doc);

        mediaelch::kodi::MovieXmlWriterV16 writer(movie);
        QString actual = writer.getMovieXml().trimmed();
        writeTempFile(filename, actual);
        checkSameXml(showContent, actual);
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

        movie.setName("Allegiant");
        movie.setOriginalName("Allegiant");
        movie.setSortTitle("TmovieFc10");
        Rating rating;
        rating.rating = 5.8;
        rating.voteCount = 1641;
        rating.source = "IMDb";
        movie.ratings().push_back(rating);
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
        movie.setFiles({R"(F:\Movies- Test - Scraped\Allegiant (2016)\BDMV\index.bdmv)"});
        // TODO: basepath
        movie.setId(ImdbId("tt3410834"));
        movie.addGenre("Adventure");
        movie.addGenre("Science Fiction");
        movie.addCountry("United States of America");

        MovieSet set;
        set.name = "Divergent Collection";
        movie.setSet(set);

        movie.addTag("Best Tag");
        movie.setDirector("Robert Schwentke");
        movie.setWriter("Adam Cooper, Bill Collage, Stephen Chbosky");
        movie.setReleased(QDate::fromString("2016-03-09", Qt::ISODate));
        movie.addStudio("Summit Entertainment");
        movie.setTrailer(QUrl("TmovieFc19"));
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

        mediaelch::kodi::MovieXmlWriterV16 writer(movie);

        QString actual = writer.getMovieXml().trimmed();
        writeTempFile("movie/kodi_v16_movie_all.nfo", actual);
        checkSameXml(getFileContent("movie/kodi_v16_movie_all.nfo"), actual);
    }
}
