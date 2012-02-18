# Add more folders to ship with the application, here
folder_01.source = qml/qtscreensaver
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

CONFIG += no_icon

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

RESOURCES += qtscreensaver.qrc

symbian: {
    TARGET.UID3 = 0x2004201C

    TARGET.CAPABILITY += NetworkServices SwEvent UserEnvironment

    LIBS += -lapgrfx -lcone -lws32 -leikcore
}


# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =


# Define QMLJSDEBUGGER to allow debugging of QML in debug builds
# (This might significantly increase build time)
# DEFINES += QMLJSDEBUGGER

# If your application uses the Qt Mobility libraries, uncomment
# the following lines and add the respective components to the
# MOBILITY variable.
CONFIG += mobility
MOBILITY += sensors

