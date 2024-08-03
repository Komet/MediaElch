#include "ui/settings/TvScraperInfoWidget.h"
#include "globals/Helper.h"
#include "ui_TvScraperInfoWidget.h"

#include <QListWidgetItem>

using namespace mediaelch;

TvScraperInfoWidget::TvScraperInfoWidget(mediaelch::scraper::TvScraper& scraper, QWidget* parent) :
    QWidget(parent), ui(new Ui::TvScraperInfoWidget), m_scraper{scraper}
{
    ui->setupUi(this);

    setupScraperDetails();
}

TvScraperInfoWidget::~TvScraperInfoWidget()
{
    delete ui;
}

void TvScraperInfoWidget::setupScraperDetails()
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
