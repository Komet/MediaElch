#ifndef CERTIFICATIONWIDGET_H
#define CERTIFICATIONWIDGET_H

#include <QWidget>

#include "globals/Globals.h"

namespace Ui {
class CertificationWidget;
}

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

private:
    Ui::CertificationWidget *ui;
    void clear();
};

#endif // CERTIFICATIONWIDGET_H
