#include "RenamerPlaceholdersWidget.h"

#include "ui_RenamerPlaceholdersWidget.h"

#include <QLabel>

namespace {
void clearLayout(QLayout* layout)
{
    if (layout == nullptr) {
        return;
    }

    QLayoutItem* item = layout->takeAt(0);
    while (item != nullptr) {
        if (item->layout()) {
            clearLayout(item->layout());
            delete item->layout();
        }
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
        item = layout->takeAt(0);
    }
}
} // namespace

RenamerPlaceholdersWidget::RenamerPlaceholdersWidget(QWidget* parent) :
    QWidget(parent), ui(new Ui::RenamerPlaceholdersWidget)
{
    ui->setupUi(this);
}

RenamerPlaceholdersWidget::~RenamerPlaceholdersWidget()
{
    delete ui;
}

void RenamerPlaceholdersWidget::setPlaceholders(mediaelch::RenamerPlaceholders& renamerPlaceholders)
{
    clearLayout(ui->placeholderLayout);
    const QVector<mediaelch::Placeholder> placeholders = renamerPlaceholders.placeholders();
    for (const auto& placeholder : placeholders) {
        ui->placeholderLayout->addWidget(new QLabel(placeholder.placeholderText()));
        ui->placeholderLayout->addWidget(new QLabel(placeholder.translation));
    }
}