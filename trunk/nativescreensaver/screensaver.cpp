#include <eikenv.h>
#include <sensrvchannelfinder.h>
#include <sensrvchannel.h>
#include <sensrvproximitysensor.h>
#include <AknAppUi.h>
#include <s32file.h>
#include <f32file.h>

#include "screensaver.h"
#include "screensaver.hrh"

const TInt KOneSecond = 1000000;

const TInt KTopMargin = 20;
const TInt KBottomMargin = 20;
const TInt KTimeDateGap = 40;
_LIT(KConfigPath, "c:\\data\\annascreensaver.cfg");
_LIT(KFontName, "Nokia Sans S60");

const TBool KUseSensor = true; //Beware: on AMOLED-screen devices proximity sensor might consume more power than the sceeen!
_LIT(KSaverName, "Anna Screensaver+");
const CAknAppUi::TAppUiOrientation KDefaultOrientation = CAknAppUi::EAppUiOrientationAutomatic;

void SetOrientationL(CAknAppUi::TAppUiOrientation orientation)
{
    CAknAppUi* appUi = dynamic_cast<CAknAppUi*>(CEikonEnv::Static()->AppUi());
    if (appUi)
        appUi->SetOrientationL(orientation);
}

CSensrvChannel* CreateSensorL()
{
    CSensrvChannel* result = NULL;
    CSensrvChannelFinder* channelFinder = CSensrvChannelFinder::NewL();
    CleanupStack::PushL(channelFinder);

    RSensrvChannelInfoList channelInfoList;
    CleanupClosePushL(channelInfoList);
    TSensrvChannelInfo channelInfo;
    channelInfo.iChannelType = KSensrvChannelTypeIdProximityMonitor;
    channelFinder->FindChannelsL(channelInfoList, channelInfo);

    if(channelInfoList.Count() >= 1)
    {
        TSensrvChannelInfo inf = channelInfoList[0];
        result = CSensrvChannel::NewL(inf);
    }

    CleanupStack::PopAndDestroy();
    CleanupStack::PopAndDestroy(channelFinder);

    return result;
}

CScreenSaver::CScreenSaver()
{
    _proximitySensor = NULL;
    _timeFont = NULL;
    _dateFont = NULL;
    _notifyFont = NULL;

    _isListening = false;
    _isVisible = true;
    _screenOrientation = KDefaultOrientation;
}

CScreenSaver::~CScreenSaver()
{
    if (_timeFont != NULL)
        CEikonEnv::Static()->ScreenDevice()->ReleaseFont(_timeFont);
    if (_dateFont != NULL)
        CEikonEnv::Static()->ScreenDevice()->ReleaseFont(_dateFont);
    if (_notifyFont != NULL)
        CEikonEnv::Static()->ScreenDevice()->ReleaseFont(_notifyFont);
    delete _proximitySensor;
}

CScreenSaver* CScreenSaver::NewLC()
{
    CScreenSaver* self = new (ELeave)CScreenSaver;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}

CScreenSaver* CScreenSaver::NewL()
{
    CScreenSaver* self = NewLC();
    CleanupStack::Pop();
    return self;
}

void CScreenSaver::SaveSettingsL()
{
    RFs& fileServer = CCoeEnv::Static()->FsSession();
    RFileWriteStream writeStream;
    TInt err = writeStream.Replace(fileServer, KConfigPath, EFileWrite);

    if (err == KErrNone)
    {
        writeStream.PushL();

        writeStream.WriteInt8L(_screenOrientation);

        writeStream.CommitL();
        CleanupStack::PopAndDestroy(&writeStream);
    }
}

void CScreenSaver::LoadSettingsL()
{
    RFs& fileServer = CCoeEnv::Static()->FsSession();
    RFileReadStream readStream;
    TInt err = readStream.Open(fileServer, KConfigPath, EFileRead);
    CleanupClosePushL(readStream);
    if (err == KErrNone)
    {
        readStream >> _screenOrientation;
    }
    CleanupStack::PopAndDestroy(&readStream);
}

void CScreenSaver::ConstructL()
{
    LoadSettingsL();
}

TInt CScreenSaver::InitializeL(MScreensaverPluginHost* aHost)
{
    //Init sensor
    if (KUseSensor)
        _proximitySensor = CreateSensorL();

    TRAP_IGNORE(SetOrientationL(static_cast<CAknAppUi::TAppUiOrientation>(_screenOrientation)));

    //Init fonts
    CWsScreenDevice* sd = CEikonEnv::Static()->ScreenDevice();

    const CFont* baseFont = CEikonEnv::Static()->NormalFont();
    TFontSpec spec = baseFont->FontSpecInTwips();

    spec.iTypeface.iName = KFontName;

    spec.iFontStyle.SetStrokeWeight(EStrokeWeightNormal);
    spec.iHeight = 4 * KTwipsPerPoint;
    sd->GetNearestFontToDesignHeightInTwips(_notifyFont, spec);

    spec.iHeight = 8 * KTwipsPerPoint;
    sd->GetNearestFontToDesignHeightInTwips(_dateFont, spec);

    spec.iHeight = 40 * KTwipsPerPoint;
    spec.iFontStyle.SetStrokeWeight(EStrokeWeightBold);
    spec.iFontStyle.SetEffects(FontEffect::EOutline, ETrue);
    sd->GetNearestFontToDesignHeightInTwips(_timeFont, spec);

    _host = aHost;
    _host->OverrideStandardIndicators();
    //iHost->UseStandardIndicators();

    return KErrNone;
}

void CScreenSaver::SetVisible(TBool isVisible)
{
    if (_isVisible != isVisible)
    {
        _isVisible = isVisible;
        _host->SetRefreshTimerValue(1);
    }
}

void CScreenSaver::DataReceived(CSensrvChannel &aChannel, TInt /*aCount*/, TInt /*aDataLost*/)
{
    if (aChannel.GetChannelInfo().iChannelType == KSensrvChannelTypeIdProximityMonitor)
    {
        TSensrvProximityData data;
        TPckg<TSensrvProximityData> package(data);
        aChannel.GetData(package);

        switch (data.iProximityState)
        {
            case TSensrvProximityData::EProximityDiscernible:
                SetVisible(EFalse);
                break;
            case TSensrvProximityData::EProximityIndiscernible:
                StopSensor();
                SetVisible(ETrue);
                break;
        }
    }
}

void CScreenSaver::DataError(CSensrvChannel &aChannel, TSensrvErrorSeverity aError)
{
    if (&aChannel == _proximitySensor)
    {
        if (aError == ESensrvErrorSeverityFatal)
        {
            _isListening = false;
            delete _proximitySensor;
            _proximitySensor = NULL;
        }
        else
        {
            StopSensor();
        }
        SetVisible(ETrue);
    }
}

void CScreenSaver::DrawIndicators(CWindowGc& gc, TInt x, TInt y)
{
    const TInt KIconWidth = 14;
    const TInt KIconHeight = 10;
    const TInt KGap = 10;
    const TInt KSpace = 5;

    TBuf<5> nMessages;
    TBuf<5> nMissedCalls;

    TInt fullWidth = 0;
    TIndicatorPayload payload;
    if (_host->GetIndicatorPayload(EScreensaverIndicatorIndexNewMessages, payload) == KErrNone
            /*&& payload.iIsDisplayed*/ && payload.iInteger > 0)
    {
        nMessages.AppendNum(payload.iInteger);
        fullWidth += _notifyFont->TextWidthInPixels(nMessages) + KIconWidth + KSpace;
    }

    if (_host->GetIndicatorPayload(EScreensaverIndicatorIndexNewMissedCalls, payload) == KErrNone
            /*&& payload.iIsDisplayed*/ && payload.iInteger > 0)
    {
        nMissedCalls.AppendNum(payload.iInteger);
        fullWidth += _notifyFont->TextWidthInPixels(nMissedCalls) + KIconWidth + KSpace;
    }

    if (nMessages.Length() * nMissedCalls.Length() > 0)
        fullWidth += KGap;

    x = (_screenRect.iWidth - fullWidth)/2;

    gc.UseFont(_notifyFont);
    //gc.SetPenColor(KRgbRed);
    if (nMessages.Length() > 0)
    {
        gc.DrawText(nMessages, TPoint(x, y + _notifyFont->AscentInPixels()));
        x += _notifyFont->TextWidthInPixels(nMessages) + KSpace;

        //CFbsBitmap* bmp = msg.iIcon->Bitmap();

        gc.DrawRect(TRect(TPoint(x, y), TSize(KIconWidth, KIconHeight)));
        gc.DrawLine(TPoint(x, y), TPoint(x + KIconWidth/2, y + KIconHeight/2));
        gc.DrawLineTo(TPoint(x + KIconWidth, y));
        x += KIconWidth;

        x += KGap;
    }

    if (nMissedCalls.Length() > 0)
    {
        gc.DrawText(nMissedCalls, TPoint(x, y + _notifyFont->AscentInPixels()));
        x += _notifyFont->TextWidthInPixels(nMissedCalls) + KSpace;

        //CFbsBitmap* bmp = msg.iIcon->Bitmap();

        const int KPenSize = 2;
        gc.SetPenSize(TSize(KPenSize, KPenSize));
        gc.DrawLine(TPoint(x + KIconWidth - KPenSize, y + 5), TPoint(x + KIconWidth - KPenSize, y));
        gc.DrawLineTo(TPoint(x, y));
        gc.DrawLineTo(TPoint(x, y + KIconHeight/2));
        gc.SetPenSize(TSize(1, 1));
        x += KIconWidth;
    }
}

TInt CScreenSaver::Draw(CWindowGc& gc)
{
    gc.Clear();
    if (!_isVisible)
    {
        _host->SetRefreshTimerValue(0); //disable timer
        return KErrNone;
    }
    gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
    gc.SetPenStyle(CGraphicsContext::ESolidPen);

    gc.SetPenColor(KRgbWhite);
    gc.SetBrushColor(KRgbBlack);
    TTime now; now.HomeTime();

    TBuf<20> timeString;
    _LIT(KTimeFormat,"%:0%J%:1%T");
    now.FormatL(timeString, KTimeFormat);
    TInt timeWidth = _timeFont->TextWidthInPixels(timeString);
    TInt xPos = (_screenRect.iWidth - timeWidth) / 2;
    TInt yPos = KTopMargin + ((now.DateTime().Hour()*60. + now.DateTime().Minute())/1439)
            * (_screenRect.iHeight / 2 - KTopMargin - KBottomMargin) + _timeFont->AscentInPixels();

    TBuf<20> dateString;
    _LIT(KDateFormat,"%*E %/0%1%/1%2%/2%3%/3");
    now.FormatL(dateString, KDateFormat);
    TInt xPosDate = (_screenRect.iWidth - _dateFont->TextWidthInPixels(dateString)) / 2;
    TInt yPosDate = yPos + _dateFont->AscentInPixels() + KTimeDateGap;

    TBuf<5> ampm;
    _LIT(KAMPMFormat, "%B");
    now.FormatL(ampm, KAMPMFormat);
    //xPos -= _dateFont->TextWidthInPixels(ampm)/2;

    gc.UseFont(_timeFont);
    for (int i = 0; i < 2; i++) //magic
    {
        gc.DrawText(timeString, TPoint(xPos, yPos));
    }

    gc.UseFont(_dateFont);

    if (ampm.Length() > 0)
        gc.DrawText(ampm, TPoint(xPos + timeWidth, yPos));

    gc.DrawText(dateString, TPoint(xPosDate, yPosDate));

    DrawIndicators(gc, xPosDate, yPosDate + 14);

    UpdateRefreshTimer();
    TRAPD(err, StartSensorL());

    return err;
}

void CScreenSaver::StartSensorL()
{
    if (!_isListening && _proximitySensor != NULL)
    {
        _proximitySensor->OpenChannelL();
        _proximitySensor->StartDataListeningL(this, 1, 0, 0);
        _isListening = ETrue;
    }
}

void CScreenSaver::StopSensor()
{
    if (_isListening && _proximitySensor != NULL)
    {
        _proximitySensor->StopDataListening();
        _proximitySensor->CloseChannel();
        _isListening = false;
    }
}

void CScreenSaver::UpdateRefreshTimer()
{
    TTime now; now.HomeTime();
    TDateTime dateTime = now.DateTime();
    _host->SetRefreshTimerValue((60 - dateTime.Second())*KOneSecond - dateTime.MicroSecond());
}

const TDesC16& CScreenSaver::Name() const
{
    return KSaverName;
}

TInt CScreenSaver::HandleScreensaverEventL(TScreensaverEvent event, TAny*)
{
    TInt err(KErrNone);
    switch (event)
    {
        case EScreensaverEventStarting:
        {
            _isVisible = true;

            TScreensaverPartialMode partial;
            partial.iType = EPartialModeTypeMostPowerSaving;
            partial.iBpp = 0;

            _screenRect.iHeight = CEikonEnv::Static()->ScreenDevice()->SizeInPixels().iHeight;
            _screenRect.iWidth = CEikonEnv::Static()->ScreenDevice()->SizeInPixels().iWidth;
            _host->SetActiveDisplayArea(KTopMargin, Max(_screenRect.iWidth, _screenRect.iHeight) - 1, partial);

            _host->SetRefreshTimerValue(KOneSecond);
            _host->RequestTimeout(0);
            StartSensorL();
            break;
        }
        case EScreensaverEventDisplayChanged:
        {
            TScreensaverDisplayInfo displayInfo;
            displayInfo.iSize = sizeof(TScreensaverDisplayInfo);
            if (_host->DisplayInfo(&displayInfo) == KErrNone)
                _screenRect = displayInfo.iRect.Size();
            break;
        }
        case EScreensaverEventStopping:
        {
            StopSensor();
            break;
        }
    }
    return err;
}

TInt CScreenSaver::PluginFunction(TScPluginCaps caps, TAny*)
{
    TInt err = KErrNone;
    if (caps == EScpCapsConfigure) {
        if(CEikonEnv::Static()->QueryWinL(_L("Allow landscape orientation?"), _L("")))
        {
            _screenOrientation = CAknAppUi::EAppUiOrientationAutomatic;
        }
        else
        {
            _screenOrientation = CAknAppUi::EAppUiOrientationPortrait;
        }
        TRAPD(err, SaveSettingsL());
    }
    return err;
}

