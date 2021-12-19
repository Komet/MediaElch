#include "scrapers/ScraperUtils.h"

#include <QTextDocument>

QString removeHtmlEntities(QString str)
{
    QTextDocument doc;
    doc.setHtml(std::move(str));
    return doc.toPlainText().trimmed();
}
