#include "test/test_helpers.h"

#include "data/Certification.h"

TEST_CASE("certification data type", "[data]")
{
    CHECK(Certification("FSK 18") == Certification::FSK("18"));
    CHECK(Certification("") == Certification::NoCertification);
    CHECK(Certification() == Certification::NoCertification);

    CHECK_FALSE(Certification().isValid());
    CHECK_FALSE(Certification("").isValid());

    CHECK(Certification("16").isValid());

    CHECK(Certification("FSK 18").toString() == "FSK 18");
}
