#if defined(DM_PLATFORM_IOS)

#include <FirebaseRemoteConfig/FirebaseRemoteConfig.h>

#include "firebase_remoteconfig_private.h"
#include "firebase_remoteconfig_callback.h"

namespace dmFirebaseRemoteConfig {

void SendSimpleMessage(Message msg, id obj) {
    NSError* error;
    NSData* jsonData = [NSJSONSerialization dataWithJSONObject:obj options:(NSJSONWritingOptions)0 error:&error];

    if (jsonData)
    {
        NSString* nsstring = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
        AddToQueueCallback(msg, (const char*)[nsstring UTF8String]);
        [nsstring release];
    }
    else
    {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        [dict setObject:error.localizedDescription forKey:@"error"];
        NSError* error2;
        NSData* errorJsonData = [NSJSONSerialization dataWithJSONObject:dict options:(NSJSONWritingOptions)0 error:&error2];
        if (errorJsonData)
        {
            NSString* nsstringError = [[NSString alloc] initWithData:errorJsonData encoding:NSUTF8StringEncoding];
            AddToQueueCallback(MSG_ERROR, (const char*)[nsstringError UTF8String]);
            [nsstringError release];
        }
        else
        {
            AddToQueueCallback(MSG_ERROR, [[NSString stringWithFormat:@"{ \"error\": \"Error while converting simple message to JSON.\"}"] UTF8String]);
        }
    }
}

void SendErrorMessage(NSError *error) 
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    [dict setObject:error.localizedDescription forKey:@"error"];
    SendSimpleMessage(MSG_ERROR, dict);
}

void SendSimpleMessage(Message msg) 
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    SendSimpleMessage(msg, dict);
}

FIRRemoteConfig *g_remoteConfig = 0;

void Initialize_Ext()
{

}
    
void Initialize() 
{
    @try {
        g_remoteConfig = [FIRRemoteConfig remoteConfig];
        
        FIRRemoteConfigSettings *remoteConfigSettings = [[FIRRemoteConfigSettings alloc] init];
        remoteConfigSettings.minimumFetchInterval = kDefaultCacheExpirationInSeconds;
        remoteConfigSettings.fetchTimeout = kDefaultTimeoutInSeconds;

        g_remoteConfig.configSettings = remoteConfigSettings;

        SendSimpleMessage(MSG_INITIALIZED);
    } @catch (NSException* e) {
        AddToQueueCallback(MSG_ERROR, [[NSString stringWithFormat:@"{ \"error\": \"Unable to init Firebase Remote Config (%1$@)\"}", e.reason] UTF8String]);
    }
}

void SetDefaults() 
{
    //TODO
}

void SetMinimumFetchInterval(double fetchInterval) 
{
    FIRRemoteConfigSettings *remoteConfigSettings = [[FIRRemoteConfigSettings alloc] init];
    remoteConfigSettings.minimumFetchInterval = fetchInterval;
    remoteConfigSettings.fetchTimeout = g_remoteConfig.configSettings.fetchTimeout;
    g_remoteConfig.configSettings = remoteConfigSettings;
    SendSimpleMessage(MSG_SETTINGS_UPDATED);
}

void SetTimeout(double timeOut) 
{
    FIRRemoteConfigSettings *remoteConfigSettings = [[FIRRemoteConfigSettings alloc] init];
    remoteConfigSettings.minimumFetchInterval = g_remoteConfig.configSettings.minimumFetchInterval;
    remoteConfigSettings.fetchTimeout = timeOut;
    g_remoteConfig.configSettings = remoteConfigSettings;
    SendSimpleMessage(MSG_SETTINGS_UPDATED);
}

void SetMinimumFetchInterval(int interval) 
{
    g_remoteConfig.configSettings.minimumFetchInterval = interval;
}
    
bool GetBoolean(const char* key) 
{
    FIRRemoteConfigValue *value = [g_remoteConfig configValueForKey:@(key)];
    return static_cast<bool>(value.boolValue);
}

double GetNumber(const char* key) 
{
    FIRRemoteConfigValue *value = [g_remoteConfig configValueForKey:@(key)];
    return value.numberValue.doubleValue;
}

char* GetData(const char* key) 
{
    FIRRemoteConfigValue *value = [g_remoteConfig configValueForKey:@(key)];
    NSData *data = value.dataValue;
    
    int size = [data length] / sizeof(char);
    char* result = static_cast<char*>(malloc(size + 1));
    memcpy(result, [data bytes], size);
    result[size] = 0;

    return result;
}

char* GetString(const char* key) 
{
    FIRRemoteConfigValue *value = [g_remoteConfig configValueForKey:@(key)];

    const char* result = [value.stringValue cStringUsingEncoding:NSUTF8StringEncoding];
    return strdup(result);
}

char * GetKeys()
{
    NSArray *jsonArray = [g_remoteConfig allKeysFromSource:FIRRemoteConfigSourceRemote];
    NSError *error;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:jsonArray options:0 error:&error];
    if (!jsonData) {
        AddToQueueCallback(MSG_ERROR, [[NSString stringWithFormat:@"{ \"error\": \"Unable to get keys (%1$@)\"}", [error code]] UTF8String]);
        NSString *empty = @"{}";
        return strdup([empty UTF8String]);
    }
    NSString *jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    char *copiedString = strdup([jsonString UTF8String]);
    [jsonString release];
    return copiedString;
}

void Fetch() 
{
    @try {
        [g_remoteConfig fetchWithCompletionHandler:^(FIRRemoteConfigFetchStatus status, NSError *error) {
            if (status == FIRRemoteConfigFetchStatusSuccess) {
                SendSimpleMessage(MSG_FETCHED);
            } else {
                SendErrorMessage(error);
            }
        }];
    } @catch (NSException* e) {
        AddToQueueCallback(MSG_ERROR, [[NSString stringWithFormat:@"{ \"error\": \"Unable to fetch Remote Config (%1$@)\"}", e.reason] UTF8String]);
    }
}

void Activate() 
{
    @try {
        [g_remoteConfig activateWithCompletion:^(BOOL changed, NSError * _Nullable error) {
            if (error == nil) {
                SendSimpleMessage(MSG_ACTIVATED);
            } else {
                SendErrorMessage(error);
            }
        }];
    } @catch (NSException* e) {
        AddToQueueCallback(MSG_ERROR, [[NSString stringWithFormat:@"{ \"error\": \"Unable to activate Remote Config (%1$@)\"}", e.reason] UTF8String]);
    }
}

void FetchAndActivate() 
{
    @try {
        [g_remoteConfig fetchAndActivateWithCompletionHandler:^(FIRRemoteConfigFetchAndActivateStatus status, NSError *error) {
            if (status != FIRRemoteConfigFetchAndActivateStatusError) {
                SendSimpleMessage(MSG_FETCHED);
                SendSimpleMessage(MSG_ACTIVATED);
            } else {
                SendErrorMessage(error);
            }
        }];
    } @catch (NSException* e) {
        AddToQueueCallback(MSG_ERROR, [[NSString stringWithFormat:@"{ \"error\": \"Unable to fetch and activate Remote Config (%1$@)\"}", e.reason] UTF8String]);
    }
}


} // namespace dmFirebaseRemoteConfig

#endif //defined(DM_PLATFORM_IOS)
