#include "RenamerPlaceholders.h"

#include "log/Log.h"

namespace mediaelch {
QString Placeholder::placeholderText() const
{
    if (isValue && isCondition) {
        return QStringLiteral("{%1}…<%1>…{/%1}").arg(name);
    } else if (isValue) {
        return QStringLiteral("<%1>").arg(name);
    } else if (isCondition) {
        return QStringLiteral("{%1}…{/%1}").arg(name);
    } else {
        qCCritical(generic) << "Placeholder is neither condition nor value";
        MediaElch_Debug_Unreachable();
        return name;
    }
}

RenamerPlaceholders::~RenamerPlaceholders() = default;


RenamerPlaceholders::ValidationResult RenamerPlaceholders::validate(const QString& text)
{
    // TODO: Properly validate placeholders in text, i.e. check that there are no unknown ones.
    Q_UNUSED(text);
    return ValidationResult::valid();
}

QString RenamerPlaceholders::replace(QString text, RenamerData& data)
{
    // TODO: Properly tokenize input string.
    for (const auto& placeholder : placeholders()) {
        if (placeholder.isCondition) {
            QRegularExpression rx(QStringLiteral("{%1}(.*){/%1}").arg(placeholder.name),
                QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
            QRegularExpressionMatch match = rx.match(text);
            if (match.hasMatch()) {
                const QString search = QStringLiteral("{%1}%2{/%1}").arg(placeholder.name, match.captured(1));
                text.replace(search, data.passesCondition(placeholder.name) ? match.captured(1) : "");
            }
        }

        if (placeholder.isValue) {
            text.replace("<" + placeholder.name + ">", data.value(placeholder.name).trimmed());
        }
    }
    return text;
}

} // namespace mediaelch
