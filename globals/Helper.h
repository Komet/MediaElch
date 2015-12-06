#ifndef HELPER_H
#define HELPER_H

#include <QComboBox>
#include <QImage>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QString>

/**
 * @brief Some convenience functions are bundled here
 */
class Helper : public QObject
{
public:
    enum ButtonStyle {
        ButtonPrimary, ButtonInfo, ButtonDanger, ButtonSuccess, ButtonWarning
    };

    Helper(QObject *parent = 0);
    static Helper *instance(QObject *parent = 0);
    virtual QString toLatin1PercentEncoding(QString str);
    virtual QString urlDecode(QString str);
    virtual QString urlEncode(QString str);
    virtual QString formatTrailerUrl(QString url);
    virtual bool isDvd(QString path, bool noSubFolder = false);
    virtual bool isBluRay(QString path);
    virtual QImage &resizeBackdrop(QImage &image, bool &resized);
    virtual QByteArray &resizeBackdrop(QByteArray &image);
    virtual QString &sanitizeFileName(QString &fileName);
    virtual QString stackedBaseName(const QString &fileName);
    virtual QString appendArticle(const QString &text);
    virtual QString mapGenre(const QString &text);
    virtual QStringList mapGenre(const QStringList &genres);
    virtual QString mapCertification(const QString &text);
    virtual QString mapStudio(const QString &text);
    virtual QString mapCountry(const QString &text);
    virtual QString formatFileSize(const qint64 &size);
    virtual void removeFocusRect(QWidget *widget);
    virtual void applyStyle(QWidget *widget, bool removeFocusRect = true, bool isTable = false);
    virtual void applyEffect(QWidget *parent);
    virtual qreal similarity(const QString &s1, const QString &s2);
    virtual QMap<int, QString> labels();
    virtual QColor colorForLabel(int label);
    virtual QIcon iconForLabel(int label);
    virtual qreal devicePixelRatio(QLabel *label);
    virtual qreal devicePixelRatio(QPushButton *button);
    virtual qreal devicePixelRatio(QWidget *widget);
    virtual qreal devicePixelRatio(const QPixmap &pixmap);
    virtual void setDevicePixelRatio(QPixmap &pixmap, qreal devicePixelRatio);
    virtual void setDevicePixelRatio(QImage &image, qreal devicePixelRatio);
    virtual int compareVersionNumbers(const QString &oldVersion, const QString &newVersion);
    virtual void setButtonStyle(QPushButton *button, Helper::ButtonStyle style);
    virtual void fillStereoModeCombo(QComboBox *box);
    virtual QMap<QString, QString> stereoModes();
    virtual QString matchResolution(int width, int height, const QString &scanType);
    virtual QImage getImage(QString path);
    virtual QString secondsToTimeCode(quint32 duration);
};

#endif // HELPER_H
