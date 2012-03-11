#ifndef SSAVER_H
#define SSAVER_H

#include <screensaverpluginintdef.h>
#include <sensrvdatalistener.h>

class CScreenSaver : public CScreensaverPluginInterfaceDefinition, MSensrvDataListener
{
public:
    static CScreenSaver* NewL();
    static CScreenSaver* NewLC();
    ~CScreenSaver();
    void SetVisible(TBool visible);

public: //MScreensaverPlugin
    virtual TInt InitializeL(MScreensaverPluginHost* aHost);
    virtual TInt Draw(CWindowGc& aGc);
    virtual const TDesC16& Name() const;
    virtual TInt HandleScreensaverEventL(TScreensaverEvent aEvent, TAny* aData);
    virtual TInt Capabilities() { return EScpCapsNone; }
    virtual TInt PluginFunction(TScPluginCaps /*aFunction*/, TAny* /*aParam*/) { return KErrNone; }

public: //MSensrvDataListener
    void DataReceived(CSensrvChannel &aChannel, TInt aCount, TInt aDataLost);
    void DataError(CSensrvChannel &aChannel, TSensrvErrorSeverity aError);
    void GetDataListenerInterfaceL(TUid aInterfaceUid, TAny *&aInterface) {}

private:
    CScreenSaver();
    void ConstructL();
    void DrawIndicators(CWindowGc& gc, TInt x, TInt y);
    void UpdateRefreshTimer();
    void StartSensorL();
    void StopSensor();

private: // Data
    TSize _screenRect;
    TBool _isVisible;
    TBool _isListening;
    MScreensaverPluginHost *_host;
    CSensrvChannel* _proximitySensor;
    CFont* _timeFont;
    CFont* _dateFont;
    CFont* _notifyFont;
};

#endif
