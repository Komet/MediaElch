#ifndef TEST_HELPER_MATCHERS
#define TEST_HELPER_MATCHERS

#include "thirdParty/catch2/catch.hpp"

#include <QRegExp>
#include <QString>
#include <string>

struct QStringMatcherBase : Catch::MatcherBase<QString>
{
    QStringMatcherBase(const QString &operation, const QString &comparator);
    std::string describe() const override;

    QString m_comparator;
    QString m_operation;
};

struct EqualsMatcher : QStringMatcherBase
{
    EqualsMatcher(const QString &comparator) : QStringMatcherBase("equals", comparator) {}
    bool match(const QString &source) const override;
};

struct ContainsMatcher : QStringMatcherBase
{
    ContainsMatcher(const QString &comparator) : QStringMatcherBase("contains", comparator) {}
    bool match(const QString &source) const override;
};

struct StartsWithMatcher : QStringMatcherBase
{
    StartsWithMatcher(const QString &comparator) : QStringMatcherBase("starts with", comparator) {}
    bool match(const QString &source) const override;
};

struct EndsWithMatcher : QStringMatcherBase
{
    EndsWithMatcher(const QString &comparator) : QStringMatcherBase("ends with", comparator) {}
    bool match(const QString &source) const override;
};

struct RegexMatcher : Catch::MatcherBase<QString>
{
    RegexMatcher(QString regex);
    bool match(const QString &matchee) const override;
    std::string describe() const override;

private:
    QRegExp m_regex;
};

EqualsMatcher Equals(const QString &str);
ContainsMatcher Contains(const QString &str);
EndsWithMatcher EndsWith(const QString &str);
StartsWithMatcher StartsWith(const QString &str);
RegexMatcher Matches(const QString &regex);

#endif // TEST_HELPER_MATCHERS
