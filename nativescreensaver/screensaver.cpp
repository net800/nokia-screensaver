#include <eikenv.h>

#include "screensaver.h"
#include "screensaver.hrh"

const TInt KLightsOnTimeoutInterval = 30*60 + 30;//30.5 min
const TInt KOneSecond = 1000000;
const TInt KTopMargin = 20;
const TInt KBottomMargin = 20;
const TInt KGap = 40;

_LIT(KSaverName, "Native Screensaver");
_LIT(KFontName, "Nokia Sans S60");

CLaunchSaver::CLaunchSaver()
{
    CWsScreenDevice* sd = CEikonEnv::Static()->ScreenDevice();

    const CFont* baseFont = CEikonEnv::Static()->NormalFont();
    TFontSpec spec = baseFont->FontSpecInTwips();

    spec.iTypeface.iName = KFontName;

    spec.iFontStyle.SetStrokeWeight(EStrokeWeightNormal);
    spec.iHeight = 4 * KTwipsPerPoint;
    sd->GetNearestFontToDesignHeightInTwips(notifyFont, spec);

    spec.iHeight = 8 * KTwipsPerPoint;
    sd->GetNearestFontToDesignHeightInTwips(dateFont, spec);

    spec.iHeight = 40 * KTwipsPerPoint;
    spec.iFontStyle.SetStrokeWeight(EStrokeWeightBold);
    spec.iFontStyle.SetEffects(FontEffect::EOutline, true);

    sd->GetNearestFontToDesignHeightInTwips(timeFont, spec);

    TInt height = sd->SizeInPixels().iHeight;
    TInt width = sd->SizeInPixels().iWidth;
    screenRect.SetWidth(width);
    screenRect.SetHeight(height);
}

CLaunchSaver::~CLaunchSaver()
{
    CEikonEnv::Static()->ScreenDevice()->ReleaseFont(timeFont);
    CEikonEnv::Static()->ScreenDevice()->ReleaseFont(dateFont);
    CEikonEnv::Static()->ScreenDevice()->ReleaseFont(notifyFont);
}

CLaunchSaver* CLaunchSaver::NewL()
{
    CLaunchSaver* self = new (ELeave)CLaunchSaver();
    return self;
}

CLaunchSaver* CLaunchSaver::NewLC()
{
    CLaunchSaver* self = new (ELeave)CLaunchSaver;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}

void CLaunchSaver::ConstructL()
{}

TInt CLaunchSaver::InitializeL( MScreensaverPluginHost* aHost )
{
    iHost = aHost;
    iHost->SetRefreshTimerValue(KOneSecond);
    iHost->OverrideStandardIndicators();
    //iHost->UseStandardIndicators();

    return KErrNone;
}

void CLaunchSaver::DrawIndicators(CWindowGc& gc, int x, int y)
{
    const TInt KIconWidth = 14;
    const TInt KIconHeight = 10;
    const TInt KGap = 10;
    const TInt KSpace = 5;

    TBuf<5> nMessages;
    TBuf<5> nMissedCalls;

    TInt fullWidth = 0;
    TIndicatorPayload payload;
    if ((iHost->GetIndicatorPayload(EScreensaverIndicatorIndexNewMessages, payload) == KErrNone)
            /*&& payload.iIsDisplayed > 0*/ && payload.iInteger > 0)
    {
        nMessages.AppendNum(payload.iInteger);
        fullWidth += notifyFont->TextWidthInPixels(nMessages) + KIconWidth + KSpace;
    }

    if ((iHost->GetIndicatorPayload(EScreensaverIndicatorIndexNewMissedCalls, payload) == KErrNone)
            /*&& payload.iIsDisplayed > 0*/ && payload.iInteger > 0)
    {
        nMissedCalls.AppendNum(payload.iInteger);
        fullWidth += notifyFont->TextWidthInPixels(nMissedCalls) + KIconWidth + KSpace;
    }

    if (nMessages.Length() * nMissedCalls.Length() > 0)
        fullWidth += KGap;

    x = (screenRect.Width() - fullWidth)/2;

    gc.UseFont(notifyFont);

    if (nMessages.Length() > 0)
    {
        gc.DrawText(nMessages, TPoint(x, y + notifyFont->AscentInPixels()));
        x += notifyFont->TextWidthInPixels(nMessages) + KSpace;

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
        gc.DrawText(nMissedCalls, TPoint(x, y + notifyFont->AscentInPixels()));
        x += notifyFont->TextWidthInPixels(nMissedCalls) + KSpace;

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

TInt CLaunchSaver::Draw(CWindowGc& gc)
{
    gc.Clear();
    gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
    gc.SetPenStyle(CGraphicsContext::ESolidPen);

    gc.SetPenColor(KRgbWhite);
    gc.SetBrushColor(KRgbBlack);
    TTime now; now.HomeTime();

    TBuf<20> timeString;
    _LIT(KOwnTimeFormat,"%:0%H%:1%T");
    //_LIT(KOwnTimeFormat,"%:0%H%:1%T%:2%S.%*C3%:3");
    now.FormatL(timeString, KOwnTimeFormat);

    TInt xPos = (screenRect.Width() - timeFont->TextWidthInPixels(timeString)) / 2;
    TInt yPos = ((now.DateTime().Hour()*60. + now.DateTime().Minute())/1439)
            * (screenRect.Height() / 2 - KTopMargin - KBottomMargin) + timeFont->AscentInPixels() + KTopMargin;

    TBuf<20> dateString;
    _LIT(KOwnDateFormat,"%F%*E %D/%M/%*Y");
    now.FormatL(dateString, KOwnDateFormat);
    TInt xPosDate = (screenRect.Width() - dateFont->TextWidthInPixels(dateString)) / 2;
    TInt yPosDate = yPos + dateFont->AscentInPixels() + KGap;

    for (int i = 0; i < 2; i++) {
        gc.UseFont(timeFont);
        gc.DrawText(timeString, TPoint(xPos, yPos));

        gc.UseFont(dateFont);
        gc.DrawText(dateString, TPoint(xPosDate, yPosDate));
    }

    DrawIndicators(gc, xPosDate, yPosDate + 14);

    UpdateRefreshTimer();

    return KErrNone;
}

void CLaunchSaver::UpdateRefreshTimer()
{
    TTime now; now.HomeTime();
    TDateTime dateTime = now.DateTime();
    iHost->SetRefreshTimerValue((60 - dateTime.Second())*KOneSecond - dateTime.MicroSecond());
}

const TDesC16& CLaunchSaver::Name() const
{
    return KSaverName;
}

TInt CLaunchSaver::HandleScreensaverEventL(TScreensaverEvent event, TAny* /*aData*/)
{
    TInt err(KErrNone);
    switch (event)
    {
        case EScreensaverEventTimeout:
        {
            // Keep lights on
            iHost->RequestTimeout( KLightsOnTimeoutInterval );

            UpdateRefreshTimer();
            break;
        }
        case EScreensaverEventStarting:
        {
            // Switch to partial mode to save power
            TScreensaverPartialMode partial;
            partial.iType = EPartialModeTypeMostPowerSaving;
            partial.iBpp = 0;
            TInt height = CEikonEnv::Static()->ScreenDevice()->SizeInPixels().iHeight;
            iHost->SetActiveDisplayArea(KTopMargin, height - KBottomMargin, partial);

            iHost->RequestTimeout(KLightsOnTimeoutInterval);
            break;
        }
        default:
        {
            break;
        }
    }

    return err;
}

