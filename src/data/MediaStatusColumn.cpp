#include "data/MediaStatusColumn.h"

#include "log/Log.h"

#include <QPainter>

#ifndef QT_NO_DEBUG
#    include <QFile>
#endif

QIcon iconWithMediaStatusColor(const QString& iconName, MediaStatusState state)
{
    // These colors are based on the old media status icons.
    // For example:
    // https://github.com/Komet/MediaElch/blob/fd6172a88c7bda946bc3fc827a2e1f6a5a374a39/data/img/mediaStatus/actors_green.png
    QColor highlight;
    switch (state) {
    case MediaStatusState::GREEN: highlight.setRgb(91, 183, 91); break;
    case MediaStatusState::RED: highlight.setRgb(216, 82, 79); break;
    case MediaStatusState::YELLOW: highlight.setRgb(239, 173, 77); break;
    }

    // Note: The ending ".svg" is required. Otherwise, Qt does not scale the icon
    //       for High DPI use!
    const QString file = QStringLiteral(":/icons/actions/16/%1.svg").arg(iconName);
    QIcon i;
    i.addFile(file);

#ifndef QT_NO_DEBUG
    // For debugging purposes: Check if the icon can be found.
    if (!QFile::exists(file)) {
        qCCritical(generic) << "[MovieModel] Missing icon for:" << iconName;
    }
#endif

    // Sizes match the ones in MovieFilesWidget
    QPixmap pixmap = i.pixmap(16, 16);

    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.setBrush(highlight);
    painter.setPen(highlight);
    painter.drawRect(pixmap.rect());

    return QIcon(pixmap);
}
