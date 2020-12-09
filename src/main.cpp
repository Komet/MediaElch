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

static void setupTranslation(const QString& filename)
{
    auto* qtTranslator = new QTranslator(QCoreApplication::instance());
    // advanced settings are already loaded in Setting's constructor.
    QLocale locale = Settings::instance()->advanced()->locale();
    if (qtTranslator->load(locale, filename, QLatin1String("_"), QLatin1String(":/i18n"), QLatin1String(".qm"))) {
        QApplication::installTranslator(qtTranslator);
    } else if (filename != "qt") {
        // Only warn if "MediaElch" translation could not be loaded.
        qWarning() << "[i18n] Could not load " << filename << "translation file for" << locale << locale.uiLanguages();
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

    // MediaElch localization
    setupTranslation(QLatin1String("qt"));
    setupTranslation(QLatin1String("MediaElch"));

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
