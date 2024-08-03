#include "Version.h"
#include "globals/Manager.h"
#include "log/Log.h"
#include "settings/Settings.h"
#include "ui/main/MainWindow.h"

#ifdef Q_OS_MAC
#    include "ui/MacUiUtilities.h"
#endif

#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QObject>
#include <QStyleFactory>
#include <QTextStream>
#include <QTimer>
#include <QTranslator>

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

static QString themeStylesheetName(const QString& theme, const QString& customStylesheet)
{
    const QStringList availableStyles = QStyleFactory::keys();
    QString filename;
    qCDebug(generic) << "Using theme:" << theme;

#ifdef Q_OS_MAC
    QString mainWindowTheme;
    if (theme == "auto") {
        mainWindowTheme = mediaelch::ui::macIsInDarkTheme() ? "dark" : "light";
    } else {
        mainWindowTheme = theme.startsWith("dark") ? "dark" : "light";
    }
#else
    // i.e. "auto" is also "light".
    QString mainWindowTheme = theme.startsWith("dark") ? "dark" : "light";
#endif

    if (!customStylesheet.isEmpty()) {
        filename = customStylesheet;
    } else {
        MediaElch_Assert(!mainWindowTheme.isEmpty());
        filename = QStringLiteral(":/src/ui/%1.css").arg(mainWindowTheme);
    }
    return filename;
}

static void loadStylesheet(QApplication& app, const QString& stylesheetFile, bool isCustomStylesheet)
{
    QFile file(stylesheetFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (isCustomStylesheet) {
            qCInfo(generic) << "Using custom stylesheet from:" << stylesheetFile;
        }
        app.setStyleSheet(file.readAll());
        file.close();

    } else {
        qCCritical(generic) << "[UI] The stylesheet could not be opened for reading:" << stylesheetFile;
        const QString heading = QObject::tr("Stylesheet could not be opened!");
        const QString body =
            !isCustomStylesheet
                ? QObject::tr("The default stylesheet could not be opened for reading.")
                : QObject::tr("The custom stylesheet could not be opened for reading. Using: %1").arg(stylesheetFile);
        QMessageBox::critical(nullptr, heading, body);
    }
}

class ThemeWatcher
{
public:
    explicit ThemeWatcher(QApplication& app) : m_app{app} {}

    /// \brief Initialize the main window's theme and subscribe to theme updates.
    void initTheme()
    {
        QString customStylesheet = Settings::instance()->advanced()->customStylesheet();
        m_stylesheetFile = themeStylesheetName(Settings::instance()->theme(), customStylesheet);
        loadStylesheet(m_app, m_stylesheetFile, !customStylesheet.isEmpty());
        if (!customStylesheet.isEmpty()) {
            return;
        }

        QApplication::connect(Settings::instance(), &Settings::sigSettingsSaved, &m_app, [this]() {
            QString newFile = themeStylesheetName(Settings::instance()->theme(), "");
            if (m_stylesheetFile != newFile) {
                m_stylesheetFile = newFile;
                loadStylesheet(m_app, m_stylesheetFile, false);
            }
        });
    }

private:
    QApplication& m_app;
    QString m_stylesheetFile;
};
static void installTranslations(const QLocale& locale)
{
    static QTranslator qtTranslator;
    static QTranslator mediaelchTranslator;

    // ------------------------------------------------------------------------
    // Qt localization

    // Note:
    // If compiled, this path will point to Qt's installation directory.
    // For MediaElch.app (if packaged as *.dmg), it will be MediaElch.app/Contents/translations
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QString qtSearchDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#else
    QString qtSearchDir = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#endif

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
        qCWarning(generic) << "Could NOT find MediaElch's translations for " << locale;
    }
}

int main(int argc, char* argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Default in Qt 6
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    // TODO: Test on Windows/Linux (not supported on macOS)
    // QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#endif

    QApplication app(argc, argv);
    registerAllMetaTypes();

    QCoreApplication::setOrganizationName(mediaelch::constants::OrganizationName);
    QCoreApplication::setApplicationName(mediaelch::constants::AppName);
    QCoreApplication::setApplicationVersion(mediaelch::constants::AppVersionFullStr);

    mediaelch::initLoggingPattern();

    // Set parent of the singleton to core application
    Q_UNUSED(Settings::instance(QCoreApplication::instance()));

    // Install a message handler here to get "nice" output but instantiate the
    // logger after the translator is installed to avoid calls to tr() prior
    // to updating the application's language. "Settings::instance()" instantiates
    // "Manager" which instantiates all scrapers which themselves add their settings
    // with translated values to the settings dialog.
    qInstallMessageHandler(mediaelch::messageHandler);

    // Qt's and MediaElch's translations.
    installTranslations(Settings::instance()->advanced()->locale());

    // Load the system's settings, e.g. window position, etc.
    Settings::instance()->loadSettings();

    initLogFile();

    ThemeWatcher w(app);
    w.initTheme();

    // Set the current font again. Workaround for #1502.
    QFont font = QApplication::font();
    QApplication::setFont(font);

    Manager::instance(); // initialize
    MainWindowConfiguration windowConfig(*Settings::instance());
    windowConfig.init();
    MainWindow window(windowConfig);
    window.show();
    int ret = QApplication::exec();

    mediaelch::closeLogFile();

    return ret;
}
