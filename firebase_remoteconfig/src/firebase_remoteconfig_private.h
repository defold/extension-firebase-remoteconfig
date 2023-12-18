#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)

#pragma once

namespace dmFirebaseRemoteConfig {

static const double kDefaultCacheExpirationInSeconds = 60 * 60 * 12;
static const double kDefaultTimeoutInSeconds = 30;

void Initialize_Ext();
	
void Initialize();

void SetDefaults(); // TODO

void SetMinimumFetchInterval(double fetchInterval); // in seconds
void SetTimeout(double timeOut); // in seconds
	
bool GetBoolean(const char* key);
double GetNumber(const char* key);
char * GetData(const char* key); // free() need to be called after data is pushed into lua
char * GetString(const char* key); // free() need to be called after data is pushed into lua

void Fetch();
void Activate();
void FetchAndActivate();

} //namespace dmFirebaseRemoteConfig

#endif

