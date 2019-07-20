#pragma once

#include "third_party/catch2/catch.hpp"

#include <QRegExp>
#include <QString>
#include <string>

struct QStringMatcherBase : Catch::MatcherBase<QString>
{
    QStringMatcherBase(QString operation, QString comparator);
    std::string describe() const override;

    QString m_comparator;
    QString m_operation;
};

struct EqualsMatcher : QStringMatcherBase
{
    EqualsMatcher(const QString& comparator) : QStringMatcherBase("equals", comparator) {}
    bool match(const QString& source) const override;
};

struct ContainsMatcher : QStringMatcherBase
{
    ContainsMatcher(const QString& comparator) : QStringMatcherBase("contains", comparator) {}
    bool match(const QString& source) const override;
};

struct ContainsNotMatcher : QStringMatcherBase
{
    ContainsNotMatcher(const QString& comparator) : QStringMatcherBase("does not contain", comparator) {}
    bool match(const QString& source) const override;
};

struct StartsWithMatcher : QStringMatcherBase
{
    StartsWithMatcher(const QString& comparator) : QStringMatcherBase("starts with", comparator) {}
    bool match(const QString& source) const override;
};

struct EndsWithMatcher : QStringMatcherBase
{
    EndsWithMatcher(const QString& comparator) : QStringMatcherBase("ends with", comparator) {}
    bool match(const QString& source) const override;
};

struct RegexMatcher : Catch::MatcherBase<QString>
{
    RegexMatcher(QString regex);
    bool match(const QString& matchee) const override;
    std::string describe() const override;

private:
    QRegExp m_regex;
};

EqualsMatcher Equals(const QString& str);
ContainsMatcher Contains(const QString& str);
ContainsNotMatcher ContainsNot(const QString& str);
EndsWithMatcher EndsWith(const QString& str);
StartsWithMatcher StartsWith(const QString& str);
RegexMatcher Matches(const QString& regex);
