#pragma once

#include <QWidget>

namespace Ui {
class ImageLabel;
}

class QMovie;

/**
 * \brief The ImageLabel class
 * A widget with an image and a text below
 */
class ImageLabel : public QWidget
{
    Q_OBJECT

public:
    explicit ImageLabel(QWidget* parent = nullptr);
    ~ImageLabel() override;
    void setImage(QPixmap img);
    void setHint(QSize resolution, QString hint = "");
    QImage image() const;

    void setLoading(bool isLoading);

private:
    Ui::ImageLabel* ui;
    QMovie* m_loadingSpinner = nullptr;
};
