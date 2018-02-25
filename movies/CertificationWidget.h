#ifndef CERTIFICATIONWIDGET_H
#define CERTIFICATIONWIDGET_H

#include <QMenu>
#include <QSplitter>
#include <QTableWidgetItem>
#include <QWidget>
#include "globals/Globals.h"
#include "movies/Movie.h"

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
    explicit CertificationWidget(QWidget *parent = nullptr);
    ~CertificationWidget();

signals:
    void setActionSaveEnabled(bool, MainWidgets);
    void sigJumpToMovie(Movie*);

public slots:
    void onSaveInformation();
    void loadCertifications();
    QSplitter *splitter();

private slots:
    void addCertification();
    void deleteCertification();
    void onCertificationNameChanged(QTableWidgetItem *item);
    void onCertificationSelected();
    void addMovie();
    void removeMovie();
    void showCertificationsContextMenu(QPoint point);
    void onJumpToMovie(QTableWidgetItem *item);

private:
    Ui::CertificationWidget *ui;
    QMenu *m_tableContextMenu;
    QStringList m_addedCertifications;

    void clear();
};

#endif // CERTIFICATIONWIDGET_H
