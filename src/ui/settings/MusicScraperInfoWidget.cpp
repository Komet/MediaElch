#include "ui/settings/MusicScraperInfoWidget.h"
#include "globals/Helper.h"
#include "ui_MusicScraperInfoWidget.h"

#include <QListWidgetItem>

using namespace mediaelch;

MusicScraperInfoWidget::MusicScraperInfoWidget(mediaelch::scraper::MusicScraper& scraper, QWidget* parent) :
    QWidget(parent), ui(new Ui::MusicScraperInfoWidget), m_scraper{scraper}
{
    ui->setupUi(this);

    setupScraperDetails();
}

MusicScraperInfoWidget::~MusicScraperInfoWidget()
{
    delete ui;
}

void MusicScraperInfoWidget::setupScraperDetails()
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
