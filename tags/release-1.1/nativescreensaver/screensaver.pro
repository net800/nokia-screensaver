QT       -= gui core

TARGET = annascreensaver
TEMPLATE = lib
VERSION = 1.1.0

symbian: {
    INCLUDEPATH += $${EPOCROOT}epoc32/include/mw \
        $${EPOCROOT}epoc32/include/platform \
        $${EPOCROOT}epoc32/include/platform/mw

    SOURCES += screensaver.cpp \
        ImplementationProxy.cpp

    HEADERS += screensaver.h \
        screensaver.hrh

    OTHER_FILES += \
        screensaver.rss \
        eabi/annascreensaveru.def

    pluginrsc = "SOURCEPATH ."\
      "START RESOURCE screensaver.rss" \
      "END"

    MMP_RULES += pluginrsc

    LIBS += -lecom -lcone -lgdi -lSensrvClient -lsensrvutil -lavkon -leiksrv -leikcore -lefsrv -lestor
    # -lapgrfx

    TARGET.UID2 = 0x10009D8D
    TARGET.UID3 = 0x20060C5D # SCREENSAVER_UID3
    TARGET.CAPABILITY = NetworkServices LocalServices Location ReadUserData WriteUserData ReadDeviceData WriteDeviceData SwEvent UserEnvironment PowerMgmt
    # additional capabilities required by Symbian Anna
    #TARGET.CAPABILITY += CommDD MultimediaDD TrustedUI ProtServ DiskAdmin NetworkControl SurroundingsDD
    TARGET.EPOCALLOWDLLDATA = 1

    default_deployment.pkg_prerules -= pkg_depends_qt

    addFiles.path = !:/sys/bin
    addFiles.sources = annascreensaver.dll
    deployRsc = "\"$${EPOCROOT}epoc32/data/screensaver.rsc\" - \"!:/resource/plugins/annascreensaver.rsc\""
    addFiles.pkg_postrules += deployRsc
    DEPLOYMENT += addFiles
    DEPLOYMENT.display_name = Anna Screensaver
}
