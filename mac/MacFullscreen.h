#ifndef MACFULLSCREEN_H
#define MACFULLSCREEN_H

#include "main/MainWindow.h"

class MacFullscreen
{
public:
    static bool supportsFullscreen();
    static void addFullscreen(MainWindow *mainWindow);
};
#endif // MACFULLSCREEN_H
