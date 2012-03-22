#-------------------------------------------------
#
# Project created by QtCreator 2011-06-07T14:09:50
#
#-------------------------------------------------

QT       -= gui

TARGET = sslauncher
TEMPLATE = lib

symbian: {

    SOURCES += sslauncher.cpp

    HEADERS += sslauncher.h \
        sslauncher.hrh

    OTHER_FILES += \
        sslauncher.rss \
        eabi/sslauncheru.def

    pluginrsc = "SOURCEPATH ."\
      "START RESOURCE sslauncher.rss" \
      "END"
    
    MMP_RULES += pluginrsc

    LIBS += -lapgrfx -lecom -lcone

    TARGET.UID2 = 0x10009D8D
    TARGET.UID3 = 0x20060C5E # SSLAUNCHER_UID3
    TARGET.CAPABILITY = NetworkServices LocalServices Location ReadUserData WriteUserData ReadDeviceData WriteDeviceData SwEvent UserEnvironment PowerMgmt
    # additional capabilities required by Symbian Anna
    # TARGET.CAPABILITY += CommDD MultimediaDD TrustedUI ProtServ DiskAdmin NetworkControl SurroundingsDD
    TARGET.EPOCALLOWDLLDATA = 1

    addFiles.path = !:/sys/bin
    addFiles.sources = sslauncher.dll
    deployRsc = "\"$${EPOCROOT}epoc32/data/sslauncher.rsc\" - \"!:/resource/plugins/sslauncher.rsc\""
    addFiles.pkg_postrules += deployRsc
    DEPLOYMENT += addFiles
}
