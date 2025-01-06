#pragma once

#include "utils/Meta.h"

#include <QDebug>
#include <QStack>
#include <QString>
#include <QVector>

namespace mediaelch {

class PlaceholderParser
{
public:
    struct Error
    {
        QString code;
        QString message;
    };

    struct Result
    {
        QVector<Error> errors;
        QStringList valuePlaceholders;
        QStringList conditionPlaceholders;

        ELCH_NODISCARD bool isValid() const { return errors.isEmpty(); }
        ELCH_NODISCARD bool hasError() const { return !errors.isEmpty(); }

        ELCH_NODISCARD QStringList errorMessageList() const
        {
            QStringList messages;
            for (const Error& error : errors) {
                messages << error.message;
            }
            return messages;
        }
    };

    ~PlaceholderParser() = default;

    static Result parse(const QString& input)
    {
        PlaceholderParser p(input);
        p.createPlaceholderTree();
        return p.m_result;
    }

private:
    PlaceholderParser(const QString& input) : m_input{input} {}

    void createPlaceholderTree();
    elch_ssize_t readOpeningCondition(elch_ssize_t i);
    elch_ssize_t readClosingCondition(elch_ssize_t i);
    elch_ssize_t readValuePlaceholder(elch_ssize_t i);

private:
    const QString& m_input;
    QStack<QString> m_conditionStack;
    Result m_result;
};

} // namespace mediaelch

QDebug operator<<(QDebug dbg, const mediaelch::PlaceholderParser::Error& err);
