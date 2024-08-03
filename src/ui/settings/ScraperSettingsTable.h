#pragma once

#include "data/Locale.h"
#include "scrapers/ScraperInfos.h"
#include "ui/scrapers/ScraperManager.h"
#include "utils/Meta.h"

#include <QListWidgetItem>
#include <QString>
#include <QWidget>

namespace Ui {
class ScraperSettingsTable;
}

namespace mediaelch {
namespace scraper {

class SettingsTableData
{
public:
    virtual ~SettingsTableData() = default;

    ELCH_NODISCARD virtual QString scraperName() const = 0;
    ELCH_NODISCARD virtual QWidget* createScraperInfoWidget() const = 0;
    ELCH_NODISCARD virtual QWidget* createScraperSettingsWidget() const = 0;
    ELCH_NODISCARD virtual bool isAdultScraper() const = 0;
};

} // namespace scraper
} // namespace mediaelch


class ScraperSettingsTable : public QWidget
{
    Q_OBJECT

public:
    explicit ScraperSettingsTable(QWidget* parent = nullptr);
    ~ScraperSettingsTable() override;

    void addScraper(const mediaelch::scraper::SettingsTableData& tableData);
    void addDelimiter(const QString& title, const QString& description);
    void clear();

    ELCH_NODISCARD bool showAdultScraper();
    void setShowAdultScraper(bool enabled);

signals:
    void sigShowAdultScraper(bool enabled);

private slots:
    void onSelectedScraperChanges(QListWidgetItem* current, QListWidgetItem* prev);
    void onShowAdultScrapersChanged(bool enabled);


private:
    Ui::ScraperSettingsTable* ui = nullptr;
};
