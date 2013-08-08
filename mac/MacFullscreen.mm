#include "MacFullscreen.h"

#import <Foundation/NSString.h>
#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>

bool MacFullscreen::supportsFullscreen()
{
    NSString *string = [NSString string];
    return [string respondsToSelector:@selector(linguisticTagsInRange:scheme:options:orthography:tokenRanges:)];
}

void MacFullscreen::addFullscreen(MainWindow *mainWindow)
{
#if defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    if (supportsFullscreen()) {
        NSView *nsview = (NSView *) mainWindow->winId();
        NSWindow *nswindow = [nsview window];
        [nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    }
#endif
}
