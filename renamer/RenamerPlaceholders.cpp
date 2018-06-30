#include "RenamerPlaceholders.h"
#include "ui_RenamerPlaceholders.h"

#include "Renamer.h"

RenamerPlaceholders::RenamerPlaceholders(QWidget *parent) : QWidget(parent), ui(new Ui::RenamerPlaceholders)
{
    ui->setupUi(this);
}

RenamerPlaceholders::~RenamerPlaceholders()
{
    delete ui;
}

void RenamerPlaceholders::setType(int renameType)
{
    foreach (QLabel *label, ui->groupBox->findChildren<QLabel *>()) {
        if (label->property("itemTypes").toStringList().isEmpty()) {
            continue;
        }
        label->setVisible(
            (renameType == Renamer::TypeMovies && label->property("itemTypes").toStringList().contains("movie"))
            || (renameType == Renamer::TypeTvShows && label->property("itemTypes").toStringList().contains("tvshow"))
            || (renameType == Renamer::TypeConcerts
                   && label->property("itemTypes").toStringList().contains("concert")));
    }
}
