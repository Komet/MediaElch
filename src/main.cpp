#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QObject>
#include <QTextCodec>
#include <QTextStream>
#include <QTimer>
#include <QTranslator>

#include "Version.h"
#include "log/Log.h"
#include "settings/Settings.h"
#include "ui/main/MainWindow.h"

static void initLogFile()
{
    if (!Settings::instance()->advanced()->debugLog()) {
        return;
    }
    const QString logFile = Settings::instance()->advanced()->logFile();
    bool success = mediaelch::openLogFile(logFile);
    if (success) {
        return;
    }
    QMessageBox::critical(nullptr,
        QObject::tr("Logfile could not be openened"),
        QObject::tr("The logfile %1 could not be openend for writing.").arg(logFile));
}

static void loadStylesheet(QApplication& app, const QString& customStylesheet)
{
    QString filename = customStylesheet.isEmpty() ? ":/src/ui/default.css" : customStylesheet;
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (!customStylesheet.isEmpty()) {
            qInfo() << "Using custom stylesheet from:" << customStylesheet;
        }
        app.setStyleSheet(file.readAll());
        file.close();

    } else {
        qCritical() << "The stylesheet could not be openend for reading:" << filename;
        const QString heading = QObject::tr("Stylesheet could not be opened!");
        const QString body = customStylesheet.isEmpty()
                                 ? QObject::tr("The default stylesheet could not be openend for reading.")
                                 : QObject::tr("The custom stylesheet could not be openend for reading. Using: %1")
                                       .arg(customStylesheet);
        QMessageBox::critical(nullptr, heading, body);
    }
}

static void installTranslations(const QLocale& locale)
{
    static QTranslator qtTranslator;
    static QTranslator mediaelchTranslator;

    // ------------------------------------------------------------------------
    // Qt localization

    // Note:
    // If compiled, this path will point to Qt's installation directory.
    // For MediaElch.app (if packaged as *.dmg), it will be MediaElch.app/Contents/translations
    QString qtSearchDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    const bool qtLoaded = qtTranslator.load(locale, "qt", "_", qtSearchDir, ".qm");
    if (qtLoaded) {
        QApplication::installTranslator(&qtTranslator);
    }

    // ------------------------------------------------------------------------
    // MediaElch localization.  Allow the usage of a "local" qm file that can
    // be used for testing.
    const auto localFileName = QStringLiteral("%1%2MediaElch_local.qm") //
                                   .arg(QCoreApplication::applicationDirPath(), QDir::separator());
    const QFileInfo fi{localFileName};
    bool i18nLoaded = false;
    if (fi.isFile()) {
        i18nLoaded = mediaelchTranslator.load(localFileName);
    } else {
        i18nLoaded = mediaelchTranslator.load(locale, "MediaElch", "_", ":/i18n/", ".qm");
    }

    if (i18nLoaded) {
        QApplication::installTranslator(&mediaelchTranslator);
    } else {
        qWarning() << "Could NOT find MediaElch's translations for " << locale;
    }
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    registerAllMetaTypes();

    QCoreApplication::setOrganizationName(mediaelch::constants::OrganizationName);
    QCoreApplication::setApplicationName(mediaelch::constants::AppName);
    QCoreApplication::setApplicationVersion(mediaelch::constants::AppVersionFullStr);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    mediaelch::initLoggingPattern();

    // Set parent of the singleton to core application
    Q_UNUSED(Settings::instance(QCoreApplication::instance()));

    // Install a message handler here to get "nice" output but instantiate the
    // logger after the translator is installed to avoid calls to tr() prior
    // to updating the application's language. "Settings::instance()" instantiates
    // "Manager" which instantiates all scrapers which themself add their settings
    // with translated values to the settings dialog.
    qInstallMessageHandler(mediaelch::messageHandler);

    // Qt's and MediaElch's translations.
    installTranslations(Settings::instance()->advanced()->locale());

    // Load the system's settings, e.g. window position, etc.
    Settings::instance()->loadSettings();

    initLogFile();
    loadStylesheet(app, Settings::instance()->advanced()->customStylesheet());

    MainWindow window;
    window.show();
    int ret = QApplication::exec();

    mediaelch::closeLogFile();

    return ret;
}
