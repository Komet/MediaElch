#include "ui/MacUiUtilities.h"

#import <Cocoa/Cocoa.h>

namespace mediaelch {
namespace ui {

bool macIsInDarkTheme()
{
    // See tutorial at <https://successfulsoftware.net/2021/03/31/how-to-add-a-dark-theme-to-your-qt-application/>
    if (__builtin_available(macOS 10.14, *)) {
        auto appearance = [NSApp.effectiveAppearance
            bestMatchFromAppearancesWithNames:@[NSAppearanceNameAqua, NSAppearanceNameDarkAqua]];
        return [appearance isEqualToString:NSAppearanceNameDarkAqua];
    }
    return false;
}

} // namespace ui
} // namespace mediaelch
