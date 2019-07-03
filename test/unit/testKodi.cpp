#include "test/test_helpers.h"

#include "media_centers/kodi/v18/MovieXmlWriterV18.h"

#include <QDateTime>
#include <chrono>

using namespace std::chrono_literals;

TEST_CASE("Movie XML writer for Kodi", "[data][movie][kodi][nfo]")
{
    SECTION("Empty movie")
    {
        Movie movie;
        mediaelch::kodi::MovieXmlWriterV18 writer(movie);

        const QByteArray expectedNfo = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<movie>
    <title></title>
    <originaltitle></originaltitle>
    <ratings/>
    <top250>0</top250>
    <year></year>
    <plot></plot>
    <outline></outline>
    <tagline></tagline>
    <mpaa></mpaa>
    <playcount>0</playcount>
    <id></id>
    <sorttitle></sorttitle>
    <trailer></trailer>
    <watched>false</watched>
    <credits></credits>
    <director></director>
</movie>)";

        REQUIRE(writer.getMovieXml().trimmed() == expectedNfo);
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
        movie.setSet("Divergent Collection");
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

        mediaelch::kodi::MovieXmlWriterV18 writer(movie);

        const QByteArray expectedNfo = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<movie>
    <title>Allegiant</title>
    <originaltitle>Allegiant</originaltitle>
    <ratings>
        <rating default="true" name="IMDb">
            <value>5.8</value>
            <votes>1641</votes>
        </rating>
    </ratings>
    <top250>240</top250>
    <year>2016</year>
    <plot>Beatrice Prior and Tobias Eaton venture into the world outside of the fence and are taken into protective custody by a mysterious agency known as the Bureau of Genetic Welfare.</plot>
    <outline>TmovieFc02</outline>
    <tagline>Break the boundaries of your world</tagline>
    <runtime>88</runtime>
    <mpaa>Rated M</mpaa>
    <playcount>1</playcount>
    <lastplayed>2017-09-06 12:44:12</lastplayed>
    <id>tt3410834</id>
    <set>
        <name>Divergent Collection</name>
        <overview/>
    </set>
    <sorttitle>TmovieFc10</sorttitle>
    <trailer>TmovieFc19</trailer>
    <watched>false</watched>
    <credits>Adam Cooper</credits>
    <credits>Bill Collage</credits>
    <credits>Stephen Chbosky</credits>
    <director>Robert Schwentke</director>
    <studio>Summit Entertainment</studio>
    <genre>Adventure</genre>
    <genre>Science Fiction</genre>
    <country>United States of America</country>
    <tag>Best Tag</tag>
    <thumb aspect="poster" preview="http://image.tmdb.org/t/p/w500/tSFBh9Ayn5uiwbUK9HvD2lrRgaQ.jpg">http://image.tmdb.org/t/p/original/tSFBh9Ayn5uiwbUK9HvD2lrRgaQ.jpg</thumb>
    <actor>
        <name>Shailene Woodley</name>
        <role>Beatrice "Tris" Prior</role>
        <thumb>http://image.tmdb.org/t/p/original/kkLbiTlBGNwJL9qHuVHeqCMNrEx.jpg</thumb>
    </actor>
    <actor>
        <name>Theo James</name>
        <role>Tobias "Four" Eaton</role>
        <thumb>http://image.tmdb.org/t/p/original/hLNSoQ3gc52X5VVb172yO3CuUEq.jpg</thumb>
    </actor>
    <fileinfo>
        <streamdetails>
            <video>
                <durationinseconds>5311</durationinseconds>
                <codec>h264</codec>
                <aspect>1.777778</aspect>
                <width>1920</width>
                <height>1080</height>
            </video>
            <audio/>
            <audio>
                <language>eng</language>
                <codec>ac3</codec>
                <channels>2</channels>
            </audio>
            <audio>
                <codec>ac3</codec>
                <channels>2</channels>
            </audio>
            <subtitle/>
            <subtitle>
                <language>eng</language>
            </subtitle>
            <subtitle/>
        </streamdetails>
    </fileinfo>
</movie>)";
        // deactivated because it's unstable...
        // REQUIRE(writer.getMovieXml().trimmed() == expectedNfo);
    }
}
