#include "test/helpers/matchers.h"

QStringMatcherBase::QStringMatcherBase(const QString &operation, const QString &comparator) :
    m_comparator(comparator),
    m_operation(operation)
{
}

std::string QStringMatcherBase::describe() const
{
    QString description;
    description.reserve(5 + m_operation.size() + m_comparator.size());
    description += m_operation;
    description += ": \"";
    description += m_comparator;
    description += "\"";
    return description.toStdString();
}

bool EqualsMatcher::match(const QString &source) const
{
    return source == m_comparator;
}

bool ContainsMatcher::match(const QString &source) const
{
    return source.contains(m_comparator);
}

bool StartsWithMatcher::match(const QString &source) const
{
    return source.startsWith(m_comparator);
}

bool EndsWithMatcher::match(const QString &source) const
{
    return source.endsWith(m_comparator);
}

RegexMatcher::RegexMatcher(QString regex) : m_regex(regex)
{
}

bool RegexMatcher::match(const QString &matchee) const
{
    return m_regex.indexIn(matchee) != -1;
}

std::string RegexMatcher::describe() const
{
    QString description("matches \"" + m_regex.pattern() + "\"");
    return description.toStdString();
}

// Matcher Functions

EqualsMatcher Equals(const QString &str)
{
    return EqualsMatcher(str);
}

ContainsMatcher Contains(const QString &str)
{
    return ContainsMatcher(str);
}

EndsWithMatcher EndsWith(const QString &str)
{
    return EndsWithMatcher(str);
}

StartsWithMatcher StartsWith(const QString &str)
{
    return StartsWithMatcher(str);
}

RegexMatcher Matches(const QString &regex)
{
    return RegexMatcher(regex);
}
