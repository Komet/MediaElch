#pragma once

#include <QString>
#include <QWidget>

namespace Ui {
class UnpackButtons;
}

class UnpackButtons : public QWidget
{
    Q_OBJECT

public:
    explicit UnpackButtons(QWidget* parent = nullptr);
    ~UnpackButtons() override;
    void setBaseName(QString baseName);
    QString baseName() const;
    void setShowProgress(bool showProgress);
    void setProgress(int progress);

signals:
    void sigUnpack(QString, QString);
    void sigStop(QString);
    void sigDelete(QString);

private slots:
    void onUnpack();
    void onUnpackWithPassword();
    void onStop();
    void onDelete();

private:
    Ui::UnpackButtons* ui;
    QString m_baseName;
};
