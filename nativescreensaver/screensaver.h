#ifndef SSAVER_H
#define SSAVER_H

#include <screensaverpluginintdef.h>

class CLaunchSaver : public CScreensaverPluginInterfaceDefinition
{
public:
    static CLaunchSaver* NewL();

    ~CLaunchSaver();

public: // From MScreensaverPlugin
    virtual TInt InitializeL(MScreensaverPluginHost* aHost);
    virtual TInt Draw(CWindowGc& aGc);
    virtual const TDesC16& Name() const;
    virtual TInt HandleScreensaverEventL(TScreensaverEvent aEvent, TAny* aData);
    virtual TInt Capabilities() { return EScpCapsNone; }
    virtual TInt PluginFunction(TScPluginCaps /*aFunction*/, TAny* /*aParam*/) { return KErrNone; }

private:
    CLaunchSaver();
    static CLaunchSaver* NewLC();
    void ConstructL();
    void DrawIndicators(CWindowGc& gc, int x, int y);
    void UpdateRefreshTimer();

private: // Data
    MScreensaverPluginHost *iHost;
    TRect screenRect;
    CFont* timeFont;
    CFont* dateFont;
    CFont* notifyFont;
};

#endif
