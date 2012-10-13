#ifndef CERTIFICATIONWIDGET_H
#define CERTIFICATIONWIDGET_H

#include <QContextMenuEvent>
#include <QMenu>
#include <QSplitter>
#include <QTableWidgetItem>
#include <QWidget>
#include "globals/Globals.h"

namespace Ui {
class CertificationWidget;
}

/**
 * @brief The CertificationWidget class
 */
class CertificationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CertificationWidget(QWidget *parent = 0);
    ~CertificationWidget();

signals:
    void setActionSaveEnabled(bool, MainWidgets);

public slots:
    void onSaveInformation();
    void loadCertifications();
    QSplitter *splitter();

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void deleteCertification();
    void onCertificationNameChanged(QTableWidgetItem *item);
    void onCertificationSelected();
    void addMovie();
    void removeMovie();

private:
    Ui::CertificationWidget *ui;
    QMenu *m_tableContextMenu;

    void clear();
};

#endif // CERTIFICATIONWIDGET_H
