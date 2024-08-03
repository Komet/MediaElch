#include "ui/settings/ImageProviderInfoWidget.h"
#include "globals/Helper.h"
#include "ui_ImageProviderInfoWidget.h"

#include <QListWidgetItem>

using namespace mediaelch;

ImageProviderInfoWidget::ImageProviderInfoWidget(mediaelch::scraper::ImageProvider& scraper, QWidget* parent) :
    QWidget(parent), ui(new Ui::ImageProviderInfoWidget), m_scraper{scraper}
{
    ui->setupUi(this);

    setupScraperDetails();
}

ImageProviderInfoWidget::~ImageProviderInfoWidget()
{
    delete ui;
}

void ImageProviderInfoWidget::setupScraperDetails()
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
