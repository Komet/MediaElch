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

    out << "[" << srcFile << "] " << typeStr << msg.toLocal8Bit() << newLine;

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

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("kvibes");
    QCoreApplication::setApplicationName("MediaElch");
    QCoreApplication::setApplicationVersion("2.4.3-dev");
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

    MainWindow window;
    window.show();
    int ret = app.exec();

    if (data.isOpen()) {
        data.close();
    }

    return ret;
}
