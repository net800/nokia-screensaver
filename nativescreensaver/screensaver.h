#ifndef SSAVER_H
#define SSAVER_H

#include <screensaverpluginintdef.h>

class CScreenSaver : public CScreensaverPluginInterfaceDefinition
{
public:
    static CScreenSaver* NewL();
    static CScreenSaver* NewLC();
    ~CScreenSaver();

public: //MScreensaverPlugin
    virtual TInt InitializeL(MScreensaverPluginHost* aHost);
    virtual TInt Draw(CWindowGc& aGc);
    virtual const TDesC16& Name() const;
    virtual TInt HandleScreensaverEventL(TScreensaverEvent aEvent, TAny* aData);
    virtual TInt Capabilities() { return EScpCapsNone; }
    virtual TInt PluginFunction(TScPluginCaps /*aFunction*/, TAny* /*aParam*/) { return KErrNone; }

private:
    CScreenSaver();
    void ConstructL();
    void DrawIndicators(CWindowGc& gc, TInt x, TInt y);
    void UpdateRefreshTimer();

private: // Data
    TRect _screenRect;
    MScreensaverPluginHost *_host;
    CFont* _timeFont;
    CFont* _dateFont;
    CFont* _notifyFont;
};

#endif
