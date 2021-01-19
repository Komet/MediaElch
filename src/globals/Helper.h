#pragma once

#include "data/Certification.h"
#include "file/Path.h"
#include "globals/Globals.h"

#include <QComboBox>
#include <QImage>
#include <QLabel>
#include <QLocale>
#include <QObject>
#include <QPushButton>
#include <QString>

namespace helper {

enum ButtonStyle
{
    ButtonPrimary,
    ButtonInfo,
    ButtonDanger,
    ButtonSuccess,
    ButtonWarning
};

QString toLatin1PercentEncoding(QString str);
QString urlDecode(QString str);
QString urlEncode(QString str);
QString formatTrailerUrl(QString url);

bool isDvd(const mediaelch::DirectoryPath& path, bool noSubFolder = false);
bool isDvd(const mediaelch::FilePath& path, bool noSubFolder = false);
bool isDvd(QString path, bool noSubFolder = false);
bool isBluRay(const mediaelch::DirectoryPath& path);
bool isBluRay(const mediaelch::FilePath& path);
bool isBluRay(QString path);

QImage& resizeBackdrop(QImage& image, bool& resized);
QByteArray& resizeBackdrop(QByteArray& image);
void sanitizeFileName(QString& fileName);
void sanitizeFolderName(QString& fileName);
QString appendArticle(const QString& text);

QString mapGenre(const QString& text);
QStringList mapGenre(const QStringList& genres);
Certification mapCertification(const Certification& certification);
QString mapStudio(const QString& text);
QString mapCountry(const QString& text);

QString formatFileSizeBinary(double size, const QLocale& locale);
QString formatFileSize(double size, const QLocale& locale);
void removeFocusRect(QWidget* widget);
void applyStyle(QWidget* widget, bool removeFocus = true, bool isTable = false);
void applyEffect(QWidget* parent);
qreal similarity(const QString& s1, const QString& s2);
QMap<ColorLabel, QString> labels();
QColor colorForLabel(ColorLabel label);
QIcon iconForLabel(ColorLabel label);
qreal devicePixelRatio(QLabel* label);
qreal devicePixelRatio(QPushButton* button);
qreal devicePixelRatio(QWidget* widget);
qreal devicePixelRatio(const QPixmap& pixmap);
void setDevicePixelRatio(QPixmap& pixmap, qreal devicePixelRatio);
void setDevicePixelRatio(QImage& image, qreal devicePixelRatio);
void setButtonStyle(QPushButton* button, ButtonStyle style);
void fillStereoModeCombo(QComboBox* box);
QMap<QString, QString> stereoModes();
QString matchResolution(int width, int height, const QString& scanType);
QImage getImage(mediaelch::FilePath path);

/// \brief Take the given URL and make an HTML link tag.
QString makeHtmlLink(const QUrl& url);

// String Utils
bool containsIgnoreCase(const QStringList& list, const QString& compare);

} // namespace helper
