#pragma once

#include "data/Locale.h"
#include "data/Poster.h"
#include "src/workers/Job.h"

#include <QPointer>
#include <QWidget>
#include <memory>

namespace Ui {
class ScrapePreview;
}

class ScrapePreview : public QWidget
{
    Q_OBJECT

public:
    struct JobAdapter
    {
        virtual ~JobAdapter() = default;

        ELCH_NODISCARD virtual mediaelch::worker::Job* scrapeJob() = 0;

        ELCH_NODISCARD virtual QString title() = 0;
        ELCH_NODISCARD virtual QString description() = 0;
        ELCH_NODISCARD virtual Poster poster() = 0;
    };

public:
    explicit ScrapePreview(QWidget* parent = nullptr);

    void load(std::unique_ptr<JobAdapter> jobAdapter);
    void clearAndAbortPreview();

private slots:
    void onScrapeJobFinished(mediaelch::worker::Job* scrapeJob);

private:
    Ui::ScrapePreview* ui = nullptr;
    std::unique_ptr<JobAdapter> m_currentAdapter = nullptr;
    QPixmap m_placeholderPoster;
};
