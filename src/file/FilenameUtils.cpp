#include "file/FilenameUtils.h"

#include "globals/Meta.h"

#include <QRegExp>
#include <QStringList>
#include <QVector>

namespace mediaelch {
namespace file {

QString stackedBaseName(const QString& fileName)
{
    // TODO: Rework: The variable "volume" is not used. Why was is set in the first place?

    QString baseName = fileName;
    // Assumes that there aren't more parts that 'a' through 'f'.
    QRegExp rx1a(R"((.*)([ _.-]+(?:cd|dvd|pt|part|dis[ck])[ _.-]*[0-9a-f]+)(.*)(\.[^.]+)$)", Qt::CaseInsensitive);
    QRegExp rx1b("(.*)([ _.-]+)$");
    // TODO: DO NOT remove the file extension, see https://github.com/Komet/MediaElch/issues/1175
    // The file extension is removed elsewhere if there is only one file per movie directory.
    // Removing the extension here would mean that many movies are no longer identified!
    // In case that the first does not match, just remove the extension.
    // QRegExp rx2a(R"((.*)(\.[^.]+)$)", Qt::CaseInsensitive);
    // QRegExp rx2b("(.*)([ _.-]+)$");

    QVector<QVector<QRegExp>> regex;
    regex << (QVector<QRegExp>() << rx1a << rx1b);
    // regex << (QVector<QRegExp>() << rx2a << rx2b);

    for (const QVector<QRegExp>& rx : asConst(regex)) {
        if (rx.at(0).indexIn(fileName) == -1) {
            continue;
        }

        QString title = rx.at(0).cap(1);
        QString volume = rx.at(0).cap(2);
        /*QString ignore = rx.at(0).cap(3);
        QString extension = rx.at(0).cap(4);*/
        while (rx.at(1).indexIn(title) != -1) {
            title = rx.at(1).capturedTexts().at(1);
            volume.prepend(rx.at(1).capturedTexts().at(2));
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
