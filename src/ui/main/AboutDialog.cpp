#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include "Version.h"
#include "globals/Manager.h"

#include "MediaInfoDLL/MediaInfoDLL.h"
#include "media/MediaInfoFile.h"

#include <QClipboard>
#include <QLibraryInfo>
#include <QStandardPaths>

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent), ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    setAttribute(Qt::WA_DeleteOnClose);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &AboutDialog::close);
    connect(ui->copyToClipboard, &QPushButton::clicked, this, &AboutDialog::copyToClipboard);

    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    ui->labelMediaElch->setText(QStringLiteral("MediaElch %1 - %2")
            .arg(QApplication::applicationVersion())
            .arg(mediaelch::constants::VersionName));

    QPixmap p(":/img/MediaElch.png");
    p = p.scaled(ui->icon->size() * devicePixelRatioF(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p.setDevicePixelRatio(devicePixelRatioF());
    ui->icon->setPixmap(p);

    setLibraryDetails();
    setDeveloperDetails();
    adjustSize();

    connect(ui->btnAboutQt, &QPushButton::clicked, qApp, &QApplication::aboutQt);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::setDeveloperDetails()
{
    const auto* infoVersionStr = __T("Info_Version");
    // format: MediaInfoLib - v20.03
    auto mediaInfoVersion = MediaInfoDLL::MediaInfo::Option_Static(MediaInfoDLL::String(infoVersionStr));

    QString infos;
    QTextStream infoStream(&infos);
    infoStream << mediaelch::constants::AppName << " " << mediaelch::constants::AppVersionFullStr << " - "
               << mediaelch::constants::VersionName << "<br><br>";

    // Compilation Details
    infoStream << QStringLiteral("Compiled with Qt version %1 (%2 %3, %4)<br>")
                      .arg(QT_VERSION_STR,
                          QString::number(QSysInfo::WordSize),
                          mediaelch::constants::CompilationType,
                          mediaelch::constants::CompilerString)
               << QStringLiteral("Using Qt version %1<br>").arg(qVersion())
               << QStringLiteral("System: %1 (%2)").arg(QSysInfo::prettyProductName(), QSysInfo::buildAbi())
               << "<br><br>";

    // Directories
    infoStream << "Application directory: " << QDir::toNativeSeparators(Settings::applicationDir()) << "<br>"
               << "Settings file: " << Settings::instance()->settings()->fileName() << "<br>"
               << "Data dir: "
               << QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation))
               << "<br>"
               << "Image cache directory: " << Settings::instance()->imageCacheDir().toNativePathString() << "<br>"
               << "Qt Translation Path: "

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
               << QDir::toNativeSeparators(QLibraryInfo::location(QLibraryInfo::TranslationsPath))
#else
               << QDir::toNativeSeparators(QLibraryInfo::path(QLibraryInfo::TranslationsPath))
#endif
               << "<br><br>";

    // Dependencies
    infoStream << "MediaInfo Version: ";
    infoStream << (mediaInfoVersion.empty() ? "&lt;not available&gt;" : MI2QString(mediaInfoVersion));
    infoStream << "<br><br>";
    infoStream << "Default UI languages: " << QLocale().uiLanguages().join(", ");
    infoStream << "<br><br>";
    infoStream << "Custom advancedsettings.xml: "
               << (Settings::instance()->advanced()->isUserDefined() ? "true" : "false");
    infoStream << "<br>";

    ui->txtDetails->setHtml(infos);
}

void AboutDialog::copyToClipboard()
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(ui->txtDetails->toPlainText());
}

void AboutDialog::setLibraryDetails()
{
    elch_ssize_t episodes = 0;
    for (TvShow* show : Manager::instance()->tvShowModel()->tvShows()) {
        episodes += show->episodes().count();
    }

    elch_ssize_t albums = 0;
    for (Artist* artist : Manager::instance()->musicModel()->artists()) {
        albums += artist->albums().count();
    }

    ui->numMovies->setText(QString::number(Manager::instance()->movieModel()->movies().count()));
    ui->numConcerts->setText(QString::number(Manager::instance()->concertModel()->concerts().count()));
    ui->numShows->setText(QString::number(Manager::instance()->tvShowModel()->tvShows().count()));
    ui->numEpisodes->setText(QString::number(episodes));
    ui->numArtists->setText(QString::number(Manager::instance()->musicModel()->artists().count()));
    ui->numAlbums->setText(QString::number(albums));
}
