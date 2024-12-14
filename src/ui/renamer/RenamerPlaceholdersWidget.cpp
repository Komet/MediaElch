#include "RenamerPlaceholdersWidget.h"
#include "ui_RenamerPlaceholdersWidget.h"

RenamerPlaceholdersWidget::RenamerPlaceholdersWidget(QWidget* parent) :
    QWidget(parent), ui(new Ui::RenamerPlaceholdersWidget)
{
    ui->setupUi(this);
    setType(RenameType::Movies);
}

RenamerPlaceholdersWidget::~RenamerPlaceholdersWidget()
{
    delete ui;
}

void RenamerPlaceholdersWidget::setType(RenameType renameType)
{
    using Type = RenameType;

    for (const auto label : ui->groupBox->findChildren<QLabel*>()) {
        const auto itemTypes = label->property("itemTypes").toStringList();
        if (itemTypes.isEmpty()) {
            continue;
        }
        label->setVisible((renameType == Type::Movies && itemTypes.contains("movie"))
                          || (renameType == Type::TvShows && itemTypes.contains("tvshow"))
                          || (renameType == Type::Concerts && itemTypes.contains("concert")));
    }
}
