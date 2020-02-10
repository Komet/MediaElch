#include "test/test_helpers.h"

#include "data/Certification.h"

#include <QDebug>
#include <sstream>
#include <string>

using namespace mediaelch;

TEST_CASE("Certification class", "[globals]")
{
    SECTION("constructors")
    {
        CHECK(Certification::FSK("16").toString() == "FSK 16");
        CHECK(Certification("18").toString() == "18");

        auto cert = Certification::FSK("18");
        CHECK(Certification(cert).toString() == "FSK 18");
    }

    SECTION("operators")
    {
        CHECK(Certification("FSK 16") == Certification::FSK("16"));
        CHECK(Certification("FSK 16") != Certification::FSK("18"));
    }

    SECTION("validation")
    {
        CHECK_FALSE(Certification("").isValid());
        CHECK_FALSE(Certification().isValid());
        CHECK_FALSE(Certification::NoCertification.isValid());

        // Class does not trim input
        CHECK(Certification(" ").isValid());
        CHECK(Certification("18").isValid());
        CHECK(Certification::FSK("18").isValid());
    }

    SECTION("output stream")
    {
        std::stringstream stream;

        stream << Certification::FSK("18") << ';';
        stream << Certification::FSK("16") << ';';

        CHECK(stream.str() == "FSK 18;FSK 16;");
    }

    SECTION("QDebug output")
    {
        QString buffer;
        QDebug stream(&buffer);

        stream << Certification::FSK("18");
        stream << Certification::FSK("16");

        CHECK(buffer == "Certification(\"FSK 18\") Certification(\"FSK 16\") ");
    }
}
