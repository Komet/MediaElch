#pragma once

#include <QRegularExpression>
#include <QString>
#include <QVector>
#include <utils/Meta.h>

namespace mediaelch {

struct Placeholder
{
    Placeholder(QString name_, bool isValue_, bool isCondition_, QString translation_) :
        name{name_}, translation{translation_}, isValue{isValue_}, isCondition{isCondition_}
    {
    }

    QString name;
    QString translation;
    bool isValue{false};
    bool isCondition{false};

    QString placeholderText() const;
};

class RenamerData
{
public:
    explicit RenamerData() = default;
    virtual ~RenamerData() = default;

    ELCH_NODISCARD virtual QString value(const QString& name) const = 0;
    ELCH_NODISCARD virtual bool passesCondition(const QString& name) const = 0;
};

class RenamerPlaceholders
{
public:
    struct ValidationError
    {
        QString id;
        QString message;
    };

    struct ValidationResult
    {
        ELCH_NODISCARD static ValidationResult valid() { return ValidationResult{{}}; }
        ELCH_NODISCARD bool isValid() const { return errors.empty(); }
        ELCH_NODISCARD bool hasError() const { return !errors.empty(); }
        QVector<ValidationError> errors;
    };

public:
    explicit RenamerPlaceholders() = default;
    virtual ~RenamerPlaceholders();

    virtual QVector<Placeholder> placeholders() = 0;

    ValidationResult validate(const QString& text);
    QString replace(QString text, RenamerData& data);
};


} // namespace mediaelch
