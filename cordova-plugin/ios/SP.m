#import <Cordova/CDVPlugin.h>

#include "json-sqlite-engine-wrapper-pro.h"

@interface SP : CDVPlugin

- (void) openDatabaseHandle:(CDVInvokedUrlCommand *)c;
- (void) bb:(CDVInvokedUrlCommand *)c;

@end

@implementation SP

- (void) openDatabaseHandle:(CDVInvokedUrlCommand *)c
{
  if ([c.arguments count] == 0) {
    [self.commandDelegate
      sendPluginResult :
          [CDVPluginResult resultWithStatus : CDVCommandStatus_OK
                            messageAsArray  : @[[NSNull null]]]
      callbackId : c.callbackId];
    return;
  }

  const jsql_handle_t mydbhandle = jsql_pro_db_open(JSQL_PRO_API_VERSION, [[c.arguments objectAtIndex: 0] UTF8String], JSQL_OPEN_READWRITE | JSQL_OPEN_CREATE | JSQL_OPEN_MEMORY);

  [self.commandDelegate
    sendPluginResult :
        [CDVPluginResult resultWithStatus : CDVCommandStatus_OK
                          messageAsArray  : @[@(mydbhandle)]]
    callbackId : c.callbackId];
}

- (void) jsonExecuteStatement:(CDVInvokedUrlCommand *)c
{
  if ([c.arguments count] < 2) {
    [self.commandDelegate
      sendPluginResult :
          [CDVPluginResult resultWithStatus : CDVCommandStatus_OK
                            messageAsArray  : @[[NSNull null]]]
      callbackId : c.callbackId];
    return;
  }

  const char * rj = jsql_pro_execute([[c.arguments objectAtIndex: 0] longLongValue], [[c.arguments objectAtIndex: 1] UTF8String], 0);

  [self.commandDelegate
    sendPluginResult :
        [CDVPluginResult resultWithStatus : CDVCommandStatus_OK
                          messageAsArray  : @[@(rj)]]
    callbackId : c.callbackId];

  jsql_pro_internal_cleanup([[c.arguments objectAtIndex: 0] longLongValue]);
}

@end
