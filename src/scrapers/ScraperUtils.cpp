#include "scrapers/ScraperUtils.h"

#include "utils/Meta.h"

#include <QRegularExpression>
#include <QTextDocument>

namespace mediaelch {

void removeUnicodeSpaces(QString& input)
{
    // \u200B 'ZERO WIDTH SPACE'
    // \u00A0 NON BREAKING SPACE
    static QRegularExpression zeroWidth(QString::fromUtf8("\u200B"));
    static QRegularExpression spaces(QString::fromUtf8("[\u00A0 \\t\\v\\f\\r]+"));
    MediaElch_Debug_Assert(zeroWidth.isValid());
    MediaElch_Debug_Assert(spaces.isValid());
    input.remove(zeroWidth);
    input.replace(spaces, " ");
}

QString removeHtmlEntities(const QString& str)
{
    QTextDocument doc;
    doc.setHtml(std::move(str));
    return doc.toPlainText().trimmed();
}

QString normalizeFromHtml(const QString& str)
{
    QString withoutHtml = removeHtmlEntities(str);
    removeUnicodeSpaces(withoutHtml);
    return withoutHtml;
}

} // namespace mediaelch
