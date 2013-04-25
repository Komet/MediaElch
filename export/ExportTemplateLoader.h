#ifndef EXPORTTEMPLATELOADER_H
#define EXPORTTEMPLATELOADER_H

#include <QBuffer>
#include <QNetworkAccessManager>
#include <QObject>
#include <QXmlStreamReader>

#include "export/ExportTemplate.h"
#include "globals/Globals.h"

class ExportTemplateLoader : public QObject
{
    Q_OBJECT
public:
    explicit ExportTemplateLoader(QObject *parent = 0);
    static ExportTemplateLoader *instance(QObject *parent = 0);
    QList<ExportTemplate*> installedTemplates();

signals:
    void sigTemplatesLoaded(QList<ExportTemplate*>);
    void sigTemplateInstalled(ExportTemplate*, bool);

public slots:
    void getRemoteTemplates();
    void installTemplate(ExportTemplate *exportTemplate);
    bool uninstallTemplate(ExportTemplate *exportTemplate);

private slots:
    void onLoadRemoteTemplatesFinished();
    void onDownloadTemplateFinished();

private:
    QNetworkAccessManager m_qnam;
    QList<ExportTemplate*> m_localTemplates;
    QList<ExportTemplate*> m_remoteTemplates;
    QNetworkAccessManager *qnam();
    void loadLocalTemplates();
    ExportTemplate *parseTemplate(QXmlStreamReader &xml);
    bool unpackTemplate(QBuffer &buffer, ExportTemplate *exportTemplate);
    bool removeDir(const QString &dirName);
    QList<ExportTemplate*> mergeTemplates(QList<ExportTemplate*> local, QList<ExportTemplate*> remote);
};

#endif // EXPORTTEMPLATELOADER_H
