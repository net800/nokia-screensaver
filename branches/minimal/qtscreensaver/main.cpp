/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#include <QtGui/QApplication>
#include <QDebug>
#include "qmlapplicationviewer.h"

#ifdef Q_WS_S60
#include <eikenv.h>
#endif

int main(int argc, char *argv[])
{
    // Use raster graphics, not much use for hw accelleration
    QApplication::setGraphicsSystem("raster");

    QApplication app(argc, argv);

    QmlApplicationViewer viewer;
    //viewer.setOrientation(QmlApplicationViewer::ScreenOrientationLockPortrait);
    viewer.setMainQmlFile(QLatin1String("qml/qtscreensaver/main.qml"));
    viewer.showFullScreen();

#ifdef Q_WS_S60
    // Bring window in front of screensaver window (which has only +2 magic)
    CEikonEnv::Static()->SetAutoForwarding(ETrue);
    CEikonEnv::Static()->BringForwards(ETrue, ECoeWinPriorityAlwaysAtFront + 3);

    // set window non-focusable, so that the events are passed to actual screensaver window
    RWindowGroup &wg = CEikonEnv::Static()->RootWin();
    wg.EnableReceiptOfFocus(EFalse);   
#endif

    return app.exec();
}
