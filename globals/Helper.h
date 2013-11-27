#ifndef HELPER_H
#define HELPER_H

#include <QImage>
#include <QString>

/**
 * @brief Some convenience functions are bundled here
 */
class Helper
{
public:
    static QString toLatin1PercentEncoding(QString str);
    static QString urlDecode(QString str);
    static QString urlEncode(QString str);
    static QString formatTrailerUrl(QString url);
    static bool isDvd(QString path, bool noSubFolder = false);
    static bool isBluRay(QString path);
    static QImage &resizeBackdrop(QImage &image, bool &resized);
    static QByteArray &resizeBackdrop(QByteArray &image);
    static QString &sanitizeFileName(QString &fileName);
    static QString stackedBaseName(const QString &fileName);
    static QString appendArticle(const QString &text);
    static QString mapGenre(const QString &text);
    static QStringList mapGenre(const QStringList &genres);
    static QString mapCertification(const QString &text);
    static QString mapStudio(const QString &text);
    static QString mapCountry(const QString &text);
    static QString formatFileSize(const qint64 &size);
    static void removeFocusRect(QWidget *widget);
    static void applyStyle(QWidget *widget, bool removeFocusRect = true, bool isTable = false);
    static void applyEffect(QWidget *parent);
    static qreal similarity(const QString &s1, const QString &s2);
};

#endif // HELPER_H
