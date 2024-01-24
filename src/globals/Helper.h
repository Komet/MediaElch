#pragma once

#include "data/Certification.h"
#include "globals/Globals.h"
#include "media/Path.h"

#include <QLabel>
#include <QLocale>
#include <QObject>
#include <QString>

namespace helper {

QString toLatin1PercentEncoding(QString str);
QString urlDecode(QString str);
QString urlEncode(QString str);
QString formatTrailerUrl(QString url);

/// \brief Returns the index for the short English month name (Jan = 1, Dec = 12);
/// Returns -1 for unknown one.
/// This function can be replaced by QDate::fromString(str, "MMM") in Qt 6,
/// but in Qt 5, that function is locale aware.
int monthNameToInt(const QString& month);

bool isDvd(const mediaelch::DirectoryPath& path, bool noSubFolder = false);
bool isDvd(const mediaelch::FilePath& path, bool noSubFolder = false);
bool isDvd(QString path, bool noSubFolder = false);
bool isBluRay(const mediaelch::DirectoryPath& path);
bool isBluRay(const mediaelch::FilePath& path);
bool isBluRay(QString path);

void sanitizeFileName(QString& fileName);
void sanitizeFolderName(QString& fileName);
QString appendArticle(const QString& text);

QString mapGenre(const QString& text);
QStringList mapGenre(const QStringList& genres);
Certification mapCertification(const Certification& certification);
QString mapStudio(const QString& text);
QStringList mapStudio(const QStringList& studio);
QString mapCountry(const QString& text);
QStringList mapCountry(const QStringList& countries);

QString formatFileSizeBinary(double size, const QLocale& locale);
QString formatFileSize(double size, const QLocale& locale);
QString formatFileSize(int64_t size, const QLocale& locale);

qreal similarity(const QString& s1, const QString& s2);
QMap<ColorLabel, QString> labels();
QColor colorForLabel(ColorLabel label, QString theme);
QIcon iconForLabel(ColorLabel label);
QMap<QString, QString> stereoModes();
QString matchResolution(int width, int height, const QString& scanType);

/// \brief Take the given URL and make an HTML link tag.
QString makeHtmlLink(const QUrl& url);

// String Utils
bool containsIgnoreCase(const QStringList& list, const QString& compare);

} // namespace helper
