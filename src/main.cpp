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
#include "settings/Settings.h"
#include "ui/main/MainWindow.h"

static QFile data;

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
#ifdef Q_OS_WIN
    const QString newLine = "\r\n";
#else
    const QString newLine = "\n";
#endif

    QTextStream out(stderr);
    if (data.isOpen()) {
        out.setDevice(&data);
    }

    const auto typeStr = [type]() -> QString {
        switch (type) {
        case QtInfoMsg: return QStringLiteral("INFO:     ");
        case QtDebugMsg: return QStringLiteral("DEBUG:    ");
        case QtWarningMsg: return QStringLiteral("WARNING:  ");
        case QtCriticalMsg: return QStringLiteral("CRITICAL: ");
        case QtFatalMsg: return QStringLiteral("FATAL:    ");
        }
        return QStringLiteral("UNKNOWN:  ");
    }();

#ifdef QT_DEBUG
    auto srcFile = QString("%1").arg(context.function, -70, QChar(' '));
    srcFile.truncate(70);
    out << "[" << srcFile << "] " << typeStr << msg.toLocal8Bit() << newLine;
#else
    Q_UNUSED(context);
    out << "[MediaElch] " << typeStr << msg.toLocal8Bit() << newLine;
#endif

    if (type == QtFatalMsg) {
        abort();
    }
}

void installLogger()
{
    if (Settings::instance()->advanced()->debugLog() && !Settings::instance()->advanced()->logFile().isEmpty()) {
        data.setFileName(Settings::instance()->advanced()->logFile());
        if (!data.open(QFile::WriteOnly | QFile::Truncate)) {
            QMessageBox::critical(nullptr,
                QObject::tr("Logfile could not be openened"),
                QObject::tr("The logfile %1 could not be openend for writing.")
                    .arg(Settings::instance()->advanced()->logFile()));
        }
    }
}

void loadStylesheet(QApplication& app)
{
    QFile file(":/src/ui/default.css");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(file.readAll());
        file.close();

    } else {
        qCritical() << "The default stylesheet could not be openend for reading.";
        QMessageBox::critical(nullptr,
            QObject::tr("Stylesheet could not be opened!"),
            QObject::tr("The default stylesheet could not be openend for reading."));
    }
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(mediaelch::constants::OrganizationName);
    QCoreApplication::setApplicationName(mediaelch::constants::AppName);
    QCoreApplication::setApplicationVersion(mediaelch::constants::AppVersionFullStr);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    // Install a message handler here to get "nice" output but instantiate the
    // logger after the translator is installed to avoid calls to tr() prior
    // to updating the application's language. "Settings::instance()" instantiates
    // "Manager" which instantiates all scrapers which themself add their settings
    // with translated values to the settings dialog.
    qInstallMessageHandler(messageHandler);

    // Qt localization
    QTranslator qtTranslator;
    qtTranslator.load(":/i18n/qt_" + Settings::instance()->advanced()->locale().name());
    QApplication::installTranslator(&qtTranslator);

    // MediaElch localization
    QTranslator editTranslator;
    const auto localFileName =
        QStringLiteral("%1%2MediaElch_local.qm").arg(QCoreApplication::applicationDirPath(), QDir::separator());
    const QFileInfo fi{localFileName};
    if (fi.isFile()) {
        editTranslator.load(localFileName);
    } else {
        editTranslator.load(Settings::instance()->advanced()->locale(), "MediaElch", "_", ":/i18n/", ".qm");
    }
    QApplication::installTranslator(&editTranslator);

    // Load the system's settings, e.g. window position, etc.
    Settings::instance(QCoreApplication::instance())->loadSettings();

    installLogger();
    loadStylesheet(app);

    MainWindow window;
    window.show();
    int ret = QApplication::exec();

    if (data.isOpen()) {
        data.close();
    }

    return ret;
}
