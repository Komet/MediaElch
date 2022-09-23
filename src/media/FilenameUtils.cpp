#include "media/FilenameUtils.h"

#include "utils/Meta.h"

#include <QRegularExpression>
#include <QStringList>
#include <QVector>

namespace mediaelch {
namespace file {

QString stackedBaseName(const QString& fileName)
{
    // TODO: Rework: The variable "volume" is not used. Why was is set in the first place?

    QString baseName = fileName;
    // Assumes that there aren't more parts that 'a' through 'f'.
    static QRegularExpression rx1a(R"(^(.*)([ _.-]+(?:cd|dvd|pt|part|dis[ck])[ _.-]*[0-9a-f]+)(.*)(\.[^.]+)$)",
        QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression rx1b("^(.*)([ _.-]+)$");
    // TODO: DO NOT remove the file extension, see https://github.com/Komet/MediaElch/issues/1175
    // The file extension is removed elsewhere if there is only one file per movie directory.
    // Removing the extension here would mean that many movies are no longer identified!
    // In case that the first does not match, just remove the extension.
    // QRegularExpression rx2a(R"((.*)(\.[^.]+)$)", Qt::CaseInsensitive);
    // QRegularExpression rx2b("(.*)([ _.-]+)$");

    QVector<QVector<QRegularExpression>> regex;
    regex << (QVector<QRegularExpression>() << rx1a << rx1b);
    // regex << (QVector<QRegularExpression>() << rx2a << rx2b);

    for (const QVector<QRegularExpression>& rx : asConst(regex)) {
        QRegularExpressionMatch match = rx.at(0).match(fileName);
        if (!match.hasMatch()) {
            continue;
        }

        QString title = match.captured(1);
        QString volume = match.captured(2);
        /*QString ignore = rx.at(0).cap(3);
        QString extension = rx.at(0).cap(4);*/

        QRegularExpressionMatch titleMatch = rx.at(1).match(title);
        while (titleMatch.hasMatch()) {
            title = titleMatch.captured(1);
            volume.prepend(titleMatch.captured(2));
            titleMatch = rx.at(1).match(title);
        }
        return title;
    }

    return baseName;
}

QString withoutExtension(const QString& fileName)
{
    return fileName.left(fileName.lastIndexOf("."));
}

void sortFilenameList(QStringList& fileNames)
{
    std::sort(fileNames.begin(), fileNames.end(), [](const QString& lhs, const QString& rhs) {
        return (QString::localeAwareCompare(                //
                    mediaelch::file::withoutExtension(lhs), //
                    mediaelch::file::withoutExtension(rhs))
                < 0);
    });
}

} // namespace file
} // namespace mediaelch
