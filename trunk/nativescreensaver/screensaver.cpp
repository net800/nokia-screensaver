#include <eikenv.h>
#include <sensrvchannelfinder.h>
#include <sensrvchannel.h>
#include <sensrvproximitysensor.h>

#include "screensaver.h"
#include "screensaver.hrh"

#define SCREENSAVER_EXTRA_FEATURES

const TInt KLightsOnTimeoutInterval = 30*60 + 30;//30.5 min
const TInt KOneSecond = 1000000;
const TInt KTopMargin = 20;
const TInt KBottomMargin = 20;
const TInt KGap = 40;

#ifdef SCREENSAVER_EXTRA_FEATURES
_LIT(KSaverName, "Anna Screensaver+");
const TBool KUseSensor = true;
#else
_LIT(KSaverName, "Anna Screensaver");
const TBool KUseSensor = false;
#endif

_LIT(KFontName, "Nokia Sans S60");

CScreenSaver::CScreenSaver()
{
    _proximitySensor = NULL;
    _isListening = false;
    _isVisible = true;

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
    spec.iFontStyle.SetEffects(FontEffect::EOutline, true);

    sd->GetNearestFontToDesignHeightInTwips(_timeFont, spec);

    TInt height = sd->SizeInPixels().iHeight;
    TInt width = sd->SizeInPixels().iWidth;
    _screenRect.SetWidth(width);
    _screenRect.SetHeight(height);
}

CScreenSaver::~CScreenSaver()
{
    CEikonEnv::Static()->ScreenDevice()->ReleaseFont(_timeFont);
    CEikonEnv::Static()->ScreenDevice()->ReleaseFont(_dateFont);
    CEikonEnv::Static()->ScreenDevice()->ReleaseFont(_notifyFont);
    if (_proximitySensor != NULL)
        CSensrvChannel::Delete(_proximitySensor);
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

void CScreenSaver::ConstructL()
{
    if (KUseSensor)
    {
        CSensrvChannelFinder* channelFinder = CSensrvChannelFinder::NewL();
        CleanupStack::PushL(channelFinder);

        RSensrvChannelInfoList channelInfoList;
        TSensrvChannelInfo channelInfo;
        channelInfo.iChannelType = KSensrvChannelTypeIdProximityMonitor;
        channelFinder->FindChannelsL(channelInfoList, channelInfo);

        if(channelInfoList.Count() >= 1)
        {
            TSensrvChannelInfo inf = channelInfoList[0];
            _proximitySensor = CSensrvChannel::NewL(inf);
        }

        channelInfoList.Close();
        CleanupStack::PopAndDestroy(channelFinder);
    }
}

TInt CScreenSaver::InitializeL(MScreensaverPluginHost* aHost)
{
    _host = aHost;
    _host->SetRefreshTimerValue(KOneSecond);
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

void CScreenSaver::DataReceived(CSensrvChannel &aChannel, TInt aCount, TInt aDataLost)
{
    if (aChannel.GetChannelInfo().iChannelType == KSensrvChannelTypeIdProximityMonitor)
    {
        TSensrvProximityData data;
        TPckg<TSensrvProximityData> package(data);
        aChannel.GetData(package);

        switch (data.iProximityState) {
            case TSensrvProximityData::EProximityDiscernible:
                SetVisible(false);
                break;
            case TSensrvProximityData::EProximityIndiscernible:
                StopSensor();
                SetVisible(true);
                break;
        }
    }
}

void CScreenSaver::DataError(CSensrvChannel &aChannel, TSensrvErrorSeverity aError)
{
    if (&aChannel == _proximitySensor && aError == ESensrvErrorSeverityFatal)
    {
        _isListening = false;
        _proximitySensor = NULL;
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
    if ((_host->GetIndicatorPayload(EScreensaverIndicatorIndexNewMessages, payload) == KErrNone)
            /*&& payload.iIsDisplayed > 0*/ && payload.iInteger > 0)
    {
        nMessages.AppendNum(payload.iInteger);
        fullWidth += _notifyFont->TextWidthInPixels(nMessages) + KIconWidth + KSpace;
    }

    if ((_host->GetIndicatorPayload(EScreensaverIndicatorIndexNewMissedCalls, payload) == KErrNone)
            /*&& payload.iIsDisplayed > 0*/ && payload.iInteger > 0)
    {
        nMissedCalls.AppendNum(payload.iInteger);
        fullWidth += _notifyFont->TextWidthInPixels(nMissedCalls) + KIconWidth + KSpace;
    }

    if (nMessages.Length() * nMissedCalls.Length() > 0)
        fullWidth += KGap;

    x = (_screenRect.Width() - fullWidth)/2;

    gc.UseFont(_notifyFont);

    if (nMessages.Length() > 0)
    {
        gc.DrawText(nMessages, TPoint(x, y + _notifyFont->AscentInPixels()));
        x += _notifyFont->TextWidthInPixels(nMessages) + KSpace;

        //CFbsBitmap* bmp = msg.iIcon->Bitmap();
        //gc.DrawBitmap(TPoint(10, 100), bmp);

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
        //gc.BitBlt(TPoint(10, 100), bmp);

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
        //_host->SetRefreshTimerValue(0); //disable timer? test this or UseRefreshTimer
        return KErrNone;
    }
    gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
    gc.SetPenStyle(CGraphicsContext::ESolidPen);

    gc.SetPenColor(KRgbWhite);
    gc.SetBrushColor(KRgbBlack);
    TTime now; now.HomeTime();

    TBuf<20> timeString;
    _LIT(KOwnTimeFormat,"%:0%H%:1%T");
    //_LIT(KOwnTimeFormat,"%:0%H%:1%T%:2%S.%*C3%:3");
    now.FormatL(timeString, KOwnTimeFormat);

    TInt xPos = (_screenRect.Width() - _timeFont->TextWidthInPixels(timeString)) / 2;
    TInt yPos = ((now.DateTime().Hour()*60. + now.DateTime().Minute())/1439)
            * (_screenRect.Height() / 2 - KTopMargin - KBottomMargin) + _timeFont->AscentInPixels() + KTopMargin;

    TBuf<20> dateString;
    _LIT(KOwnDateFormat,"%F%*E %D/%M/%*Y");
    now.FormatL(dateString, KOwnDateFormat);
    TInt xPosDate = (_screenRect.Width() - _dateFont->TextWidthInPixels(dateString)) / 2;
    TInt yPosDate = yPos + _dateFont->AscentInPixels() + KGap;

    for (int i = 0; i < 2; i++)
    {
        gc.UseFont(_timeFont);
        gc.DrawText(timeString, TPoint(xPos, yPos));

        gc.UseFont(_dateFont);
        gc.DrawText(dateString, TPoint(xPosDate, yPosDate));
    }

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
        _proximitySensor->StartDataListeningL(this, 1, 1, 0);
        _isListening = true;
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

TInt CScreenSaver::HandleScreensaverEventL(TScreensaverEvent event, TAny* /*aData*/)
{
    TInt err(KErrNone);
    switch (event)
    {
        case EScreensaverEventTimeout:
        {
            _host->RequestTimeout(KLightsOnTimeoutInterval);

            UpdateRefreshTimer();
            break;
        }
        case EScreensaverEventStarting:
        {
            _isVisible = true;

            TScreensaverPartialMode partial;
            partial.iType = EPartialModeTypeMostPowerSaving;
            partial.iBpp = 0;
            TInt height = CEikonEnv::Static()->ScreenDevice()->SizeInPixels().iHeight;
            _host->SetActiveDisplayArea(KTopMargin, height - KBottomMargin, partial);

            _host->RequestTimeout(KLightsOnTimeoutInterval);
            StartSensorL();
            break;
        }
        case EScreensaverEventStopping:
        {
            StopSensor();
        }
        default:
        {
            break;
        }
    }

    return err;
}

