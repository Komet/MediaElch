#include "ui/MacUiUtilities.h"

#undef slots
#import <Cocoa/Cocoa.h>
#include <QtGlobal>

namespace mediaelch {
namespace ui {

bool macIsInDarkTheme()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // macOS 10.13 doesn't support NSAppearanceNameDarkAqua and we can't build it on it.
    // The easiest way I found was to just drop support for dark mode on macOS 10.X.
    // With macOS 11 and later, users can use our Qt6 version.
    return false;
#else
    // See tutorial at <https://successfulsoftware.net/2021/03/31/how-to-add-a-dark-theme-to-your-qt-application/>
    if (__builtin_available(macOS 10.14, *)) {
        auto appearance = [NSApp.effectiveAppearance
            bestMatchFromAppearancesWithNames:@[ NSAppearanceNameAqua, NSAppearanceNameDarkAqua ]];
        return [appearance isEqualToString:NSAppearanceNameDarkAqua];
    }
    return false;
#endif
}

} // namespace ui
} // namespace mediaelch
