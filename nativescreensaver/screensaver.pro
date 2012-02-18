#-------------------------------------------------
#
# Project created by QtCreator 2011-06-07T14:09:50
#
#-------------------------------------------------

QT       -= gui

TARGET = screensaver
TEMPLATE = lib

symbian: {

    SOURCES += screensaver.cpp \
        ImplementationProxy.cpp

    HEADERS += screensaver.h \
        screensaver.hrh

    OTHER_FILES += \
        screensaver.rss \
        eabi/screensaveru.def

    pluginrsc = "SOURCEPATH ."\
      "START RESOURCE screensaver.rss" \
      "END"
    
    MMP_RULES += pluginrsc

    LIBS += -lapgrfx -lecom -lcone -leikcore -lgdi -leuser -lefsrv
# -lws32
# -lavkon -lecam
# -legul

    TARGET.UID2 = 0x10009D8D
    TARGET.UID3 = 0x2004201E # SCREENSAVER_UID3
    TARGET.CAPABILITY = NetworkServices LocalServices Location ReadUserData WriteUserData ReadDeviceData WriteDeviceData SwEvent UserEnvironment PowerMgmt
    # additional capabilities required by Symbian Anna
    #TARGET.CAPABILITY += CommDD MultimediaDD TrustedUI ProtServ DiskAdmin NetworkControl SurroundingsDD
    TARGET.EPOCALLOWDLLDATA = 1

    addFiles.path = !:/sys/bin
    addFiles.sources = screensaver.dll
    deployRsc = "\"$${EPOCROOT}epoc32/data/screensaver.rsc\" - \"!:/resource/plugins/screensaver.rsc\""
    addFiles.pkg_postrules += deployRsc
    DEPLOYMENT += addFiles
}    
