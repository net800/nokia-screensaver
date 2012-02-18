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
/*
    TInt mode = sd->CurrentScreenMode();
    TPixelsAndRotation sr;
    sd->GetScreenModeSizeAndRotation(mode, sr);
    sr.iPixelSize.iHeight = 360;
    sr.iPixelSize.iWidth = 640;
    sr.iRotation = CFbsBitGc::EGraphicsOrientationRotated90;
    sd->SetScreenSizeAndRotation(sr);
    */

    //sd->SetAppScreenMode(1);

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
    CLaunchSaver* self = new ( ELeave ) CLaunchSaver();
    return self;
}

CLaunchSaver* CLaunchSaver::NewLC() {
    CLaunchSaver* self = new (ELeave) CLaunchSaver;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}

void CLaunchSaver::ConstructL() {
}

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
    TIndicatorPayload msg;
    if (iHost->GetIndicatorPayload(EScreensaverIndicatorIndexNewMessages, msg) == KErrNone) {
        if (msg.iIsDisplayed > 0 && msg.iInteger > 0)
        {
            /*
            CFbsBitmap* bmp = msg.iIcon->Bitmap();
            gc.DrawBitmap(TPoint(10, 100), bmp);
            */
            gc.DrawRect(TRect(TPoint(x, y), TSize(14,10)));
            gc.DrawLine(TPoint(x, y), TPoint(x + 7, y + 5));
            gc.DrawLineTo(TPoint(x + 14, y));
            x += 14;
            if (msg.iInteger > 0)
            {
                TBuf<5> buf;

                buf.AppendNum(msg.iInteger);

                gc.UseFont(notifyFont);
                x += 6;
                gc.DrawText(buf, TPoint(x, y + notifyFont->AscentInPixels()));
                x += notifyFont->TextWidthInPixels(buf);
            }
        }
    }

    if (iHost->GetIndicatorPayload(EScreensaverIndicatorIndexNewMissedCalls, msg) == KErrNone) {
        if (msg.iIsDisplayed > 0 && msg.iInteger > 0)
        {
            /*
            CFbsBitmap* bmp = msg.iIcon->Bitmap();
            gc.BitBlt(TPoint(10, 100), bmp);
            */
            x += 10;

            gc.SetPenSize(TSize(2, 2));
            gc.DrawLine(TPoint(x + 12, y + 5), TPoint(x + 12, y));
            gc.DrawLineTo(TPoint(x, y));
            gc.DrawLineTo(TPoint(x, y + 5));
            gc.SetPenSize(TSize(1, 1));
            x += 14;

            if (msg.iInteger > 0)
            {
                TBuf<5> buf;
                buf.AppendNum(msg.iInteger);
                gc.UseFont(notifyFont);
                x += 6;
                gc.DrawText(buf, TPoint(x, y + notifyFont->AscentInPixels()));
            }
        }
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

    TBuf<50> timeString;
    _LIT(KOwnTimeFormat,"%:0%H%:1%T");
    //_LIT(KOwnTimeFormat,"%:0%H%:1%T%:2%S.%*C3%:3");
    now.FormatL(timeString, KOwnTimeFormat);

    TInt xPos = (screenRect.Width() - timeFont->TextWidthInPixels(timeString)) / 2;
    TInt yPos = ((now.DateTime().Hour()*60. + now.DateTime().Minute())/1439)
            * (screenRect.Height() / 2 - KTopMargin - KBottomMargin) + timeFont->AscentInPixels() + KTopMargin;

    TBuf<50> dateString;
    _LIT(KOwnDateFormat,"%F%*E %D/%M/%Y");
    now.FormatL(dateString, KOwnDateFormat);
    TInt xPosDate = (screenRect.Width() - dateFont->TextWidthInPixels(dateString)) / 2;
    TInt yPosDate = yPos + dateFont->AscentInPixels() + KGap;

    for (int i = 0; i < 2; i++) {
        gc.UseFont(timeFont);
        gc.DrawText(timeString, TPoint(xPos, yPos));

        gc.UseFont(dateFont);
        gc.DrawText(dateString, TPoint(xPosDate, yPosDate));
    }

    DrawIndicators(gc, xPosDate, yPosDate + 10);

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

TInt CLaunchSaver::HandleScreensaverEventL( TScreensaverEvent aEvent, TAny* /*aData*/ )
{
    TInt err(KErrNone);
    switch (aEvent)
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
    } // switch ( aEvent )

    return err;
}

