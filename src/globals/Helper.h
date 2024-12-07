#pragma once

#include "data/Certification.h"
#include "globals/Globals.h"
#include "media/Path.h"

#include <QLabel>
#include <QLocale>
#include <QObject>
#include <QString>

namespace helper {

ELCH_NODISCARD QString toLatin1PercentEncoding(QString str);
ELCH_NODISCARD QString urlDecode(QString str);
ELCH_NODISCARD QString urlEncode(QString str);
ELCH_NODISCARD QString formatTrailerUrl(QString url);

/// \brief Returns the index for the short English month name (Jan = 1, Dec = 12);
/// Returns -1 for unknown one.
/// This function can be replaced by QDate::fromString(str, "MMM") in Qt 6,
/// but in Qt 5, that function is locale aware.
ELCH_NODISCARD int monthNameToInt(const QString& month);

ELCH_NODISCARD bool isDvd(const mediaelch::DirectoryPath& path, bool noSubFolder = false);
ELCH_NODISCARD bool isDvd(const mediaelch::FilePath& path, bool noSubFolder = false);
ELCH_NODISCARD bool isDvd(QString path, bool noSubFolder = false);
ELCH_NODISCARD bool isBluRay(const mediaelch::DirectoryPath& path);
ELCH_NODISCARD bool isBluRay(const mediaelch::FilePath& path);
ELCH_NODISCARD bool isBluRay(QString path);

/**
 * Sanitize the given file name by replacing characters commonly not supported by filesystems
 * with a space or the given `defaultDelimiter`.
 */
void sanitizeFileName(QString& fileName, const QString& defaultDelimiter = " ");
/**
 * Sanitize the given folder name by replacing characters commonly not supported by filesystems
 * with a space or the given `defaultDelimiter`.
 */
void sanitizeFolderName(QString& fileName, const QString& defaultDelimiter = " ");
ELCH_NODISCARD QString appendArticle(const QString& text);

ELCH_NODISCARD QString mapGenre(const QString& text);
ELCH_NODISCARD QStringList mapGenre(const QStringList& genres);
ELCH_NODISCARD Certification mapCertification(const Certification& certification);
ELCH_NODISCARD QString mapStudio(const QString& text);
ELCH_NODISCARD QStringList mapStudio(const QStringList& studio);
ELCH_NODISCARD QString mapCountry(const QString& text);
ELCH_NODISCARD QStringList mapCountry(const QStringList& countries);

ELCH_NODISCARD QString formatFileSizeBinary(double size, const QLocale& locale);
ELCH_NODISCARD QString formatFileSize(double size, const QLocale& locale);
ELCH_NODISCARD QString formatFileSize(int64_t size, const QLocale& locale);

ELCH_NODISCARD qreal similarity(const QString& s1, const QString& s2);
ELCH_NODISCARD QMap<ColorLabel, QString> labels();
ELCH_NODISCARD QColor colorForLabel(ColorLabel label, QString theme);
ELCH_NODISCARD QIcon iconForLabel(ColorLabel label);
ELCH_NODISCARD QString matchResolution(int width, int height, const QString& scanType);

/// \brief Take the given URL and make an HTML link tag.
ELCH_NODISCARD QString makeHtmlLink(const QUrl& url);

// String Utils
ELCH_NODISCARD bool containsIgnoreCase(const QStringList& list, const QString& compare);

} // namespace helper
