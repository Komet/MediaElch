#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include "globals/Globals.h"
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
    ui->labelMediaElch->setText(tr("MediaElch %1 - %2").arg(QApplication::applicationVersion()).arg("Qo'noS"));

#ifdef Q_WS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif
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
   foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows()) {
       episodes += show->episodes().count();
   }

   ui->numMovies->setText(QString::number(Manager::instance()->movieModel()->movies().count()));
   ui->numConcerts->setText(QString::number(Manager::instance()->concertModel()->concerts().count()));
   ui->numShows->setText(QString::number(Manager::instance()->tvShowModel()->tvShows().count()));
   ui->numEpisodes->setText(QString::number(episodes));

   return QDialog::exec();
}
