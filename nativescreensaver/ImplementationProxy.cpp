#include "screensaver.hrh"
#include "screensaver.h"
#include <ecom/ImplementationProxy.h>

// Define the interface implementors
const TImplementationProxy ImplementationTable[] =  {
        { {SCREENSAVER_UID3 }, ( TProxyNewLPtr )CLaunchSaver::NewL}
    };

/*
 * Helps the ECom framework locating the interface implementations
 *
 */

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount) {
  aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
  return ImplementationTable;
}