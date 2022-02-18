#define LIB_NAME "FirebaseRemoteConfig"
#define DLIB_LOG_DOMAIN LIB_NAME
#include <dmsdk/dlib/log.h>
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)

#include "luautils.h"
#include "firebase/app.h"
#include "firebase/remote_config.h"
#include "firebase/future.h"
#include "firebase.h"
#include "firebase_remoteconfig.h"


using namespace firebase;
using namespace firebase::remote_config;

static dmScript::LuaCallbackInfo* g_FirebaseRemoteConfigCallback;
static dmArray<FirebaseRemoteConfigEvent> g_FirebaseRemoteConfigEvents;
static dmMutex::HMutex g_FirebaseRemoteConfigEventsMutex;
static bool g_FirebaseRemoteConfig_Initialized = false;

// queue a remote config event for later processing (from the main thread)
static void FirebaseRemoteConfig_QueueEvent(FirebaseRemoteConfigEvent event)
{
	DM_MUTEX_SCOPED_LOCK(g_FirebaseRemoteConfigEventsMutex);
	if(g_FirebaseRemoteConfigEvents.Full())
	{
		g_FirebaseRemoteConfigEvents.OffsetCapacity(2);
	}
	g_FirebaseRemoteConfigEvents.Push(event);
}

// process any queued events
// this will invoke the callback set when remote config was initialised
// called automatically from extension update function
static void FirebaseRemoteConfig_ProcessEvents()
{
	if (g_FirebaseRemoteConfigEvents.Empty())
	{
		return;
	}

	if (!dmScript::IsCallbackValid(g_FirebaseRemoteConfigCallback))
	{
		dmLogWarning("RemoteConfig callback is not valid");
		return;
	}

	dmArray<FirebaseRemoteConfigEvent> tmp;
	{
		DM_MUTEX_SCOPED_LOCK(g_FirebaseRemoteConfigEventsMutex);
		tmp.Swap(g_FirebaseRemoteConfigEvents);
	}

	for(uint32_t i = 0; i != tmp.Size(); ++i)
	{
		if (dmScript::SetupCallback(g_FirebaseRemoteConfigCallback))
		{
			lua_State* L = dmScript::GetCallbackLuaContext(g_FirebaseRemoteConfigCallback);
			const FirebaseRemoteConfigEvent event = tmp[i];
			lua_pushnumber(L, event);
			dmScript::PCall(L, 2, 0);
			dmScript::TeardownCallback(g_FirebaseRemoteConfigCallback);
		}
	}
}

// get the remote config instance for this app
static RemoteConfig* FirebaseRemoteConfig_GetInstance()
{
	firebase::App* app = Firebase_GetFirebaseApp();
	return firebase::remote_config::RemoteConfig::GetInstance(app);
}

static int FirebaseRemoteConfig_Init(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 0);
	g_FirebaseRemoteConfigCallback = dmScript::CreateCallback(L, 1);

	FirebaseRemoteConfig_GetInstance()->EnsureInitialized()
	.OnCompletion([](const ::firebase::Future<ConfigInfo>& completed_future){
		if (completed_future.error() == 0)
		{
			g_FirebaseRemoteConfig_Initialized = 1;
			FirebaseRemoteConfig_QueueEvent(CONFIG_INITIALIZED);
		}
		else
		{
			dmLogError("RemoteConfig %d %s", completed_future.error(), completed_future.error_message());
			FirebaseRemoteConfig_QueueEvent(CONFIG_ERROR);
		}
	});
	return 0;
}

static int FirebaseRemoteConfig_Fetch(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 0);
	if (!g_FirebaseRemoteConfig_Initialized)
	{
		dmLogWarning("Firebase Remote Config has not been initialized! Make sure to call firebase.remoteconfig.init().");
		return 0;
	}
	FirebaseRemoteConfig_GetInstance()->Fetch()
	.OnCompletion([](const ::firebase::Future<void>& completed_future){
		if (completed_future.error() == 0)
		{
			FirebaseRemoteConfig_QueueEvent(CONFIG_FETCHED);
		}
		else
		{
			dmLogError("RemoteConfig %d %s", completed_future.error(), completed_future.error_message());
			FirebaseRemoteConfig_QueueEvent(CONFIG_ERROR);
		}
	});
	return 0;
}

static int FirebaseRemoteConfig_Activate(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 0);
	if (!g_FirebaseRemoteConfig_Initialized)
	{
		dmLogWarning("Firebase Remote Config has not been initialized! Make sure to call firebase.remoteconfig.init().");
		return 0;
	}
	FirebaseRemoteConfig_GetInstance()->Activate()
	.OnCompletion([](const ::firebase::Future<bool>& completed_future){
		if (completed_future.error() == 0)
		{
			FirebaseRemoteConfig_QueueEvent(CONFIG_ACTIVATED);
		}
		else
		{
			dmLogError("RemoteConfig %d %s", completed_future.error(), completed_future.error_message());
			FirebaseRemoteConfig_QueueEvent(CONFIG_ERROR);
		}
	});
	return 0;
}

static int FirebaseRemoteConfig_FetchAndActivate(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 0);
	if (!g_FirebaseRemoteConfig_Initialized)
	{
		dmLogWarning("Firebase Remote Config has not been initialized! Make sure to call firebase.remoteconfig.init().");
		return 0;
	}
	FirebaseRemoteConfig_GetInstance()->FetchAndActivate()
	.OnCompletion([](const ::firebase::Future<bool>& completed_future){
		if (completed_future.error() == 0)
		{
			FirebaseRemoteConfig_QueueEvent(CONFIG_FETCHED);
			FirebaseRemoteConfig_QueueEvent(CONFIG_ACTIVATED);
		}
		else
		{
			dmLogError("RemoteConfig %d %s", completed_future.error(), completed_future.error_message());
			FirebaseRemoteConfig_QueueEvent(CONFIG_ERROR);
		}
	});
	return 0;
}

static int FirebaseRemoteConfig_GetBoolean(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 1);
	if (!g_FirebaseRemoteConfig_Initialized)
	{
		dmLogWarning("Firebase Remote Config has not been initialized! Make sure to call firebase.remoteconfig.init().");
		return 0;
	}
	const char* key = luaL_checkstring(L, 1);
	bool value = FirebaseRemoteConfig_GetInstance()->GetBoolean(key);
	lua_pushboolean(L, value);
	return 1;
}

static int FirebaseRemoteConfig_GetData(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 1);
	if (!g_FirebaseRemoteConfig_Initialized)
	{
		dmLogWarning("Firebase Remote Config has not been initialized! Make sure to call firebase.remoteconfig.init().");
		return 0;
	}
	const char* key = luaL_checkstring(L, 1);
	std::vector<unsigned char> value = FirebaseRemoteConfig_GetInstance()->GetData(key);
	unsigned char* v = new unsigned char[value.size()];
	std::copy(value.begin(), value.end(), v);
	lua_pushlstring(L, (char*)v, value.size());
	delete [] v;
	return 1;
}

static int FirebaseRemoteConfig_GetNumber(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 1);
	if (!g_FirebaseRemoteConfig_Initialized)
	{
		dmLogWarning("Firebase Remote Config has not been initialized! Make sure to call firebase.remoteconfig.init().");
		return 0;
	}
	const char* key = luaL_checkstring(L, 1);
	double value = FirebaseRemoteConfig_GetInstance()->GetDouble(key);
	lua_pushnumber(L, value);
	return 1;
}

static int FirebaseRemoteConfig_GetString(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 1);
	if (!g_FirebaseRemoteConfig_Initialized)
	{
		dmLogWarning("Firebase Remote Config has not been initialized! Make sure to call firebase.remoteconfig.init().");
		return 0;
	}
	const char* key = luaL_checkstring(L, 1);

	std::string value = FirebaseRemoteConfig_GetInstance()->GetString(key);
	lua_pushstring(L, value.c_str());
	return 1;
}

static int FirebaseRemoteConfig_SetDefaults(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 0);
	if (!g_FirebaseRemoteConfig_Initialized)
	{
		dmLogWarning("Firebase Remote Config has not been initialized! Make sure to call firebase.remoteconfig.init().");
		return 0;
	}

	// populate this vector with default key value pairs
	std::vector<remote_config::ConfigKeyValueVariant> defaultConfig = {};

	luaL_checktype(L, 1, LUA_TTABLE);

	// iterate through all values in the provided table
	// get each key value pair and transfer to the defaultConfig vector
	lua_pushvalue(L, 1); // push copy of table
	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		lua_pushvalue(L, -2); // copy key
		const char *key = lua_tostring(L, -1);
		const int value_type = lua_type(L, -2);
		switch (value_type) {
			case LUA_TSTRING:
				defaultConfig.push_back({ key, lua_tostring(L, -2) });
			break;
			case LUA_TBOOLEAN:
				defaultConfig.push_back({ key, lua_toboolean(L, -2) != 0 });
			break;
			case LUA_TNUMBER:
				defaultConfig.push_back({ key, lua_tonumber(L, -2) });
			break;
			default:  /* other values */
				char msg[256];
				snprintf(msg, sizeof(msg), "Wrong type for table attribute '%s' , type: '%s'", key, luaL_typename(L, -2));
				luaL_error(L, msg);
				lua_pop(L, 3); // pop key, value and table copy
				return 0;
			break;
		}
		lua_pop(L, 2); // pop key and value
	}
	lua_pop(L, 1); // pop table copy

	// set the default config
	FirebaseRemoteConfig_GetInstance()->SetDefaults(defaultConfig.data(), defaultConfig.size())
	.OnCompletion([](const ::firebase::Future<void>& completed_future) {
		if (completed_future.error() == 0)
		{
			FirebaseRemoteConfig_QueueEvent(CONFIG_DEFAULTS_SET);
		}
		else
		{
			dmLogError("RemoteConfig %d %s", completed_future.error(), completed_future.error_message());
			FirebaseRemoteConfig_QueueEvent(CONFIG_ERROR);
		}
	});
	return 0;
}

static int FirebaseRemoteConfig_SetMinimumFetchInterval(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 0);
	if (!g_FirebaseRemoteConfig_Initialized)
	{
		dmLogWarning("Firebase Remote Config has not been initialized! Make sure to call firebase.remoteconfig.init().");
		return 0;
	}

	ConfigSettings settings = FirebaseRemoteConfig_GetInstance()->GetConfigSettings();
	settings.minimum_fetch_interval_in_milliseconds = luaL_checkint(L, 1);
	
	FirebaseRemoteConfig_GetInstance()->SetConfigSettings(settings)
		.OnCompletion([](const ::firebase::Future<void>& completed_future) {
		if (completed_future.error() == 0)
		{
			FirebaseRemoteConfig_QueueEvent(SETTINGS_UPDATED);
		}
		else
		{
			dmLogError("RemoteConfig %d %s", completed_future.error(), completed_future.error_message());
			FirebaseRemoteConfig_QueueEvent(SETTINGS_ERROR);
		}
	});
	return 0;
}

static void LuaInit(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 0);

	// get or create firebase global table
	lua_getglobal(L, "firebase");
	if (lua_isnil(L, lua_gettop(L)))
	{
		lua_pushstring(L, "firebase");
		lua_newtable(L);
		lua_settable(L, LUA_GLOBALSINDEX);
		lua_pop(L, 1);
		lua_getglobal(L, "firebase");
	}

	lua_pushstring(L, "remoteconfig");
	lua_newtable(L);
	lua_pushtablestringfunction(L, "init", FirebaseRemoteConfig_Init);
	lua_pushtablestringfunction(L, "set_defaults", FirebaseRemoteConfig_SetDefaults);
	lua_pushtablestringfunction(L, "set_minimum_fetch_interval", FirebaseRemoteConfig_SetMinimumFetchInterval);
	lua_pushtablestringfunction(L, "get_boolean", FirebaseRemoteConfig_GetBoolean);
	lua_pushtablestringfunction(L, "get_data", FirebaseRemoteConfig_GetData);
	lua_pushtablestringfunction(L, "get_number", FirebaseRemoteConfig_GetNumber);
	lua_pushtablestringfunction(L, "get_string", FirebaseRemoteConfig_GetString);

	lua_pushtablestringfunction(L, "fetch", FirebaseRemoteConfig_Fetch);
	lua_pushtablestringfunction(L, "activate", FirebaseRemoteConfig_Activate);
	lua_pushtablestringfunction(L, "fetch_and_activate", FirebaseRemoteConfig_FetchAndActivate);

	lua_setfieldstringnumber(L, "CONFIG_INITIALIZED", CONFIG_INITIALIZED);
	lua_setfieldstringnumber(L, "CONFIG_ERROR", CONFIG_ERROR);
	lua_setfieldstringnumber(L, "CONFIG_DEFAULTS_SET", CONFIG_DEFAULTS_SET);
	lua_setfieldstringnumber(L, "CONFIG_FETCHED", CONFIG_FETCHED);
	lua_setfieldstringnumber(L, "CONFIG_ACTIVATED", CONFIG_ACTIVATED);
	lua_setfieldstringnumber(L, "SETTINGS_UPDATED", SETTINGS_UPDATED);
	lua_setfieldstringnumber(L, "SETTINGS_ERROR", SETTINGS_ERROR);

	lua_settable(L, -3);

	lua_pop(L, 1); // pop "firebase" global table
}

#endif

dmExtension::Result AppInitializeFirebaseRemoteConfigExtension(dmExtension::AppParams* params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeFirebaseRemoteConfigExtension(dmExtension::Params* params) {
	#if !defined(DM_PLATFORM_ANDROID) && !defined(DM_PLATFORM_IOS)
	dmLogInfo("Extension %s is not supported", LIB_NAME);
	#else
	g_FirebaseRemoteConfigEventsMutex = dmMutex::New();
	LuaInit(params->m_L);
	#endif
	return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeFirebaseRemoteConfigExtension(dmExtension::AppParams* params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeFirebaseRemoteConfigExtension(dmExtension::Params* params) {
	#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)
	if (g_FirebaseRemoteConfigCallback)
	{
		dmScript::DestroyCallback(g_FirebaseRemoteConfigCallback);
		g_FirebaseRemoteConfigCallback = 0;
	}
	if (g_FirebaseRemoteConfigEventsMutex)
	{
		dmMutex::Delete(g_FirebaseRemoteConfigEventsMutex);
		g_FirebaseRemoteConfigEventsMutex = 0;
	}
	#endif
	return dmExtension::RESULT_OK;
}

dmExtension::Result UpdateFirebaseRemoteConfigExtension(dmExtension::Params* params) {
	#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)
	FirebaseRemoteConfig_ProcessEvents();
	#endif
	return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(FirebaseRemoteConfig, LIB_NAME, AppInitializeFirebaseRemoteConfigExtension, AppFinalizeFirebaseRemoteConfigExtension, InitializeFirebaseRemoteConfigExtension, UpdateFirebaseRemoteConfigExtension, 0, FinalizeFirebaseRemoteConfigExtension)
