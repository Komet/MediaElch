#include "ui/settings/ConcertScraperInfoWidget.h"
#include "globals/Helper.h"
#include "ui_ConcertScraperInfoWidget.h"

#include <QListWidgetItem>

using namespace mediaelch;

ConcertScraperInfoWidget::ConcertScraperInfoWidget(mediaelch::scraper::ConcertScraper& scraper, QWidget* parent) :
    QWidget(parent), ui(new Ui::ConcertScraperInfoWidget), m_scraper{scraper}
{
    ui->setupUi(this);

    setupScraperDetails();
}

ConcertScraperInfoWidget::~ConcertScraperInfoWidget()
{
    delete ui;
}

void ConcertScraperInfoWidget::setupScraperDetails()
{
    const auto& meta = m_scraper.meta();

    ui->txtName->setText(meta.name);
    ui->txtId->setText(meta.identifier);
    ui->txtDescription->setPlainText(meta.description);
    ui->txtWebsite->setText(helper::makeHtmlLink(meta.website));
    ui->txtTOS->setText(helper::makeHtmlLink(meta.termsOfService));
    ui->txtPrivacyPolicy->setText(helper::makeHtmlLink(meta.privacyPolicy));
    ui->txtHelp->setText(helper::makeHtmlLink(meta.help));
}
