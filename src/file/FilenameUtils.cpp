#include "file/FilenameUtils.h"

#include "globals/Meta.h"

#include <QRegExp>
#include <QStringList>
#include <QVector>

namespace mediaelch {
namespace file {

QString stackedBaseName(const QString& fileName)
{
    QString baseName = fileName;
    QRegExp rx1a(R"((.*)([ _.-]*(?:cd|dvd|p(?:ar)?t|dis[ck])[ _\.-]*[0-9]+)(.*)(\.[^.]+)$)", Qt::CaseInsensitive);
    QRegExp rx1b("(.*)([ _\\.-]+)$");
    QRegExp rx2a(R"((.*)([ _\.-]*(?:cd|dvd|p(?:ar)?t|dis[ck])[ _\.-]*[a-d])(.*)(\.[^.]+)$)", Qt::CaseInsensitive);
    QRegExp rx2b("(.*)([ _\\.-]+)$");

    QVector<QVector<QRegExp>> regex;
    regex << (QVector<QRegExp>() << rx1a << rx1b);
    regex << (QVector<QRegExp>() << rx2a << rx2b);

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

} // namespace file
} // namespace mediaelch
