#pragma once

#include "data/concert/Concert.h"
#include "ui/renamer/RenamerDialog.h"

#include <QDialog>

class ConcertRenamerDialog : public RenamerDialog
{
    Q_OBJECT

public:
    explicit ConcertRenamerDialog(QWidget* parent = nullptr);
    ~ConcertRenamerDialog() override;

    void setConcerts(QVector<Concert*> concerts);

private:
    void renameType(bool isDryRun) override;
    void rejectImpl() override;
    QString dialogInfoLabel() override;
    void renameConcerts(QVector<Concert*> concerts, const RenamerConfig& config);

private:
    QVector<Concert*> m_concerts;
};
