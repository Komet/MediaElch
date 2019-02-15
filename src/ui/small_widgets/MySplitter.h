#pragma once

#include <QSplitter>

class MySplitter : public QSplitter
{
    Q_OBJECT
public:
    explicit MySplitter(QWidget* parent = nullptr);

protected:
    QSplitterHandle* createHandle() override;
};
