#include "Helper.h"

#include <QBuffer>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDir>
#include <QDoubleSpinBox>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegExp>
#include <QSpinBox>
#include <QWidget>
#include "globals/Globals.h"
#include "settings/Settings.h"

/**
 * @brief Encodes a string to latin1 percent encoding needed for some scrapers
 * @param str String to encode
 * @return Encoded string
 */
QString Helper::toLatin1PercentEncoding(QString str)
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

QString Helper::urlDecode(QString str)
{
    str = str.replace("&amp;", "&");
    return str;
}

QString Helper::urlEncode(QString str)
{
    QUrl url(str);
    str = url.toEncoded();
    str = str.replace("%26", "&amp;");
    return str;
}

/**
 * @brief Changes the format of a trailer url
 * @param url Trailer Url
 * @return Reformatted trailer url
 */
QString Helper::formatTrailerUrl(QString url)
{
    if (!Settings::instance()->useYoutubePluginUrls())
        return url;

    QString videoId;
    QRegExp rx("youtube.com/watch\\?v=([^&]*)");
    if (rx.indexIn(url, 0) != -1)
        videoId = rx.cap(1);

    if (videoId.isEmpty())
        return url;

    return QString("plugin://plugin.video.youtube/?action=play_video&videoid=%1").arg(videoId);
}


/**
 * @brief Returns true if path is a DVD directory
 * @param path
 * @return
 */
bool Helper::isDvd(QString path, bool noSubFolder)
{
    if (path.endsWith("VIDEO_TS.IFO")) {
        if (noSubFolder)
            return true;
        QFileInfo fi(path);
        return fi.absolutePath().endsWith("VIDEO_TS");
    }
    QDir dir(path);
    QStringList filters;
    filters << "VIDEO_TS" << "VIDEO TS";
    if (dir.entryList(filters, QDir::Dirs | QDir::NoDotAndDotDot).count() > 0) {
        foreach (const QString filter, filters) {
            dir.setPath(path + QDir::separator() + filter);
            if (dir.entryList(QStringList() << "VIDEO_TS.IFO").count() == 1)
                return true;
        }
    }

    return false;
}

/**
 * @brief Returns true if path is a bluray directory
 * @param path
 * @return
 */
bool Helper::isBluRay(QString path)
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
        if (dir.entryList(filters).count() == 1)
            return true;
    }

    return false;
}

QImage &Helper::resizeBackdrop(QImage &image, bool &resized)
{
    resized = false;
    if ((image.width() != 1920 || image.height() != 1080) &&
        image.width() > 1915 && image.width() < 1925 && image.height() > 1075 && image.height() < 1085) {
        image = image.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        resized = true;
    }

    if ((image.width() != 1280 || image.height() != 720) &&
        image.width() > 1275 && image.width() < 1285 && image.height() > 715 && image.height() < 725) {
        image = image.scaled(1280, 720, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        resized = true;
    }

    return image;
}

QByteArray &Helper::resizeBackdrop(QByteArray &image)
{
    bool resized;
    QImage img = QImage::fromData(image);
    Helper::resizeBackdrop(img, resized);
    if (!resized)
        return image;
    QBuffer buffer(&image);
    img.save(&buffer, "jpg", 100);
    return image;
}

QString &Helper::sanitizeFileName(QString &fileName)
{
    fileName.replace("/", " ");
    fileName.replace("\\", " ");
    fileName.replace("$", "");
    fileName.replace("<", "");
    fileName.replace(">", "");
    fileName.replace(":", "");
    fileName.replace("\"", "");
    fileName.replace("?", "");
    fileName.replace("*", "");
    fileName = fileName.trimmed();
    return fileName;
}

QString Helper::stackedBaseName(const QString &fileName)
{
    QString baseName = fileName;
    QRegExp rx1a("(.*)([ _\\.-]*(?:cd|dvd|p(?:ar)?t|dis[ck])[ _\\.-]*[0-9]+)(.*)(\\.[^.]+)$", Qt::CaseInsensitive);
    QRegExp rx1b("(.*)([ _\\.-]+)$");
    QRegExp rx2a("(.*)([ _\\.-]*(?:cd|dvd|p(?:ar)?t|dis[ck])[ _.-]*[a-d])(.*)(\\.[^.]+)$", Qt::CaseInsensitive);
    QRegExp rx2b("(.*)([ _\\.-]+)$");

    QList<QList<QRegExp> > regex;
    regex << (QList<QRegExp>() << rx1a << rx1b);
    regex << (QList<QRegExp>() << rx2a << rx2b);

    foreach (QList<QRegExp> rx, regex) {
        if (rx.at(0).indexIn(fileName) != -1) {
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
    }

    return baseName;
}

QString Helper::appendArticle(const QString &text)
{
    if (!Settings::instance()->ignoreArticlesWhenSorting())
        return text;

    QString name = text;
    foreach (const QString &article, Settings::instance()->advanced()->sortTokens()) {
        if (text.startsWith(article + " ", Qt::CaseInsensitive) && text.length() > article.length()) {
            name = text.mid(article.length()+1) + ", " + text.mid(0, article.length());
            break;
        }
    }
    return name;
}

QString Helper::mapGenre(const QString &text)
{
    if (Settings::instance()->advanced()->genreMappings().isEmpty())
        return text;

    if (Settings::instance()->advanced()->genreMappings().contains(text))
        return Settings::instance()->advanced()->genreMappings().value(text);
    return text;
}

QStringList Helper::mapGenre(const QStringList &genres)
{
    if (Settings::instance()->advanced()->genreMappings().isEmpty())
        return genres;

    QStringList mappedGenres;
    foreach (const QString &genre, genres)
        mappedGenres << Helper::mapGenre(genre);
    return mappedGenres;
}

QString Helper::mapCertification(const QString &text)
{
    if (Settings::instance()->advanced()->certificationMappings().isEmpty())
        return text;

    if (Settings::instance()->advanced()->certificationMappings().contains(text))
        return Settings::instance()->advanced()->certificationMappings().value(text);
    return text;
}

QString Helper::mapStudio(const QString &text)
{
    if (Settings::instance()->advanced()->studioMappings().isEmpty())
        return text;

    if (Settings::instance()->advanced()->studioMappings().contains(text))
        return Settings::instance()->advanced()->studioMappings().value(text);
    return text;
}

QString Helper::mapCountry(const QString &text)
{
    if (Settings::instance()->advanced()->countryMappings().isEmpty())
        return text;

    if (Settings::instance()->advanced()->countryMappings().contains(text))
        return Settings::instance()->advanced()->countryMappings().value(text);
    return text;
}

QString Helper::formatFileSize(const qint64 &size)
{
    if (size > 1024*1024*1024)
        return QString("%1 GB").arg(QString::number((float)size/1024/1024/1024, 'f', 2));
    else if (size > 1024*1024)
        return QString("%1 MB").arg(QString::number((float)size/1024/1024, 'f', 2));
    else if (size > 1024)
        return QString("%1 kB").arg(QString::number((float)size/1024, 'f', 2));
    else
        return QString("%1 B").arg(QString::number((float)size, 'f', 2));
}

void Helper::removeFocusRect(QWidget *widget)
{
    foreach (QLineEdit *edit, widget->findChildren<QLineEdit*>())
        edit->setAttribute(Qt::WA_MacShowFocusRect, false);
    foreach (QComboBox *box, widget->findChildren<QComboBox*>())
        box->setAttribute(Qt::WA_MacShowFocusRect, false);
    foreach (QSpinBox *box, widget->findChildren<QSpinBox*>())
        box->setAttribute(Qt::WA_MacShowFocusRect, false);
    foreach (QDoubleSpinBox *box, widget->findChildren<QDoubleSpinBox*>())
        box->setAttribute(Qt::WA_MacShowFocusRect, false);
    foreach (QDateEdit *dateEdit, widget->findChildren<QDateEdit*>())
        dateEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    foreach (QDateTimeEdit *dateTimeEdit, widget->findChildren<QDateTimeEdit*>())
        dateTimeEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
}

void Helper::applyStyle(QWidget *widget, bool removeFocusRect, bool isTable)
{
    if (removeFocusRect)
        Helper::removeFocusRect(widget);

    QStringList styleSheet = QStringList()
        << "QLabel {"
        << "    font-family: \"Helvetica Neue\";"
        << "    color: #666666;"
    #ifndef Q_OS_MACX
        << "    font-size: 12px;"
    #endif
        << "}"

        << "QLineEdit, QSpinBox, QDateTimeEdit, QTextEdit, QComboBox, QDoubleSpinBox, QCheckBox {"
        << "    border: 0;"
        << "    border-bottom: 1px dotted #e0e0e0;"
        << "}"

        << "QComboBox::down-arrow {"
        << "    image: url(':/img/ui_select.png');"
        << "    width: 16px;"
        << "    height: 16px;"
        << "}";

        if (!isTable)
            styleSheet << "QComboBox { margin-left: 5px; padding-right: 5px; }";

        styleSheet
        << "QCheckBox::indicator:unchecked {"
        << "    image: url(':/img/ui_uncheck.png');"
        << "    width: 16px;"
        << "    height: 16px;"
        << "}"

        << "QCheckBox::indicator:checked {"
        << "    image: url(':/img/ui_check.png');"
        << "    width: 16px;"
        << "    height: 16px;"
        << "}"

        << "QComboBox::drop-down {"
        << "    background-color: #ffffff;"
        << "}"

        << "QTabWidget::pane {"
        << "    border-top: 1px solid #ebebeb;"
        << "    margin-top: -1px;"
        << "}"

        << "QTabBar::tab {"
        << "    padding: 8px;"
        << "    color: #666666;"
        << "    border: 0;"
        << "}"

        << "QTabBar::tab:selected {"
        << "    border: 1px solid #ebebeb;"
        << "    border-bottom: 1px solid #ffffff;"
        << "    border-top-left-radius: 4px;"
        << "    border-top-right-radius: 4px;"
        << "}"

        << "QTabBar::tab:first:!selected {"
        << "    border-left: none;"
        << "}"

        << "QTableWidget {"
        << "    border: none;"
        << "    selection-background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #4185b6, stop:1 #1b6aa5);"
        << "    alternate-background-color: #f9f9f9;"
        << "    selection-color: #ffffff;"
        << "}"

        << "QTableWidget QHeaderView::section {"
        << "    background-color: #f9f9f9;"
        << "    color: rgb(27, 105, 165);"
        << "    border: none;"
        << "    border-left: 1px solid #f0f0f0;"
        << "    font-weight: normal;"
        << "    padding-top: 4px;"
        << "    padding-bottom: 4px;"
        << "    margin-top: 1px;"
        << "    margin-bottom: 1px;"
        << "}"

        << "QTableWidget QHeaderView::section:first {"
        << "    border: none;"
        << "}"

        << "QPushButton {"
        << "    background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #428BCA, stop:1 #3071A9);"
        << "    color: #ffffff;"
        << "    border: 1px solid #2D6CA2;"
        << "    border-radius: 4px;"
        << "    padding: 4px;"
        << "}"

        << "QPushButton::pressed {"
        << "    background-color: #3071A9;"
        << "}"

        << "QPushButton::disabled {"
        << "    background-color: #83b1d9;"
        << "    border: 1px solid #7ca7cb;"
        << "}"

        << ";";

        foreach (QTabWidget *tabWidget, widget->findChildren<QTabWidget*>()) {
            QFont font = tabWidget->font();
            font.setFamily("Helvetica Neue");
            #ifdef Q_OS_MAC
                font.setPointSize(13);
            #else
                font.setPixelSize(12);
            #endif
            tabWidget->setFont(font);
        }

    widget->setStyleSheet(widget->styleSheet() + styleSheet.join("\n"));
}

void Helper::applyEffect(QWidget *parent)
{
    foreach (QLabel *label, parent->findChildren<QLabel*>()) {
        if (label->property("dropShadow").toBool()) {
            QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(parent);
            effect->setColor(QColor(0, 0, 0, 30));
            effect->setOffset(4);
            effect->setBlurRadius(8);
            label->setGraphicsEffect(effect);
        }
    }

    foreach (QPushButton *button, parent->findChildren<QPushButton*>()) {
        if (button->property("dropShadow").toBool()) {
            QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(parent);
            effect->setColor(QColor(0, 0, 0, 30));
            effect->setOffset(2);
            effect->setBlurRadius(4);
            button->setGraphicsEffect(effect);
        }
    }
}

qreal Helper::similarity(const QString &s1, const QString &s2)
{
    const int len1 = s1.length();
    const int len2 = s2.length();

    if (s1 == s2)
        return 1;

    if (len1 == 0 || len2 == 0)
        return 0;

    QList<QList<int> > d;

    d.insert(0, QList<int>());
    d[0].insert(0, 0);
    for (int i=1 ; i<=len1 ; ++i) {
        d.insert(i, QList<int>());
        d[i].insert(0, i);
    }
    for (int i=1; i<=len2 ; ++i)
        d[0].insert(i, i);

    for (int i=1 ; i<=len1; ++i) {
        for (int j=1 ; j<=len2 ; ++j) {
            d[i].insert(j, qMin(qMin(d[i - 1][j] + 1,d[i][j - 1] + 1),
                                d[i - 1][j - 1] + (s1.at(i-1) == s2.at(j-1) ? 0 : 1) ));
        }
    }

    qreal dist = d[len1][len2];
    return 1-(dist/qMax(len1, len2));
}
