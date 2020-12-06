#include "MyIconFont.h"

/**
 * This is all taken from the nice QtAwesome class which can be found at
 * https://github.com/gamecreature/QtAwesome/
 *
 * It has been renamed and adjusted to match another font than FontAwesome
 */

#include <QDebug>
#include <QFile>
#include <QFontDatabase>
#include <utility>

class StarIconPainter : public MyIconFontIconPainter
{
public:
    void paint(MyIconFont* awesome,
        QPainter* painter,
        const QRect& rect,
        QIcon::Mode mode,
        QIcon::State state,
        const QVariantMap& options) override
    {
        Q_UNUSED(mode);
        Q_UNUSED(state);
        Q_UNUSED(options);

        painter->save();

        // set the correct color
        QColor color = options.value("color").value<QColor>();
        QColor starColor = options.value("color-star").value<QColor>();
        QString text = options.value("text").toString();

        if (mode == QIcon::Disabled) {
            color = options.value("color-disabled").value<QColor>();
            QVariant alt = options.value("text-disabled");
            if (alt.isValid()) {
                text = alt.toString();
            }
        } else if (mode == QIcon::Active) {
            color = options.value("color-active").value<QColor>();
            QVariant alt = options.value("text-active");
            if (alt.isValid()) {
                text = alt.toString();
            }
        } else if (mode == QIcon::Selected) {
            color = options.value("color-selected").value<QColor>();
            QVariant alt = options.value("text-selected");
            if (alt.isValid()) {
                text = alt.toString();
            }
        }
        painter->setPen(color);

        // add some 'padding' around the icon
        int drawSize = qRound(rect.height() * options.value("scale-factor").toFloat());

        painter->setFont(awesome->font(drawSize));
        painter->drawText(rect, text, QTextOption(Qt::AlignCenter | Qt::AlignVCenter));

        const float size = 0.5f;
        QRect starRect(rect.left() + 1,
            qRound(rect.top() + rect.height() * (1 - size)) - 1,
            qRound(rect.width() * size),
            qRound(rect.height() * size));
        painter->setBrush(starColor);
        painter->setPen(starColor);
#ifdef Q_OS_MAC
        drawSize = qRound(starRect.height() * options.value("scale-factor").toFloat());
        const int digits = options.value("marker-text").toString().size();
        painter->setFont(QFont("", drawSize - (digits * 2 + 1))); // mac needs a smaller font
#else
        QFont f;
        f.setPointSize(6);
        painter->setFont(f);
#endif
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawEllipse(starRect);
        painter->setPen(QColor(255, 255, 255));
        painter->drawText(
            starRect, options.value("marker-text").toString(), QTextOption(Qt::AlignCenter | Qt::AlignVCenter));

        painter->restore();
    }
};

class DuplicateIconPainter : public MyIconFontIconPainter
{
public:
    void paint(MyIconFont* awesome,
        QPainter* painter,
        const QRect& rect,
        QIcon::Mode mode,
        QIcon::State state,
        const QVariantMap& options) override
    {
        Q_UNUSED(mode);
        Q_UNUSED(state);
        Q_UNUSED(options);

        painter->save();

        // set the correct color
        QColor color = options.value("color").value<QColor>();
        QString text = options.value("text").toString();

        if (mode == QIcon::Disabled) {
            color = options.value("color-disabled").value<QColor>();
            QVariant alt = options.value("text-disabled");
            if (alt.isValid()) {
                text = alt.toString();
            }
        } else if (mode == QIcon::Active) {
            color = options.value("color-active").value<QColor>();
            QVariant alt = options.value("text-active");
            if (alt.isValid()) {
                text = alt.toString();
            }
        } else if (mode == QIcon::Selected) {
            color = options.value("color-selected").value<QColor>();
            QVariant alt = options.value("text-selected");
            if (alt.isValid()) {
                text = alt.toString();
            }
        }
        painter->setPen(color);

        float scale = 0.7f;
        QRect firstRect = rect;
        firstRect.setWidth(static_cast<int>(rect.width() * scale));
        firstRect.setHeight(static_cast<int>(rect.height() * scale));

        QRect secondRect = rect;
        secondRect.setTop(rect.top() + (rect.width() - firstRect.width()));
        secondRect.setLeft(rect.left() + (rect.height() - firstRect.height()));
        secondRect.setWidth(static_cast<int>(rect.width() * scale));
        secondRect.setHeight(static_cast<int>(rect.height() * scale));

        int drawSize = qRound(firstRect.height() * options.value("scale-factor").toFloat());

        painter->setFont(awesome->font(drawSize));
        painter->drawText(firstRect, text, QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
        painter->drawText(secondRect, text, QTextOption(Qt::AlignCenter | Qt::AlignVCenter));

        painter->restore();
    }
};


class MyIconFontCharIconPainter : public MyIconFontIconPainter
{
public:
    void paint(MyIconFont* awesome,
        QPainter* painter,
        const QRect& rect,
        QIcon::Mode mode,
        QIcon::State state,
        const QVariantMap& options) override
    {
        Q_UNUSED(mode);
        Q_UNUSED(state);
        Q_UNUSED(options);

        painter->save();

        // set the correct color
        QColor color = options.value("color").value<QColor>();
        QString text = options.value("text").toString();

        if (mode == QIcon::Disabled) {
            color = options.value("color-disabled").value<QColor>();
            QVariant alt = options.value("text-disabled");
            if (alt.isValid()) {
                text = alt.toString();
            }
        } else if (mode == QIcon::Active) {
            color = options.value("color-active").value<QColor>();
            QVariant alt = options.value("text-active");
            if (alt.isValid()) {
                text = alt.toString();
            }
        } else if (mode == QIcon::Selected) {
            color = options.value("color-selected").value<QColor>();
            QVariant alt = options.value("text-selected");
            if (alt.isValid()) {
                text = alt.toString();
            }
        }
        painter->setPen(color);

        // add some 'padding' around the icon
        int drawSize = qRound(rect.height() * options.value("scale-factor").toFloat());

        painter->setFont(awesome->font(drawSize));
        painter->drawText(rect, text, QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
        painter->restore();
    }
};


//---------------------------------------------------------------------------------------


/// The painter icon engine.
class MyIconFontIconPainterIconEngine : public QIconEngine
{
public:
    MyIconFontIconPainterIconEngine(MyIconFont* awesome, MyIconFontIconPainter* painter, QVariantMap options) :
        awesomeRef_(awesome), iconPainterRef_(painter), options_(std::move(options))
    {
    }

    ~MyIconFontIconPainterIconEngine() override = default;

    MyIconFontIconPainterIconEngine* clone() const override
    {
        return new MyIconFontIconPainterIconEngine(awesomeRef_, iconPainterRef_, options_);
    }

    void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override
    {
        Q_UNUSED(mode);
        Q_UNUSED(state);
        iconPainterRef_->paint(awesomeRef_, painter, rect, mode, state, options_);
    }

    QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override
    {
        QPixmap pm(size);
        pm.fill(Qt::transparent); // we need transparency
        {
            QPainter p(&pm);
            paint(&p, QRect(QPoint(0, 0), size), mode, state);
        }
        return pm;
    }

private:
    MyIconFont* awesomeRef_;                ///< a reference to the QtAwesome instance
    MyIconFontIconPainter* iconPainterRef_; ///< a reference to the icon painter
    QVariantMap options_;                   ///< the options for this icon painter
};


//---------------------------------------------------------------------------------------

/// The default icon colors
MyIconFont::MyIconFont(QObject* parent) : QObject(parent)
{
    // initialize the default options
    setDefaultOption("color", QColor(50, 50, 50));
    setDefaultOption("color-star", QColor(248, 71, 68));
    setDefaultOption("color-disabled", QColor(70, 70, 70, 60));
    setDefaultOption("color-active", QColor(10, 10, 10));
    setDefaultOption("color-selected", QColor(10, 10, 10));
    setDefaultOption("scale-factor", 0.9);

    setDefaultOption("text", QVariant());
    setDefaultOption("text-disabled", QVariant());
    setDefaultOption("text-active", QVariant());
    setDefaultOption("text-selected", QVariant());

    fontIconPainter_ = new MyIconFontCharIconPainter();
}


MyIconFont::~MyIconFont()
{
    delete fontIconPainter_;
    //    delete errorIconPainter_;
    qDeleteAll(painterMap_);
}

/// initializes the QtAwesome icon factory with the given fontname
void MyIconFont::init(const QString& fontname)
{
    fontName_ = fontname;
}


/// a specialized init function so font-awesome is loaded and initialized
/// this method return true on success, it will return false if the fnot cannot be initialized
/// To initialize QtAwesome with font-awesome you need to call this method
bool MyIconFont::initFontAwesome()
{
    static int fontAwesomeFontId = -1;

    // only load font-awesome once
    if (fontAwesomeFontId < 0) {
        // load the font file
        QFile res(":/fonts/Pe-icon-7-stroke.ttf");
        if (!res.open(QIODevice::ReadOnly)) {
            qDebug() << "Font awesome font could not be loaded!";
            return false;
        }
        QByteArray fontData(res.readAll());
        res.close();

        // fetch the given font
        fontAwesomeFontId = QFontDatabase::addApplicationFontFromData(fontData);
    }

    QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(fontAwesomeFontId);
    if (!loadedFontFamilies.isEmpty()) {
        fontName_ = loadedFontFamilies.at(0);
    } else {
        qDebug() << "Font awesome font is empty?!";
        fontAwesomeFontId = -1; // restore the font-awesome id
        return false;
    }

    give("star", new StarIconPainter());
    give("duplicate", new DuplicateIconPainter());

    // intialize the map
    QHash<QString, int>& m = namedCodepoints_;
    m.insert("film", icon_film);
    m.insert("monitor", icon_monitor);
    m.insert("micro", icon_micro);
    m.insert("music", icon_music);
    m.insert("key", icon_key);
    m.insert("photo_gallery", icon_photo_gallery);
    m.insert("notebook", icon_notebook);
    m.insert("download", icon_download);
    m.insert("cloud_download", icon_cloud_download);
    m.insert("diskette", icon_diskette);
    m.insert("cloud_upload", icon_cloud_upload);
    m.insert("info", icon_info);
    m.insert("refresh", icon_refresh);
    m.insert("folder", icon_folder);
    m.insert("tools", icon_tools);
    m.insert("network", icon_network);
    m.insert("world", icon_world);
    m.insert("box2", icon_box2);
    m.insert("search", icon_search);
    m.insert("angle_right", icon_angle_right);
    m.insert("angle_right_circle", icon_angle_right_circle);
    m.insert("angle_up", icon_angle_up);
    m.insert("angle_up_circle", icon_angle_up_circle);
    m.insert("angle_down", icon_angle_down);
    m.insert("angle_down_circle", icon_angle_down_circle);
    m.insert("angle_left", icon_angle_left);
    m.insert("angle_left_circle", icon_angle_left_circle);
    m.insert("star", icon_star);
    m.insert("plus", icon_plus);
    m.insert("close", icon_close);
    m.insert("play", icon_play);
    m.insert("note", icon_note);
    m.insert("pen", icon_pen);
    m.insert("attention", icon_attention);
    m.insert("airplay", icon_airplay);
    m.insert("refresh_cloud", icon_refresh_cloud);
    m.insert("repeat", icon_repeat);
    m.insert("copy_file", icon_copy_file);
    m.insert("close_circle", icon_close_circle);
    m.insert("check", icon_check);

    return true;
}

void MyIconFont::addNamedCodepoint(const QString& name, int codePoint)
{
    namedCodepoints_.insert(name, codePoint);
}


/// Sets a default option. These options are passed on to the icon painters
void MyIconFont::setDefaultOption(const QString& name, const QVariant& value)
{
    defaultOptions_.insert(name, value);
}


/// Returns the default option for the given name
QVariant MyIconFont::defaultOption(const QString& name)
{
    return defaultOptions_.value(name);
}


// internal helper method to merge to option amps
static QVariantMap mergeOptions(const QVariantMap& defaults, const QVariantMap& override)
{
    QVariantMap result = defaults;
    if (!override.isEmpty()) {
        QMapIterator<QString, QVariant> itr(override);
        while (itr.hasNext()) {
            itr.next();
            result.insert(itr.key(), itr.value());
        }
    }
    return result;
}


/// Creates an icon with the given code-point
/// <code>
///     awesome->icon( icon_group )
/// </code>
QIcon MyIconFont::icon(int character, const QVariantMap& options)
{
    // create a merged QVariantMap to have default options and icon-specific options
    QVariantMap optionMap = mergeOptions(defaultOptions_, options);
    optionMap.insert("text", QString(QChar(character)));

    MyIconFontIconPainter* painter = painterMap_.value(options.value("painter-name").toString());
    if (painter == nullptr) {
        painter = fontIconPainter_;
    }

    return icon(painter, optionMap);
}


/// Creates an icon with the given name
///
/// You can use the icon names as defined on https://fontawesome.com/ without
/// the 'icon-' prefix
/// \param name the name of the icon
/// \param options extra option to pass to the icon renderer
QIcon MyIconFont::icon(const QString& name, const QVariantMap& options)
{
    // when it's a named codepoint
    if (namedCodepoints_.count(name) != 0) {
        return icon(namedCodepoints_.value(name), options);
    }


    // create a merged QVariantMap to have default options and icon-specific options
    QVariantMap optionMap = mergeOptions(defaultOptions_, options);

    // this method first tries to retrieve the icon
    MyIconFontIconPainter* painter = painterMap_.value(name);
    if (painter == nullptr) {
        return QIcon();
    }

    return icon(painter, optionMap);
}

QIcon MyIconFont::icon(const QString& name,
    const QColor& color,
    const QColor& selectionColor,
    const QString& painterName,
    int markerNum,
    float scaleFactor)
{
    QVariantMap options;
    options.insert("color", color);
    options.insert("painter-name", painterName);
    options.insert("scale-factor", scaleFactor);
    options.insert("color-selected", selectionColor);
    if (markerNum != 0) {
        options.insert("marker-text", QString::number(markerNum));
    }
    return icon(name, options);
}

QIcon MyIconFont::icon(const QString& name,
    const QColor& color,
    const QString& painterName,
    int markerNum,
    float scaleFactor)
{
    return icon(name, color, QColor(10, 10, 10), painterName, markerNum, scaleFactor);
}

/// Create a dynamic icon by simlpy supplying a painter object
/// The ownership of the painter is NOT transfered.
/// \param painter a dynamic painter that is going to paint the icon
/// \param optionMap the options to pass to the painter
QIcon MyIconFont::icon(MyIconFontIconPainter* painter, const QVariantMap& optionMap)
{
    // Warning, when you use memoryleak detection. You should turn it of for the next call
    // QIcon's placed in gui items are often cached and not deleted when my memory-leak detection checks for leaks.
    // I'm not sure if it's a Qt bug or something I do wrong
    auto* engine = new MyIconFontIconPainterIconEngine(this, painter, optionMap);
    return QIcon(engine);
}

/// Adds a named icon-painter to the QtAwesome icon map
/// As the name applies the ownership is passed over to QtAwesome
///
/// \param name the name of the icon
/// \param painter the icon painter to add for this name
void MyIconFont::give(const QString& name, MyIconFontIconPainter* painter)
{
    delete painterMap_.value(name); // delete the old one
    painterMap_.insert(name, painter);
}

/// Creates/Gets the icon font with a given size in pixels. This can be usefull to use a label for displaying icons
/// Example:
///
///    QLabel* label = new QLabel( QChar( icon_group ) );
///    label->setFont( awesome->font(16) )
QFont MyIconFont::font(int size)
{
    QFont font(fontName_);
    font.setPixelSize(size);
    return font;
}
