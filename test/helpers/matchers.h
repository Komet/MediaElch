#pragma once

#include "third_party/catch2/catch.hpp"

#include "data/Actor.h"

#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QStringList>
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
    bool match(const QStringList& source) const;
    bool match(const QSet<QString>& source) const;
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
    QRegularExpression m_regex;
};

struct IsInRangeMatcher : Catch::MatcherBase<long long>
{
    IsInRangeMatcher(long long startInclusive, long long endExclusive) :
        m_startInclusive{startInclusive}, m_endExclusive{endExclusive}
    {
    }

    bool match(const long long& other) const override { return m_startInclusive <= other && other < m_endExclusive; }

    std::string describe() const override;

private:
    long long m_startInclusive;
    long long m_endExclusive;
};

struct HasActorMatcher : Catch::MatcherBase<QVector<Actor*>>
{
    HasActorMatcher(const QString& name, const QString& role) : m_name{name}, m_role{role} {}
    bool match(const QVector<Actor*>& actors) const override;
    std::string describe() const override;

private:
    QString m_name;
    QString m_role;
};


EqualsMatcher Equals(const QString& str);
ContainsMatcher Contains(const QString& str);
ContainsNotMatcher ContainsNot(const QString& str);
EndsWithMatcher EndsWith(const QString& str);
StartsWithMatcher StartsWith(const QString& str);
RegexMatcher Matches(const QString& regex);
IsInRangeMatcher IsInRange(long long startInclusive, long long endExclusive);
HasActorMatcher HasActor(const QString& name, const QString& role);
