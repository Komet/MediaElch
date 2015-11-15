#ifndef MYICONFONT_H
#define MYICONFONT_H

/**
 * This is all taken from the nice QtAwesome class which can be found at
 * https://github.com/gamecreature/QtAwesome/
 *
 * It has been renamed and adjusted to match another font than FontAwesome
 */

#include <QIcon>
#include <QIconEngine>
#include <QPainter>
#include <QRect>
#include <QVariantMap>

/// A list of all icon-names with the codepoint (unicode-value) on the right
/// You can use the names on the page  http://fortawesome.github.io/Font-Awesome/design.html  ( replace every dash '-' with an underscore '_')
enum MyIconFontName {
    icon_film = 0xe6a5,
    icon_monitor = 0xe634,
    icon_micro = 0xe635,
    icon_music = 0xe630,
    icon_key = 0xe6a3,
    icon_photo_gallery = 0xe626,
    icon_notebook = 0xe62b,
    icon_download = 0xe65c,
    icon_cloud_download = 0xe68b,
    icon_diskette = 0xe65f,
    icon_cloud_upload = 0xe68a,
    icon_info = 0xe647,
    icon_refresh = 0xe61c,
    icon_folder = 0xe653,
    icon_tools = 0xe60a,
    icon_network = 0xe69e,
    icon_world = 0xe691,
    icon_box2 = 0xe673,
    icon_search = 0xe618,
    icon_angle_up = 0xe682,
    icon_angle_up_circle = 0xe683,
    icon_angle_right = 0xe684,
    icon_angle_right_circle = 0xe685,
    icon_angle_left = 0xe686,
    icon_angle_left_circle = 0xe687,
    icon_angle_down = 0xe688,
    icon_angle_down_circle = 0xe689,
    icon_star = 0xe611,
    icon_plus = 0xe623,
    icon_close = 0xe680,
    icon_play = 0xe624,
    icon_note = 0xe62c,
    icon_pen = 0xe628,
    icon_attention = 0xe67b,
    icon_airplay = 0xe67f,
    icon_refresh_cloud = 0xe61d,
    icon_repeat = 0xe61b
};


//---------------------------------------------------------------------------------------

class MyIconFontIconPainter;

/// The main class for managing icons
/// This class requires a 2-phase construction. You must first create the class and then initialize it via an init* method
class MyIconFont : public QObject
{
Q_OBJECT

public:

    MyIconFont(QObject *parent = 0);
    virtual ~MyIconFont();

    void init( const QString& fontname );
    bool initFontAwesome();

    void addNamedCodepoint( const QString& name, int codePoint );
    QHash<QString,int> namedCodePoints() { return namedCodepoints_; }

    void setDefaultOption( const QString& name, const QVariant& value  );
    QVariant defaultOption( const QString& name );

    QIcon icon( int character, const QVariantMap& options = QVariantMap() );
    QIcon icon( const QString& name, const QVariantMap& options = QVariantMap() );
    QIcon icon(const QString& name, const QColor &color, const QString &painterName = QString(), int markerNum = -1, float scaleFactor = 0.9);
    QIcon icon(const QString& name, const QColor &color, const QColor &selectionColor, const QString &painterName = QString(), int markerNum = -1, float scaleFactor = 0.9);
    QIcon icon(MyIconFontIconPainter* painter, const QVariantMap& optionMap = QVariantMap() );

    void give( const QString& name, MyIconFontIconPainter* painter );

    QFont font( int size );

    /// Returns the font-name that is used as icon-map
    QString fontName() { return fontName_ ; }

private:
    QString fontName_;                                     ///< The font name used for this map
    QHash<QString,int> namedCodepoints_;                   ///< A map with names mapped to code-points

    QHash<QString, MyIconFontIconPainter*> painterMap_;     ///< A map of custom painters
    QVariantMap defaultOptions_;                           ///< The default icon options
    MyIconFontIconPainter* fontIconPainter_;                ///< A special painter fo painting codepoints
};


//---------------------------------------------------------------------------------------


/// The QtAwesomeIconPainter is a specialized painter for painting icons
/// your can implement an iconpainter to create custom font-icon code
class MyIconFontIconPainter
{
public:
    virtual ~MyIconFontIconPainter() {}
    virtual void paint( MyIconFont* awesome, QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state, const QVariantMap& options ) = 0;
};

#endif // MYICONFONT_H
