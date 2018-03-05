#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QWidget>

namespace Ui {
class ImageLabel;
}

/**
 * @brief The ImageLabel class
 * A widget with an image and a text below
 */
class ImageLabel : public QWidget
{
    Q_OBJECT

public:
    explicit ImageLabel(QWidget *parent = nullptr);
    ~ImageLabel() override;
    void setImage(QPixmap img);
    void setHint(QSize resolution, QString hint = "");
    QImage image() const;

private:
    Ui::ImageLabel *ui;
};

#endif // IMAGELABEL_H
