#include "Helper.h"

#include "globals/Globals.h"
#include "settings/Settings.h"

#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QRegularExpression>

#ifdef Q_OS_MAC
#    include "ui/MacUiUtilities.h"
#endif

namespace helper {

/// \brief Encodes a string to latin1 percent encoding needed for some scrapers
/// \param str String to encode
/// \return Encoded string
QString toLatin1PercentEncoding(QString str)
{
    str = str.toUtf8();
    str = str.replace("%", "%25");
    str = str.replace("ä", "%E4");
    str = str.replace("ö", "%F6");
    str = str.replace("ü", "%FC");
    str = str.replace("Ä", "%C4");
    str = str.replace("Ö", "%D6");
    str = str.replace("Ü", "%DC");
    str = str.replace("?", "%3F");
    str = str.replace(" ", "%20");
    str = str.replace("/", "%2F");
    str = str.replace(";", "%3B");
    str = str.replace("=", "%3D");
    str = str.replace("\"", "%22");
    str = str.replace("(", "%28");
    str = str.replace(")", "%29");
    str = str.replace("'", "%2C");
    str = str.replace(".", "%2E");
    str = str.replace(":", "%3A");
    str = str.replace("<", "%3C");
    str = str.replace(">", "%3E");
    str = str.replace("@", "%40");
    str = str.replace("[", "%5B");
    str = str.replace("]", "%5D");
    str = str.replace("\\", "%5C");
    str = str.replace("^", "%5E");
    str = str.replace("{", "%7B");
    str = str.replace("}", "%7D");
    str = str.replace("|", "%7C");
    str = str.replace("&", "%26");
    str = str.replace("#", "%23");
    return str;
}

QString urlDecode(QString str)
{
    str = str.replace("&amp;", "&");
    return str;
}

QString urlEncode(QString str)
{
    QUrl url(str);
    str = url.toEncoded();
    str = str.replace("%26", "&amp;");
    return str;
}

int monthNameToInt(const QString& month)
{
    if (month == "Jan") {
        return 1;
    }
    if (month == "Feb") {
        return 2;
    }
    if (month == "Mar") {
        return 3;
    }
    if (month == "Apr") {
        return 4;
    }
    if (month == "May") {
        return 5;
    }
    if (month == "June") {
        return 6;
    }
    if (month == "July") {
        return 7;
    }
    if (month == "Aug") {
        return 8;
    }
    if (month == "Sept") {
        return 9;
    }
    if (month == "Oct") {
        return 10;
    }
    if (month == "Nov") {
        return 11;
    }
    if (month == "Dec") {
        return 12;
    }
    return -1;
}

/**
 * \brief Changes the format of a trailer url
 * \param url Trailer Url
 * \return Reformatted trailer url
 */
QString formatTrailerUrl(QString url)
{
    if (!Settings::instance()->useYoutubePluginUrls()) {
        return url;
    }

    QString videoId;
    QRegularExpression rx("youtube.com/watch\\?v=([^&]*)");
    QRegularExpressionMatch match = rx.match(url);
    if (match.hasMatch()) {
        videoId = match.captured(1);
    }

    if (videoId.isEmpty()) {
        return url;
    }

    return QStringLiteral("plugin://plugin.video.youtube/?action=play_video&videoid=%1").arg(videoId);
}

bool isDvd(const mediaelch::DirectoryPath& path, bool noSubFolder)
{
    return isDvd(path.toString(), noSubFolder);
}

bool isDvd(const mediaelch::FilePath& path, bool noSubFolder)
{
    return isDvd(path.toString(), noSubFolder);
}

/**
 * \brief Returns true if path is a DVD directory
 */
bool isDvd(QString path, bool noSubFolder)
{
    if (path.endsWith("VIDEO_TS.IFO")) {
        if (noSubFolder) {
            return true;
        }
        QFileInfo fi(path);
        return fi.absolutePath().endsWith("VIDEO_TS");
    }
    QDir dir(path);
    const QStringList filters{"VIDEO_TS", "VIDEO TS"};
    if (dir.entryList(filters, QDir::Dirs | QDir::NoDotAndDotDot).count() > 0) {
        for (const QString& filter : filters) {
            dir.setPath(path + QDir::separator() + filter);
            if (dir.entryList(QStringList() << "VIDEO_TS.IFO").count() == 1) {
                return true;
            }
        }
    }

    return false;
}

bool isBluRay(const mediaelch::DirectoryPath& path)
{
    return isBluRay(path.toString());
}

bool isBluRay(const mediaelch::FilePath& path)
{
    return isBluRay(path.toString());
}

/**
 * \brief Returns true if path is a bluray directory
 */
bool isBluRay(QString path)
{
    if (path.endsWith("index.bdmv")) {
        QFileInfo fi(path);
        return fi.absolutePath().endsWith("BDMV");
    }

    QDir dir(path);
    QStringList filters;
    filters << "BDMV";
    if (dir.entryList(filters, QDir::Dirs | QDir::NoDotAndDotDot).count() == 1) {
        dir.setPath(path + QDir::separator() + "BDMV");
        filters.clear();
        filters << "index.bdmv";
        if (dir.entryList(filters).count() == 1) {
            return true;
        }
    }

    return false;
}

void sanitizeFileName(QString& fileName)
{
    // Just a few changes to avoid invalid filenames.
    // See https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file
    fileName.replace("<", " ");
    fileName.replace(">", " ");
    fileName.replace(":", " ");
    fileName.replace("\"", " ");
    fileName.replace("/", " ");
    fileName.replace("\\", " ");
    fileName.replace("|", " ");
    fileName.replace("?", "");
    fileName.replace("*", "");

    // Not part of the list above but may cause issues for Shell-Scripts.
    fileName.replace("$", "");

    // If the filename starts with a dot then it is hidden on *nix systems.
    fileName.remove(QRegularExpression("^[.]+"));
    // Replace consecutive spaces
    fileName.replace(QRegularExpression(R"(\s\s+)"), " ");

    fileName = fileName.trimmed();
}

void sanitizeFolderName(QString& fileName)
{
    return sanitizeFileName(fileName);
}

QString appendArticle(const QString& text)
{
    if (!Settings::instance()->ignoreArticlesWhenSorting()) {
        return text;
    }

    QString name = text;
    const auto& tokens = Settings::instance()->advanced()->sortTokens();
    for (const QString& article : tokens) {
        if (text.length() > article.length() && text.startsWith(article, Qt::CaseInsensitive)
            && text[article.size()] == ' ') {
            name = text.mid(article.length() + 1) + ", " + text.mid(0, article.length());
            break;
        }
    }
    return name;
}

QString mapGenre(const QString& text)
{
    if (Settings::instance()->advanced()->genreMappings().isEmpty()) {
        return text;
    }

    if (Settings::instance()->advanced()->genreMappings().contains(text)) {
        return Settings::instance()->advanced()->genreMappings().value(text);
    }
    return text;
}

QStringList mapGenre(const QStringList& genres)
{
    if (Settings::instance()->advanced()->genreMappings().isEmpty()) {
        return genres;
    }

    QStringList mappedGenres;
    for (const QString& genre : genres) {
        mappedGenres << mapGenre(genre);
    }
    return mappedGenres;
}

Certification mapCertification(const Certification& certification)
{
    if (Settings::instance()->advanced()->certificationMappings().isEmpty()) {
        return certification;
    }
    const QString certStr = certification.toString();
    if (Settings::instance()->advanced()->certificationMappings().contains(certStr)) {
        return Certification(Settings::instance()->advanced()->certificationMappings().value(certStr));
    }
    return certification;
}

QString mapStudio(const QString& text)
{
    if (Settings::instance()->advanced()->studioMappings().isEmpty()) {
        return text;
    }

    if (Settings::instance()->advanced()->studioMappings().contains(text)) {
        return Settings::instance()->advanced()->studioMappings().value(text);
    }
    return text;
}

QStringList mapStudio(const QStringList& studios)
{
    if (Settings::instance()->advanced()->studioMappings().isEmpty()) {
        return studios;
    }

    QStringList mappedStudios;
    for (const QString& studio : studios) {
        mappedStudios << mapStudio(studio);
    }
    return mappedStudios;
}


QString mapCountry(const QString& text)
{
    if (Settings::instance()->advanced()->countryMappings().isEmpty()) {
        return text;
    }

    if (Settings::instance()->advanced()->countryMappings().contains(text)) {
        return Settings::instance()->advanced()->countryMappings().value(text);
    }
    return text;
}

QStringList mapCountry(const QStringList& countries)
{
    if (Settings::instance()->advanced()->countryMappings().isEmpty()) {
        return countries;
    }

    QStringList mappedCountries;
    for (const QString& country : countries) {
        mappedCountries << mapCountry(country);
    }
    return mappedCountries;
}


QString formatFileSizeBinary(double size, const QLocale& locale)
{
    // We use the decimal system, i.e. 1000 and not the binary system with 1000.
    // Otherwise, the units would be GiB, Mib and kiB.
    if (size > 1024. * 1024. * 1024.) {
        return QString("%1 GiB").arg(locale.toString(size / 1024.0 / 1024.0 / 1024.0, 'f', 2));
    }
    if (size > 1024. * 1024.) {
        return QString("%1 MiB").arg(locale.toString(size / 1024.0 / 1024.0, 'f', 2));
    }
    if (size > 1024.) {
        return QString("%1 kiB").arg(locale.toString(size / 1024.0, 'f', 2));
    }
    return QString("%1 B").arg(locale.toString(size, 'f', 2));
}

QString formatFileSize(double size, const QLocale& locale)
{
    // We use the decimal system, i.e. 1000 and not the binary system with 1000.
    // Otherwise the units would be GiB, Mib and kiB.
    if (size > 1000. * 1000. * 1000.) {
        return QString("%1 GB").arg(locale.toString(size / 1000.0 / 1000.0 / 1000.0, 'f', 2));
    }
    if (size > 1000. * 1000.) {
        return QString("%1 MB").arg(locale.toString(size / 1000.0 / 1000.0, 'f', 2));
    }
    if (size > 1000.) {
        return QString("%1 kB").arg(locale.toString(size / 1000.0, 'f', 2));
    }
    return QString("%1 B").arg(locale.toString(size, 'f', 2));
}

QString formatFileSize(int64_t size, const QLocale& locale)
{
    return formatFileSize(static_cast<double>(size), locale);
}

qreal similarity(const QString& s1, const QString& s2)
{
    const elch_ssize_t len1 = s1.length();
    const elch_ssize_t len2 = s2.length();

    if (s1 == s2) {
        return 1;
    }

    if (len1 == 0 || len2 == 0) {
        return 0;
    }

    QVector<QVector<int>> d(len1);

    d.insert(0, QVector<int>(len1));
    d[0].insert(0, 0);
    for (int i = 1; i <= len1; ++i) {
        d.insert(i, QVector<int>());
        d[i].insert(0, i);
    }
    for (int i = 1; i <= len2; ++i) {
        d[0].insert(i, i);
    }

    for (int i = 1; i <= len1; ++i) {
        for (int j = 1; j <= len2; ++j) {
            d[i].insert(j,
                qMin(qMin(d[i - 1][j] + 1, d[i][j - 1] + 1), d[i - 1][j - 1] + (s1.at(i - 1) == s2.at(j - 1) ? 0 : 1)));
        }
    }

    qreal dist = d[len1][len2];
    return 1 - (dist / static_cast<qreal>(qMax(len1, len2)));
}

QMap<ColorLabel, QString> labels()
{
    QMap<ColorLabel, QString> labels;
    labels.insert(ColorLabel::NoLabel, QObject::tr("No Label"));
    labels.insert(ColorLabel::Red, QObject::tr("Red"));
    labels.insert(ColorLabel::Orange, QObject::tr("Orange"));
    labels.insert(ColorLabel::Yellow, QObject::tr("Yellow"));
    labels.insert(ColorLabel::Green, QObject::tr("Green"));
    labels.insert(ColorLabel::Blue, QObject::tr("Blue"));
    labels.insert(ColorLabel::Purple, QObject::tr("Purple"));
    labels.insert(ColorLabel::Grey, QObject::tr("Grey"));
    return labels;
}


QColor colorForLabel(ColorLabel label, QString theme)
{
    // TODO: Introduce enum for main window theme; move macOS detection to settings (?)

#ifdef Q_OS_MAC
    if (theme == "auto") {
        theme = mediaelch::ui::macIsInDarkTheme() ? "dark" : "light";
    }
#endif
    theme = theme.startsWith("dark") ? "dark" : "light";

    if (theme == "dark") {
        switch (label) {
        case ColorLabel::Red: return {163, 49, 51};
        case ColorLabel::Orange: return {191, 131, 13};
        case ColorLabel::Yellow: return {191, 176, 29};
        case ColorLabel::Green: return {125, 161, 16};
        case ColorLabel::Blue: return {67, 125, 168};
        case ColorLabel::Purple: return {124, 77, 145};
        case ColorLabel::Grey: return {133, 133, 133};
        case ColorLabel::NoLabel: return {0, 0, 0, 0};
        }
    } else {
        switch (label) {
        case ColorLabel::Red: return {252, 124, 126};
        case ColorLabel::Orange: return {253, 189, 65};
        case ColorLabel::Yellow: return {245, 228, 68};
        case ColorLabel::Green: return {182, 223, 55};
        case ColorLabel::Blue: return {132, 201, 253};
        case ColorLabel::Purple: return {226, 167, 253};
        case ColorLabel::Grey: return {200, 200, 200};
        case ColorLabel::NoLabel: return {0, 0, 0, 0};
        }
    }
    return {0, 0, 0, 0};
}

QIcon iconForLabel(ColorLabel label)
{
    // TODO: Get rid of Settings::instance() here.
    QColor color = colorForLabel(label, Settings::instance()->theme());
    QPainter p;
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    p.begin(&pixmap);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setBrush(QBrush(color));
    QPen pen(color.darker(200));
    pen.setWidth(2);
    p.setPen(pen);
    p.drawEllipse(4, 4, 24, 24);
    p.end();
    return QIcon(pixmap);
}

QString matchResolution(int width, int height, const QString& scanType)
{
    QString res;
    if (height >= 4312 || width >= 7672) {
        res = "4320";
    } else if (height >= 2152 || width >= 3832) {
        res = "2160";
    } else if (height >= 1072 || width >= 1912) {
        res = "1080";
    } else if (height >= 712 || width >= 1272) {
        res = "720";
    } else if (height >= 576) {
        res = "576";
    } else if (height >= 540) {
        res = "540";
    } else if (height >= 480) {
        res = "480";
    } else {
        return "SD";
    }
    if (scanType.toLower() == "progressive") {
        return res + "p";
    }
    if (scanType.toLower() == "interlaced") {
        return res + "i";
    }
    return res;
}

bool containsIgnoreCase(const QStringList& list, const QString& compare)
{
    for (const auto& item : list) {
        if (item.contains(compare, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

QString makeHtmlLink(const QUrl& url)
{
    return QStringLiteral("<a href=\"%1\">%1</a>").arg(url.toString());
}

} // namespace helper
