#include "test/helpers/matchers.h"

#include "test/helpers/debug_output.h"

#include <sstream>
#include <utility>

QStringMatcherBase::QStringMatcherBase(QString operation, QString comparator) :
    m_comparator(std::move(comparator)), m_operation(std::move(operation))
{
}

std::string QStringMatcherBase::describe() const
{
    QString description;
    description.reserve(6 + m_operation.size() + m_comparator.size());
    description += m_operation;
    description += ": \"";
    description += m_comparator;
    description += "\"";
    return description.toStdString();
}

bool EqualsMatcher::match(const QString& source) const
{
    return source == m_comparator;
}

bool ContainsMatcher::match(const QString& source) const
{
    return source.contains(m_comparator);
}

bool ContainsMatcher::match(const QStringList& source) const
{
    return source.contains(m_comparator);
}

bool ContainsMatcher::match(const QSet<QString>& source) const
{
    return source.contains(m_comparator);
}

bool ContainsNotMatcher::match(const QString& source) const
{
    return !source.contains(m_comparator);
}

bool StartsWithMatcher::match(const QString& source) const
{
    return source.startsWith(m_comparator);
}

bool EndsWithMatcher::match(const QString& source) const
{
    return source.endsWith(m_comparator);
}

RegexMatcher::RegexMatcher(QString regex) : m_regex(regex)
{
}

bool RegexMatcher::match(const QString& matchee) const
{
    return m_regex.match(matchee).hasMatch();
}

std::string RegexMatcher::describe() const
{
    QString description("matches \"" + m_regex.pattern() + "\"");
    return description.toStdString();
}

// Matcher Functions

EqualsMatcher Equals(const QString& str)
{
    return EqualsMatcher(str);
}

ContainsMatcher Contains(const QString& str)
{
    return ContainsMatcher(str);
}

ContainsNotMatcher ContainsNot(const QString& str)
{
    return ContainsNotMatcher(str);
}

EndsWithMatcher EndsWith(const QString& str)
{
    return EndsWithMatcher(str);
}

StartsWithMatcher StartsWith(const QString& str)
{
    return StartsWithMatcher(str);
}

RegexMatcher Matches(const QString& regex)
{
    return RegexMatcher(regex);
}

std::string IsInRangeMatcher::describe() const
{
    std::ostringstream ss;
    ss << "IsInRange [" << m_startInclusive << ", " << m_endExclusive << ")";
    return ss.str();
}

IsInRangeMatcher IsInRange(long long startInclusive, long long endExclusive)
{
    return IsInRangeMatcher(startInclusive, endExclusive);
}

bool HasActorMatcher::match(const QVector<Actor*>& actors) const
{
    for (const Actor* actor : asConst(actors)) {
        if (actor->name == m_name) {
            return (actor->role == m_role);
        }
    }
    return false;
}

std::string HasActorMatcher::describe() const
{
    std::ostringstream ss;
    ss << "has actor " << m_name << " with role " << m_role << ".";
    return ss.str();
}

HasActorMatcher HasActor(const QString& name, const QString& role)
{
    return HasActorMatcher(name, role);
}
