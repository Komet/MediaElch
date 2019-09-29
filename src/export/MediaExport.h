#pragma once

#include "export/ExportTemplate.h"

#include <QDir>
#include <QObject>
#include <atomic>

namespace mediaelch {

class MediaExport : public QObject
{
    Q_OBJECT
public:
    /// Export class for MediaElch. Uses the correct export engine depending
    /// on the template.
    /// If cancelFlag becomes true at any point, the export is canceled.
    /// No cleanup is performed.
    explicit MediaExport(std::atomic_bool& cancelFlag, QObject* parent = nullptr);

signals:
    /// Signal is emitted each time an item is exported (e.g. image, generated HTML, etc.)
    /// Useful for progress bars.
    void sigItemExported();

public:
    /// Export the user's media library to the given directory. Only export the given sections.
    void
    doExport(ExportTemplate& exportTemplate, QDir directory, const QVector<ExportTemplate::ExportSection>& sections);

private:
    std::atomic_bool& m_canceled;
};

} // namespace mediaelch
