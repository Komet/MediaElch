#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"

/**
 * @brief AboutDialog::AboutDialog
 * @param parent
 */
AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->labelMediaElch->setText(tr("MediaElch %1 - %2").arg(QApplication::applicationVersion()).arg("Talax"));

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    QPixmap p(":/img/MediaElch.png");
    p = p.scaled(ui->icon->size() * Helper::instance()->devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    Helper::instance()->setDevicePixelRatio(p, Helper::instance()->devicePixelRatio(this));
    ui->icon->setPixmap(p);
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
   foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows())
       episodes += show->episodes().count();

   int albums = 0;
   foreach (Artist *artist, Manager::instance()->musicModel()->artists())
       albums += artist->albums().count();

   ui->numMovies->setText(QString::number(Manager::instance()->movieModel()->movies().count()));
   ui->numConcerts->setText(QString::number(Manager::instance()->concertModel()->concerts().count()));
   ui->numShows->setText(QString::number(Manager::instance()->tvShowModel()->tvShows().count()));
   ui->numEpisodes->setText(QString::number(episodes));
   ui->numArtists->setText(QString::number(Manager::instance()->musicModel()->artists().count()));
   ui->numAlbums->setText(QString::number(albums));

   return QDialog::exec();
}
