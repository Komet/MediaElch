#include "Helper.h"

#include <QBuffer>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QRegExp>
#include <QSpinBox>
#include <QWidget>
#include "globals/Globals.h"
#include "settings/Settings.h"

Helper::Helper(QObject *parent) :
    QObject(parent)
{
}

Helper *Helper::instance(QObject *parent)
{
    static Helper *m_instance = 0;
    if (!m_instance)
        m_instance = new Helper(parent);
    return m_instance;
}

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
    Helper::instance()->resizeBackdrop(img, resized);
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
        mappedGenres << Helper::instance()->mapGenre(genre);
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
    foreach (QListWidget *list, widget->findChildren<QListWidget*>())
        list->setAttribute(Qt::WA_MacShowFocusRect, false);
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
        Helper::instance()->removeFocusRect(widget);

    QStringList styleSheet = QStringList()
        << "QLabel {"
        << "    color: #666666;"
    #ifndef Q_OS_WIN
        << "    font-family: \"Helvetica Neue\";"
       #ifndef Q_OS_MACX
        << "    font-size: 12px;"
       #endif
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
        << "    border-top: 1px solid #dddddd;"
        << "    margin-top: -1px;"
        << "}"

        << "QTabBar::tab {"
        << "    padding: 8px;"
        << "    color: #777777;"
        << "    border: 0;"
        << "}"

        << "QTabBar::tab:selected {"
        << "    border: 1px solid #dddddd;"
        << "    border-bottom: 1px solid #ffffff;"
        << "    border-top: 2px solid #43a9e4;"
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
   #if defined(Q_OS_MAC)
        << "    font-size: 11px;"
   #endif
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
            font.setWeight(QFont::DemiBold);
            tabWidget->setFont(font);
        }

    widget->setStyleSheet(widget->styleSheet() + styleSheet.join("\n"));

    foreach (QPushButton *button, widget->findChildren<QPushButton*>()) {
        QString styleType = button->property("styleType").toString();
        if (styleType.isEmpty())
            continue;

        if (styleType == "danger")
            Helper::instance()->setButtonStyle(button, Helper::ButtonDanger);
        else if (styleType == "info")
            Helper::instance()->setButtonStyle(button, Helper::ButtonInfo);
        else if (styleType == "primary")
            Helper::instance()->setButtonStyle(button, Helper::ButtonPrimary);
        else if (styleType == "success")
            Helper::instance()->setButtonStyle(button, Helper::ButtonSuccess);
        else if (styleType == "warning")
            Helper::instance()->setButtonStyle(button, Helper::ButtonWarning);
    }
}

void Helper::applyEffect(QWidget *parent)
{
    foreach (QPushButton *button, parent->findChildren<QPushButton*>()) {
        if (button->property("dropShadow").toBool() && Helper::instance()->devicePixelRatio(button) == 1) {
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

QMap<int, QString> Helper::labels()
{
    QMap<int, QString> labels;
    labels.insert(Labels::NO_LABEL, QObject::tr("No Label"));
    labels.insert(Labels::RED, QObject::tr("Red"));
    labels.insert(Labels::ORANGE, QObject::tr("Orange"));
    labels.insert(Labels::YELLOW, QObject::tr("Yellow"));
    labels.insert(Labels::GREEN, QObject::tr("Green"));
    labels.insert(Labels::BLUE, QObject::tr("Blue"));
    labels.insert(Labels::PURPLE, QObject::tr("Purple"));
    labels.insert(Labels::GREY, QObject::tr("Grey"));
    return labels;
}

QColor Helper::colorForLabel(int label)
{
    switch (label) {
    case Labels::RED:
        return QColor(252, 124, 126);
        break;
    case Labels::ORANGE:
        return QColor(253, 189, 65);
        break;
    case Labels::YELLOW:
        return QColor(245, 228, 68);
        break;
    case Labels::GREEN:
        return QColor(182, 223, 55);
        break;
    case Labels::BLUE:
        return QColor(132, 201, 253);
        break;
    case Labels::PURPLE:
        return QColor(226, 167, 253);
        break;
    case Labels::GREY:
        return QColor(200, 200, 200);
        break;
    case Labels::NO_LABEL:
    default:
        return QColor(0, 0, 0, 0);
        break;
    }
}

QIcon Helper::iconForLabel(int label)
{
    QColor color = Helper::instance()->colorForLabel(label);
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

qreal Helper::devicePixelRatio(QLabel *label)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    return label->devicePixelRatio();
#else
    return 1.0;
#endif
}

qreal Helper::devicePixelRatio(QPushButton *button)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    return button->devicePixelRatio();
#else
    return 1.0;
#endif
}

void Helper::setDevicePixelRatio(QPixmap &pixmap, qreal devicePixelRatio)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    pixmap.setDevicePixelRatio(devicePixelRatio);
#endif
}

void Helper::setDevicePixelRatio(QImage &image, qreal devicePixelRatio)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    image.setDevicePixelRatio(devicePixelRatio);
#endif
}

qreal Helper::devicePixelRatio(QWidget *widget)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    return widget->devicePixelRatio();
#else
    return 1.0;
#endif
}

qreal Helper::devicePixelRatio(const QPixmap &pixmap)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    return pixmap.devicePixelRatio();
#else
    return 1.0;
#endif
}

int Helper::compareVersionNumbers(const QString &oldVersion, const QString &newVersion)
{
    int mMajor;
    int mMinor;
    int mBugfix;
    int xmlMajor;
    int xmlMinor;
    int xmlBugfix;

    QRegExp rxBig("^([0-9])\\.([0-9])\\.([0-9])");
    QRegExp rxNormal("^([0-9])\\.([0-9])");

    if (rxBig.indexIn(oldVersion) != -1) {
        mMajor = rxBig.cap(1).toInt();
        mMinor = rxBig.cap(2).toInt();
        mBugfix = rxBig.cap(3).toInt();
    } else if (rxNormal.indexIn(oldVersion) != -1) {
        mMajor = rxNormal.cap(1).toInt();
        mMinor = rxNormal.cap(2).toInt();
        mBugfix = 0;
    } else {
        return 0;
    }

    if (rxBig.indexIn(newVersion) != -1) {
        xmlMajor = rxBig.cap(1).toInt();
        xmlMinor = rxBig.cap(2).toInt();
        xmlBugfix = rxBig.cap(3).toInt();
    } else if (rxNormal.indexIn(newVersion) != -1) {
        xmlMajor = rxNormal.cap(1).toInt();
        xmlMinor = rxNormal.cap(2).toInt();
        xmlBugfix = 0;
    } else {
        return 0;
    }

    if (xmlMajor > mMajor)
        return 1;
    else if (xmlMajor < mMajor)
        return -1;

    if (xmlMajor == mMajor && xmlMinor > mMinor)
        return 1;
    else if (xmlMajor == mMajor && xmlMinor < mMinor)
        return -1;

    if (xmlMajor == mMajor && xmlMinor == mMinor && xmlBugfix > mBugfix)
        return 1;
    else if (xmlMajor == mMajor && xmlMinor == mMinor && xmlBugfix < mBugfix)
        return -1;

    return 0;
}

void Helper::setButtonStyle(QPushButton *button, Helper::ButtonStyle style)
{
    QString styleSheet;

    styleSheet.append("QPushButton {");
    styleSheet.append("padding: 4px;");
    styleSheet.append("margin: 4px;");
    styleSheet.append("border-radius: 4px;");
#if defined(Q_OS_MAC)
    styleSheet.append("font-size: 11px;");
#endif
    styleSheet.append("}");

    if (style == Helper::ButtonDanger) {
        styleSheet.append("QPushButton { color: #ffffff; background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(238, 95, 91, 255), stop:1 rgba(189, 53, 47, 255)); border: 1px solid #C12E2A;}");
        styleSheet.append("QPushButton::pressed { background-color: rgb(189, 53, 47); }");
        styleSheet.append("QPushButton::disabled { background-color: rgb(213, 125, 120); }");
        styleSheet.append("margin-bottom: 2px;");
    } else if (style == Helper::ButtonPrimary) {
        styleSheet.append("QPushButton { color: #ffffff; background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(66, 139, 202, 255), stop:1 rgba(48, 113, 169, 255)); border: 1px solid #2D6CA2; }");
        styleSheet.append("QPushButton::pressed { background-color: rgb(48, 113, 169); }");
        styleSheet.append("QPushButton::disabled { background-color: rgb(66, 139, 202); }");
        styleSheet.append("margin-bottom: 2px;");
    } else if (style == Helper::ButtonInfo) {
        styleSheet.append("QPushButton { color: #ffffff; background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #5BC0DE, stop:1 #31B0D5); border: 1px solid #2AABD2; }");
        styleSheet.append("QPushButton::pressed { background-color: #31B0D5; }");
        styleSheet.append("QPushButton::disabled { background-color: #79cce4; }");
        styleSheet.append("margin-bottom: 2px;");
    } else if (style == Helper::ButtonWarning) {
        styleSheet.append("QPushButton { color: #ffffff; background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(251, 180, 80, 255), stop:1 rgba(248, 148, 6, 255)); border: 1px solid #EB9316; }");
        styleSheet.append("QPushButton::pressed { background-color: rgb(248, 148, 6); }");
        styleSheet.append("QPushButton::disabled { background-color: rgb(247, 177, 79); }");
        styleSheet.append("margin-bottom: 2px;");
    } else if (style == Helper::ButtonSuccess) {
        styleSheet.append("QPushButton { color: #ffffff; background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(98, 196, 98, 255), stop:1 rgba(81, 163, 81, 255)); border: 1px solid #419641; }");
        styleSheet.append("QPushButton::pressed { background-color: rgb(81, 163, 81); }");
        styleSheet.append("QPushButton::disabled { background-color: rgb(142, 196, 142); }");
        styleSheet.append("margin-bottom: 2px;");
    } else {
        styleSheet = "";
    }
    button->setStyleSheet(styleSheet);
}

void Helper::fillStereoModeCombo(QComboBox *box)
{
    bool blocked = box->blockSignals(true);
    box->clear();
    box->addItem("", "");
    QMap<QString, QString> modes = stereoModes();
    QMapIterator<QString, QString> it(modes);
    while (it.hasNext()) {
        it.next();
        box->addItem(it.value(), it.key());
    }
    box->blockSignals(blocked);
}

QMap<QString, QString> Helper::stereoModes()
{
    QMap<QString, QString> modes;
    modes.insert("left_right", "side by side (left eye first)");
    modes.insert("bottom_top", "top-bottom (right eye first)");
    modes.insert("bottom_top", "top-bottom (left eye first)");
    modes.insert("checkerboard_rl", "checkboard (right eye first)");
    modes.insert("checkerboard_lr", "checkboard (left eye first)");
    modes.insert("row_interleaved_rl", "row interleaved (right eye first)");
    modes.insert("row_interleaved_lr", "row interleaved (left eye first)");
    modes.insert("col_interleaved_rl", "column interleaved (right eye first)");
    modes.insert("col_interleaved_lr", "column interleaved (left eye first)");
    modes.insert("anaglyph_cyan_red", "anaglyph (cyan/red)");
    modes.insert("right_left", "side by side (right eye first)");
    modes.insert("anaglyph_green_magenta", "anaglyph (green/magenta)");
    modes.insert("block_lr", "both eyes laced in one block (left eye first)");
    modes.insert("block_rl", "both eyes laced in one block (right eye first)");
    return modes;
}

QString Helper::matchResolution(int width, int height, const QString &scanType)
{
    QString res;
    if (height >= 1072 || width >= 1912)
        res = "1080";
    else if (height >= 712 || width >= 1272)
        res = "720";
    else if (height >= 576)
        res = "576";
    else if (height >= 540)
        res = "540";
    else if (height >= 480)
        res = "480";
    else
        return "SD";

    if (scanType.toLower() == "progressive")
        return res + "p";
    if (scanType.toLower() == "interlaced")
        return res + "i";
    return res;
}

QImage Helper::getImage(QString path)
{
    QImage img;
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        img = QImage::fromData(file.readAll());
        file.close();
    }
    return img;
}

QString Helper::secondsToTimeCode(quint32 duration)
{
    QString res;
    int seconds = (int) (duration % 60);
    duration /= 60;
    int minutes = (int) (duration % 60);
    duration /= 60;
    int hours = (int) (duration % 24);
    int days = (int) (duration / 24);
    if(hours == 0 && days == 0)
        return res.sprintf("%02d:%02d", minutes, seconds);
    if (days == 0)
        return res.sprintf("%02d:%02d:%02d", hours, minutes, seconds);
    return res.sprintf("%dd%02d:%02d:%02d", days, hours, minutes, seconds);
}
