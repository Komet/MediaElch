#include "PlaceholderParser.h"


#include <QChar>
#include <QObject>

namespace mediaelch {

void PlaceholderParser::createPlaceholderTree()
{
    for (elch_ssize_t i = 0; i < m_input.size(); ++i) {
        QChar c = m_input[i];

        if (c == '{') {
            if ((i + 1) < m_input.size() && m_input[i + 1] == '/') {
                i = readClosingCondition(i);
            } else {
                i = readOpeningCondition(i);
            }

        } else if (c == '}') {
            Error err{QStringLiteral("mismatched '}'"),
                QObject::tr("Unexpected closing '%1', without opening '%2'").arg("}", "{")};
            m_result.errors.push_back(err);
            i = m_input.size();

        } else if (c == '<') {
            i = readValuePlaceholder(i);

        } else if (c == '>') {
            Error err{QStringLiteral("mismatched '>'"),
                QObject::tr("Unexpected closing '%1', without opening '%2'").arg(">", "<")};
            m_result.errors.push_back(err);
            i = m_input.size();

        } else {
            // nothing to do
        }
    }

    if (m_result.isValid() && !m_conditionStack.isEmpty()) {
        QString missing = m_conditionStack.pop();
        Error err{QStringLiteral("Missing closing tag for '%1'").arg(missing),
            QObject::tr("Missing closing tag for '%1'").arg(missing)};
        m_result.errors.push_back(err);
    }
}

elch_ssize_t PlaceholderParser::readOpeningCondition(elch_ssize_t i)
{
    MediaElch_Expects(m_input[i] == '{');

    const char closingCharacter = '}';
    elch_ssize_t start = i + 1;
    while (i < m_input.size() && m_input[i] != closingCharacter) {
        ++i;
    }

    if (i < m_input.size() && m_input[i] == closingCharacter) {
        QString condition = m_input.mid(start, i - start);
        if (start != i) {
            m_result.conditionPlaceholders.push_back(condition);
            m_conditionStack.push_back(condition);
            return i;

        } else {
            Error err{QStringLiteral("unexpected empty '{}'"), QObject::tr("Unexpected empty condition '{}'")};
            m_result.errors.push_back(err);
            return m_input.size();
        }

    } else {
        Error err{QStringLiteral("no closing '%1'").arg(closingCharacter),
            QObject::tr("Missing closing '%1'").arg(closingCharacter)};
        m_result.errors.push_back(err);
        return m_input.size();
    }
}

elch_ssize_t PlaceholderParser::readClosingCondition(elch_ssize_t i)
{
    MediaElch_Expects(m_input[i] == '{');
    MediaElch_Expects(m_input[i + 1] == '/');

    const char closingCharacter = '}';
    elch_ssize_t start = i + 2;
    while (i < m_input.size() && m_input[i] != closingCharacter) {
        ++i;
    }

    if (i < m_input.size() && m_input[i] == closingCharacter) {
        QString actual = m_input.mid(start, i - start);
        if (m_conditionStack.isEmpty()) {
            Error err{QStringLiteral("unexpected '{/%1}'").arg(actual),
                QObject::tr("Unexpected closing tag '{/%1}'.").arg(actual)};
            m_result.errors.push_back(err);
            return m_input.size();
        }

        QString expected = m_conditionStack.pop();
        if (actual == expected) {
            return i;

        } else {
            Error err{QStringLiteral("expected '{/%1}', was '{/%2}'").arg(expected, actual),
                QObject::tr("Expected closing tag '{/%1}', but found '{/%2}'.").arg(expected, actual)};
            m_result.errors.push_back(err);
            return m_input.size();
        }

    } else {
        Error err{QStringLiteral("no closing '%1'").arg(closingCharacter),
            QObject::tr("Missing closing '%1'").arg(closingCharacter)};
        m_result.errors.push_back(err);
        return m_input.size();
    }
}

elch_ssize_t PlaceholderParser::readValuePlaceholder(elch_ssize_t i)
{
    MediaElch_Expects(m_input[i] == '<');

    char closingCharacter = '>';
    elch_ssize_t start = i + 1;
    while (i < m_input.size() && m_input[i] != closingCharacter) {
        ++i;
    }

    if (i < m_input.size() && m_input[i] == closingCharacter) {
        if (start != i) {
            m_result.valuePlaceholders.push_back(m_input.mid(start, i - start));
            return i;

        } else {
            Error err{QStringLiteral("unexpected empty '<>'"), QObject::tr("Unexpected empty value '<>'")};
            m_result.errors.push_back(err);
            return m_input.size();
        }

    } else {
        Error err{QStringLiteral("no closing '%1'").arg(closingCharacter),
            QObject::tr("Missing '%1' for placeholder").arg(closingCharacter)};
        m_result.errors.push_back(err);
        return m_input.size();
    }
}

} // namespace mediaelch
