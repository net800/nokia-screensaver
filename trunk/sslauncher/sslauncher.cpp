/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#include <ecom/ImplementationProxy.h>
#include <apgtask.h>
#include <apgcli.h>
#include <eikenv.h>

#include "sslauncher.h"
#include "sslauncher.hrh"

const TUid KUidSaverExe = TUid::Uid( SSAVER_UID3 );
const TInt KLightsOnTimeoutInterval = 30;

_LIT( KSaverName, "Screensaver" );
TBuf<KMaxUidName + 20> FullName;

/** 
 * This class implements Symbian screensaver plugin using ECOM plugin interface.
 * ECOM interface is returned by ImplementationGroupProxy method, which is
 * exported as ordinal 1 to the DLL we are producing.  ECOM simply gives a
 * pointer to factory method, which creates an instance of CLaunchSaver class.
 *
 * CLaunchSaver extends CScreensaverPluginInterfaceDefinition, but it does not
 * really do anything in the drawing section.  Instead, it just switches to
 * powersaving mode and launches a separate Qt application to take over the
 * rendering.
 *
 * Screensaver host limits backlight time to 30 seconds, so every 30 seconds we
 * have a timeout, which just sets new timeout.
 *
 */


// Define the interface implementors
const TImplementationProxy ImplementationTable[] =  {
    { {SSLAUNCHER_UID3 }, ( TProxyNewLPtr )CLaunchSaver::NewL}
};

EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
{
    aTableCount = sizeof( ImplementationTable ) / sizeof( TImplementationProxy );
    return ImplementationTable;
}

CLaunchSaver* CLaunchSaver::NewL()
{
    CLaunchSaver* self = new ( ELeave ) CLaunchSaver();
    return self;
}

TInt CLaunchSaver::InitializeL( MScreensaverPluginHost* aHost )
{
    iHost = aHost;
    return KErrNone;
}

TInt CLaunchSaver::Draw( CWindowGc& aGc )
{
    // Do nothing, separate process does the visuals
    return KErrNone;
}

const TDesC16& CLaunchSaver::Name() const
{
    if (FullName.Length() == 0)
    {
        FullName.Append(KSaverName);
        FullName.Append(' ');
        FullName.Append(KUidSaverExe.Name());
    }
    return FullName;
}

TInt CLaunchSaver::HandleScreensaverEventL( TScreensaverEvent aEvent, TAny* /*aData*/ )
{
    TInt err( KErrNone );
    
    switch ( aEvent )
    {
        case EScreensaverEventTimeout:
        {
            // Keep lights on
            iHost->RequestTimeout( KLightsOnTimeoutInterval );
            break;
        }
        case EScreensaverEventStarting:
        {
            // Launch new process
            RApaLsSession apaLsSession;
            TInt err = apaLsSession.Connect();
            
            if ( err == KErrNone )
            {
                TThreadId thread;
                err = apaLsSession.StartDocument( KNullDesC, KUidSaverExe, thread );
                apaLsSession.Close();
            }

            // Switch to partial mode to save power
            TScreensaverPartialMode partial;
            partial.iType = EPartialModeTypeMostPowerSaving;

            TInt height = CEikonEnv::Static()->ScreenDevice()->SizeInPixels().iHeight;
            TInt width = CEikonEnv::Static()->ScreenDevice()->SizeInPixels().iWidth;
            TInt lastRow = Max(height, width) - 1;
            iHost->SetActiveDisplayArea(0, lastRow, partial);

            if( err == KErrNone )
            {
                iHost->RequestTimeout( KLightsOnTimeoutInterval );
            }
            
            break;
        }
        case EScreensaverEventStopping:
        {
            // Kill saver process
            TApaTaskList list( CEikonEnv::Static()->WsSession() );
            TApaTask task = list.FindApp( KUidSaverExe );

            if ( task.Exists() )
            {
                task.EndTask();
            }
            break;
        }
        default:
        {
            break;
        }
    } // switch ( aEvent )

    return err;
}

CLaunchSaver::CLaunchSaver()
{
}
