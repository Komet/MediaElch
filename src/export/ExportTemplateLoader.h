#pragma once

#include <QBuffer>
#include <QObject>
#include <QXmlStreamReader>

#include "export/ExportTemplate.h"
#include "globals/Globals.h"
#include "network/NetworkManager.h"

/// Load remote and local templates and make them available.
class ExportTemplateLoader : public QObject
{
    Q_OBJECT
public:
    explicit ExportTemplateLoader(QObject* parent = nullptr);
    static ExportTemplateLoader* instance(QObject* parent = nullptr);
    QVector<ExportTemplate*> installedTemplates();
    ExportTemplate* getTemplateByIdentifier(QString identifier);

signals:
    void sigTemplatesLoaded(QVector<ExportTemplate*>);
    void sigTemplateInstalled(ExportTemplate*, bool);
    void sigTemplateUninstalled(ExportTemplate*, bool);

public slots:
    void getRemoteTemplates();
    void installTemplate(ExportTemplate* exportTemplate);
    bool uninstallTemplate(ExportTemplate* exportTemplate);

private slots:
    void onLoadRemoteTemplatesFinished();
    void onDownloadTemplateFinished();

private:
    mediaelch::network::NetworkManager m_network;
    QVector<ExportTemplate*> m_localTemplates;
    QVector<ExportTemplate*> m_remoteTemplates;

    void loadLocalTemplates();
    ExportTemplate* parseTemplate(QXmlStreamReader& xml);
    bool validateChecksum(const QByteArray& data, const ExportTemplate& exportTemplate);
    bool unpackTemplate(QBuffer& buffer, ExportTemplate* exportTemplate);
    bool removeDir(const QString& dirName);
    QVector<ExportTemplate*> mergeTemplates(QVector<ExportTemplate*> local, QVector<ExportTemplate*> remote);
};
