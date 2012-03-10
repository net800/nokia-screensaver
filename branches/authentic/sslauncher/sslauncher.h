/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <screensaverpluginintdef.h>

class CLaunchSaver : public CScreensaverPluginInterfaceDefinition
    {
public:
    /**
     * Constructor.
     */
    static CLaunchSaver* NewL();

    /**
     * Destructor.
     */
    ~CLaunchSaver() {}

public: // From MScreensaverPlugin
    virtual TInt InitializeL( MScreensaverPluginHost* aHost );
    virtual TInt Draw( CWindowGc& aGc );
    virtual const TDesC16& Name() const;
    virtual TInt HandleScreensaverEventL( TScreensaverEvent aEvent, TAny* aData );
    virtual TInt Capabilities() { return EScpCapsNone; }
    virtual TInt PluginFunction( TScPluginCaps /*aFunction*/,
                                 TAny* /*aParam*/ ) { return KErrNone; }

private:
    /**
     * Constructor.
     */
    CLaunchSaver();

private: // Data
    MScreensaverPluginHost *iHost;
    };

#endif // LAUNCHER_H
