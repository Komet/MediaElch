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
#include "main/MainWindow.h"
#include "settings/Settings.h"

static QFile data;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
#ifdef Q_OS_WIN32
    const QString newLine = "\r\n";
#else
    const QString newLine = "\n";
#endif

    QTextStream out(stderr);
    if (data.isOpen()) {
        out.setDevice(&data);
    }

    const auto srcFile = QString("%1").arg(context.function, -70, QChar(' '));
    const auto typeStr = [type]() -> QString {
        switch (type) {
        case QtInfoMsg: return QStringLiteral("INFO:     ");
        case QtDebugMsg: return QStringLiteral("DEBUG:    ");
        case QtWarningMsg: return QStringLiteral("WARNING:  ");
        case QtCriticalMsg: return QStringLiteral("CRITICAL: ");
        case QtFatalMsg: return QStringLiteral("FATAL:    ");
        default: return QStringLiteral("UNKNOWN:  ");
        }
    }();

#ifdef QT_DEBUG
    out << "[" << srcFile << "] " << typeStr << msg.toLocal8Bit() << newLine;
#else
    out << "[MediaElch] " << typeStr << msg.toLocal8Bit() << newLine;
#endif

    if (type == QtFatalMsg) {
        abort();
    }
}

void installLogger()
{
    Settings::instance(QCoreApplication::instance())->loadSettings();
    if (Settings::instance()->advanced()->debugLog() && !Settings::instance()->advanced()->logFile().isEmpty()) {
        data.setFileName(Settings::instance()->advanced()->logFile());
        if (!data.open(QFile::WriteOnly | QFile::Truncate)) {
            QMessageBox::critical(nullptr,
                QObject::tr("Logfile could not be openened"),
                QObject::tr("The logfile %1 could not be openend for writing.")
                    .arg(Settings::instance()->advanced()->logFile()));
        }
    }
    qInstallMessageHandler(messageHandler);
}

void loadStylesheet(QApplication &app)
{
    QFile file(":/ui/default.css");
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

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(MediaElch::Constants::OrganizationName);
//    QCoreApplication::setOrganizationDomain(MediaElch::Constants::OrganizationDomain);
    QCoreApplication::setApplicationName(MediaElch::Constants::AppName);
    QCoreApplication::setApplicationVersion(MediaElch::Constants::AppVersionStr);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    installLogger();

    // Qt localization
    QTranslator qtTranslator;
    qtTranslator.load(":/i18n/qt_" + Settings::instance()->advanced()->locale().name());
    app.installTranslator(&qtTranslator);

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
    app.installTranslator(&editTranslator);

    loadStylesheet(app);

    MainWindow window;
    window.show();
    int ret = app.exec();

    if (data.isOpen()) {
        data.close();
    }

    return ret;
}
