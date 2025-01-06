#include "test/test_helpers.h"

#include "renamer/PlaceholderParser.h"


inline std::ostream& operator<<(std::ostream& os, const QVector<mediaelch::PlaceholderParser::Error>& values)
{
    for (const auto& error : values) {
        os << error.code.toStdString() << "; ";
    }
    return os;
}

TEST_CASE("PlaceholderParser for parsing renamer placeholders", "[renamer][parser]")
{
    using namespace mediaelch;

    SECTION("parses valid strings with placeholders")
    {
        auto check_valid = [](const QString& input, QStringList expectedConditions, QStringList expectedValues) {
            CAPTURE(input);
            PlaceholderParser::Result result = PlaceholderParser::parse(input);
            CAPTURE(result.errors);
            CHECK(result.isValid());
            CHECK(!result.hasError());

            for (const QString& expected : expectedConditions) {
                CHECK_THAT(result.conditionPlaceholders, Contains(expected));
            }
            CHECK(result.conditionPlaceholders.size() == expectedConditions.size());

            for (const QString& expected : expectedValues) {
                CHECK_THAT(result.valuePlaceholders, Contains(expected));
            }
            CHECK(result.valuePlaceholders.size() == expectedValues.size());
        };

        check_valid("{foo}content{/foo}", {"foo"}, {});
        check_valid("{foo}content{/foo}{foo2}content{/foo2}", {"foo", "foo2"}, {});
        check_valid("{foo}<content>{/foo}{foo2}content{/foo2}", {"foo", "foo2"}, {"content"});
        check_valid("{foo2}{foo}<content>{/foo}content{/foo2}", {"foo", "foo2"}, {"content"});
        check_valid("{foo2}{foo}{foo}<content>{/foo}{/foo}{/foo2}", {"foo", "foo", "foo2"}, {"content"});

        // Theoretically allowed, we don't mind.
        check_valid("{{{{{{{{{foo}allowed{/{{{{{{{{foo}", {"{{{{{{{{foo"}, {});
        check_valid("<<<<<<<val>", {}, {"<<<<<<val"});
    }

    SECTION("reports errors for invalid strings with placeholders")
    {
        auto check_error = [](const QString& input, QStringList errorMessages) {
            CAPTURE(input);
            PlaceholderParser::Result result = PlaceholderParser::parse(input);
            CAPTURE(result.errors);
            CHECK(!result.isValid());
            REQUIRE(result.hasError());

            QStringList actualErrorMessages;
            for (const auto& err : result.errors) {
                // Missing replacement in human-readable string.
                CHECK_THAT(err.message, ContainsNot("%"));
                actualErrorMessages << err.code;
            }

            for (const QString& msg : errorMessages) {
                CHECK_THAT(actualErrorMessages, Contains(msg));
            }
            CHECK(result.errors.size() == errorMessages.size());
        };

        check_error("{foo}content{/bar}", {"expected '{/foo}', was '{/bar}'"});
        check_error("{foo}content{/foo}{/bar}", {"unexpected '{/bar}'"});
        check_error("{foo}content{/foo}{/bar}{/else}", {"unexpected '{/bar}'"});
        check_error("{foo}content{/", {"no closing '}'"});
        check_error("{foo}{bar}content{/bar}{/else}", {"expected '{/foo}', was '{/else}'"});
        check_error("{foo}content{/}", {"expected '{/foo}', was '{/}'"});

        check_error("{/}", {"unexpected '{/}'"});
        check_error("{", {"no closing '}'"});
        check_error("}", {"mismatched '}'"});
        check_error(">", {"mismatched '>'"});
        check_error(">}>}>}>", {"mismatched '>'"});
        check_error("{}", {"unexpected empty '{}'"});
        check_error("<>", {"unexpected empty '<>'"});
    }
}
