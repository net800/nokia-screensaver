#include <eikenv.h>

#include "screensaver.h"
#include "screensaver.hrh"


const TInt KLightsOnTimeoutInterval = 30*60 + 30;//30.5 min
const TInt KOneSecond = 1000000;

const TInt KTopMargin = 20;
const TInt KBottomMargin = 20;
const TInt KTimeDateGap = 40;

_LIT(KSaverName, "Anna Screensaver");

CScreenSaver::CScreenSaver()
{
    CWsScreenDevice* sd = CEikonEnv::Static()->ScreenDevice();

    const CFont* baseFont = CEikonEnv::Static()->NormalFont();
    TFontSpec spec = baseFont->FontSpecInTwips();

    spec.iFontStyle.SetStrokeWeight(EStrokeWeightNormal);
    spec.iHeight = 4 * KTwipsPerPoint;
    sd->GetNearestFontToDesignHeightInTwips(_notifyFont, spec);

    spec.iHeight = 8 * KTwipsPerPoint;
    sd->GetNearestFontToDesignHeightInTwips(_dateFont, spec);

    spec.iHeight = 40 * KTwipsPerPoint;
    spec.iFontStyle.SetStrokeWeight(EStrokeWeightBold);
    spec.iFontStyle.SetEffects(FontEffect::EOutline, ETrue);

    sd->GetNearestFontToDesignHeightInTwips(_timeFont, spec);
}

CScreenSaver::~CScreenSaver()
{
    CEikonEnv::Static()->ScreenDevice()->ReleaseFont(_timeFont);
    CEikonEnv::Static()->ScreenDevice()->ReleaseFont(_dateFont);
    CEikonEnv::Static()->ScreenDevice()->ReleaseFont(_notifyFont);
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
}

TInt CScreenSaver::InitializeL(MScreensaverPluginHost* aHost)
{
    _host = aHost;
    _host->SetRefreshTimerValue(KOneSecond);
    _host->OverrideStandardIndicators();
    //iHost->UseStandardIndicators();

    return KErrNone;
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

    gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
    gc.SetPenStyle(CGraphicsContext::ESolidPen);

    gc.SetPenColor(KRgbWhite);
    gc.SetBrushColor(KRgbBlack);
    TTime now; now.HomeTime();

    TBuf<20> timeString;
    _LIT(KTimeFormat,"%:0%J%:1%T");
    now.FormatL(timeString, KTimeFormat);

    TInt xPos = (_screenRect.Width() - _timeFont->TextWidthInPixels(timeString)) / 2;
    TInt yPos = ((now.DateTime().Hour()*60. + now.DateTime().Minute())/1439)
            * (_screenRect.Height() / 2 - KTopMargin - KBottomMargin) + _timeFont->AscentInPixels() + KTopMargin;

    TBuf<20> dateString;
    _LIT(KDateFormat,"%*E %/0%1%/1%2%/2%3%/3");
    now.FormatL(dateString, KDateFormat);
    TInt xPosDate = (_screenRect.Width() - _dateFont->TextWidthInPixels(dateString)) / 2;
    TInt yPosDate = yPos + _dateFont->AscentInPixels() + KTimeDateGap;

    gc.UseFont(_timeFont);
    for (int i = 0; i < 2; i++) //magic
    {
        gc.DrawText(timeString, TPoint(xPos, yPos));
    }

    gc.UseFont(_dateFont);
    gc.DrawText(dateString, TPoint(xPosDate, yPosDate));

    DrawIndicators(gc, xPosDate, yPosDate + 14);

    UpdateRefreshTimer();

    return KErrNone;
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
            TScreensaverPartialMode partial;
            partial.iType = EPartialModeTypeMostPowerSaving;

            _screenRect.SetHeight(CEikonEnv::Static()->ScreenDevice()->SizeInPixels().iHeight);
            _screenRect.SetWidth(CEikonEnv::Static()->ScreenDevice()->SizeInPixels().iWidth);
            _host->SetActiveDisplayArea(KTopMargin, Max(_screenRect.Width(), _screenRect.Height()) - 1, partial);

            _host->RequestTimeout(KLightsOnTimeoutInterval);
            break;
        }
    }

    return err;
}
