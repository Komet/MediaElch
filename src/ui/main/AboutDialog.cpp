#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include <QStandardPaths>

#include "Version.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"

/**
 * @brief AboutDialog::AboutDialog
 * @param parent
 */
AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent), ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->labelMediaElch->setText(QStringLiteral("MediaElch %1 - %2")
                                    .arg(QApplication::applicationVersion())
                                    .arg(mediaelch::constants::VersionName));

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    QPixmap p(":/img/MediaElch.png");
    p = p.scaled(ui->icon->size() * Helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    Helper::setDevicePixelRatio(p, Helper::devicePixelRatio(this));
    ui->icon->setPixmap(p);

    setDeveloperInformation();
}

/**
 * @brief AboutDialog::~AboutDialog
 */
AboutDialog::~AboutDialog()
{
    delete ui;
}

/**
 * @brief AboutDialog::exec
 * @return
 */
int AboutDialog::exec()
{
    adjustSize();

    int episodes = 0;
    for (TvShow* show : Manager::instance()->tvShowModel()->tvShows()) {
        episodes += show->episodes().count();
    }

    int albums = 0;
    for (Artist* artist : Manager::instance()->musicModel()->artists()) {
        albums += artist->albums().count();
    }

    ui->numMovies->setText(QString::number(Manager::instance()->movieModel()->movies().count()));
    ui->numConcerts->setText(QString::number(Manager::instance()->concertModel()->concerts().count()));
    ui->numShows->setText(QString::number(Manager::instance()->tvShowModel()->tvShows().count()));
    ui->numEpisodes->setText(QString::number(episodes));
    ui->numArtists->setText(QString::number(Manager::instance()->musicModel()->artists().count()));
    ui->numAlbums->setText(QString::number(albums));

    return QDialog::exec();
}

void AboutDialog::setDeveloperInformation()
{
    QString infos;
    QTextStream infoStream(&infos);
    infoStream << mediaelch::constants::AppName << " " << mediaelch::constants::AppVersionStr << " - "
               << mediaelch::constants::VersionName << "<br><br>"
               << QStringLiteral("Compiled with Qt version %1 (%2 %3, %4)<br>")
                      .arg(QT_VERSION_STR,
                          QString::number(QSysInfo::WordSize),
                          mediaelch::constants::CompilationType,
                          mediaelch::constants::CompilerString)
               << QStringLiteral("Using Qt version %1<br>").arg(qVersion())
               << QStringLiteral("System: %1 (%2)<br><br>").arg(QSysInfo::prettyProductName(), QSysInfo::buildAbi())
               << "Application dir: " << Settings::applicationDir() << "<br>"
               << "Settings file: " << Settings::instance()->settings()->fileName() << "<br>"
               << "Data dir: " << QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    ui->txtDetails->setHtml(infos);
}
