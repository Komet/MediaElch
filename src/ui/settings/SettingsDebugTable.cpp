#include "ui/settings/SettingsDebugTable.h"
#include "ui_SettingsDebugTable.h"

#include "settings/Settings.h"

#include <QTableWidgetItem>

SettingsDebugTable::SettingsDebugTable(Settings& settings, QWidget* parent) :
    QMainWindow(parent), ui(new Ui::SettingsDebugTable), m_settings{settings}
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    setupTable();
}

SettingsDebugTable::~SettingsDebugTable()
{
    delete ui;
}

void SettingsDebugTable::setupTable()
{
    const auto& items = m_settings.items();

    ui->settingsTable->setColumnCount(4);
    ui->settingsTable->setRowCount(items.size());
    ui->settingsTable->setSortingEnabled(true);

    ui->settingsTable->setHorizontalHeaderLabels({
        tr("Module"), //
        tr("Key"),
        tr("Value"),
        tr("Default Value"),
        tr("Description"),
    });

    QHeaderView* header = ui->settingsTable->horizontalHeader();
    header->setMinimumSectionSize(150);
    header->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::ResizeMode::Stretch);


    elch_ssize_t i = 0;
    for (const auto& item : items) {
        {
            QTableWidgetItem* newItem = new QTableWidgetItem(item.key.moduleName);
            newItem->setFlags(Qt::ItemFlag::ItemIsEnabled);
            ui->settingsTable->setItem(i, 0, newItem);
        }
        {
            QTableWidgetItem* newItem = new QTableWidgetItem(item.key.key);
            newItem->setFlags(Qt::ItemFlag::ItemIsEnabled);
            ui->settingsTable->setItem(i, 1, newItem);
        }
        {
            QTableWidgetItem* newItem = new QTableWidgetItem(item.value.toString());
            newItem->setFlags(Qt::ItemFlag::ItemIsEnabled);
            ui->settingsTable->setItem(i, 2, newItem);
            m_settings.onSettingChanged(item.key, this, [newItem, this, key = item.key]() {
                newItem->setText(m_settings.value(key).toString());
            });
        }
        {
            QTableWidgetItem* newItem = new QTableWidgetItem(item.defaultValue.toString());
            newItem->setFlags(Qt::ItemFlag::ItemIsEnabled);
            ui->settingsTable->setItem(i, 3, newItem);
        }
        {
            QTableWidgetItem* newItem = new QTableWidgetItem(item.description);
            newItem->setFlags(Qt::ItemFlag::ItemIsEnabled);
            ui->settingsTable->setItem(i, 4, newItem);
        }
        ++i;
    }
}
