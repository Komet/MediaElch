#include "ui/settings/MovieScraperInfoWidget.h"
#include "globals/Helper.h"
#include "ui_MovieScraperInfoWidget.h"

#include <QListWidgetItem>

using namespace mediaelch;

MovieScraperInfoWidget::MovieScraperInfoWidget(mediaelch::scraper::MovieScraper& scraper, QWidget* parent) :
    QWidget(parent), ui(new Ui::MovieScraperInfoWidget), m_scraper{scraper}
{
    ui->setupUi(this);

    setupScraperDetails();
}

MovieScraperInfoWidget::~MovieScraperInfoWidget()
{
    delete ui;
}

void MovieScraperInfoWidget::setupScraperDetails()
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
