#pragma once

#include "data/Certification.h"
#include "globals/Globals.h"

#include <QMenu>
#include <QSplitter>
#include <QTableWidgetItem>
#include <QWidget>

namespace Ui {
class CertificationWidget;
}

class Movie;

/**
 * @brief The CertificationWidget class
 */
class CertificationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CertificationWidget(QWidget* parent = nullptr);
    ~CertificationWidget() override;

signals:
    void setActionSaveEnabled(bool, MainWidgets);
    void sigJumpToMovie(Movie*);

public slots:
    void onSaveInformation();
    void loadCertifications();
    QSplitter* splitter();

private slots:
    void addCertification();
    void deleteCertification();
    void onCertificationNameChanged(QTableWidgetItem* item);
    void onCertificationSelected();
    void addMovie();
    void removeMovie();
    void showCertificationsContextMenu(QPoint point);
    void onJumpToMovie(QTableWidgetItem* item);

private:
    Ui::CertificationWidget* ui;
    QMenu* m_tableContextMenu;
    QVector<Certification> m_addedCertifications;

    void clear();
};
