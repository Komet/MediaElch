#pragma once

#include "scrapers/image/ImageProvider.h"

#include <QWidget>

namespace Ui {
class ImageProviderInfoWidget;
}

class ImageProviderInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageProviderInfoWidget(mediaelch::scraper::ImageProvider& scraper, QWidget* parent = nullptr);
    ~ImageProviderInfoWidget() override;

private:
    void setupScraperDetails();

private:
    Ui::ImageProviderInfoWidget* ui = nullptr;
    mediaelch::scraper::ImageProvider& m_scraper;
};
