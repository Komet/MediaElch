#pragma once

#include "globals/Globals.h"
#include "globals/ScraperInfos.h"
#include "globals/ScraperResult.h"
#include "scrapers/ScraperInterface.h"

#include <QTableWidgetItem>
#include <QWidget>

namespace Ui {
class ConcertSearchWidget;
}

namespace mediaelch {
namespace scraper {
class ConcertSearchJob;
class ConcertScraper;
} // namespace scraper
} // namespace mediaelch

class ConcertSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConcertSearchWidget(QWidget* parent = nullptr);
    ~ConcertSearchWidget() override;

    QString concertIdentifier() const;
    mediaelch::scraper::ConcertScraper* scraper();

    const mediaelch::Locale& locale() const;
    const QSet<ConcertScraperInfo>& concertDetailsToLoad() const;

public slots:
    void search(QString searchString);

signals:
    void sigResultClicked();

private slots:
    /// \brief Initializes the current scraper if necessary and start the concert search.
    void initializeAndStartSearch();
    /// \brief Starts the concert search with the selected _initialized_ scraper.
    void startSearch();
    void onConcertResults(mediaelch::scraper::ConcertSearchJob* searchJob);
    /// \brief Stores the clicked id and accepts the dialog.
    /// \param item Item which was clicked
    void onResultClicked(QTableWidgetItem* item);
    void onConcertInfoToggled();
    void onChkAllConcertInfosToggled();
    void onScraperChanged(int index);
    void onLanguageChanged();

private:
    void setupScraperDropdown();
    void setupLanguageDropdown();
    void showError(const QString& message);
    void showSuccess(const QString& message);
    void clearResultTable();
    void updateCheckBoxes();

private:
    Ui::ConcertSearchWidget* ui = nullptr;

    QString m_concertIdentifier;
    QSet<ConcertScraperInfo> m_concertDetailsToLoad;

    mediaelch::scraper::ConcertScraper* m_currentScraper = nullptr;
    mediaelch::Locale m_currentLanguage = mediaelch::Locale::English;
};
