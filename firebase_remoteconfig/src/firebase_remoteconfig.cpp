#define EXTENSION_NAME FirebaseRemoteConfigExt
#define LIB_NAME "FirebaseRemoteConfig"
#define MODULE_NAME "firebase"

#define DLIB_LOG_DOMAIN LIB_NAME
#include <stdlib.h>

#include <dmsdk/dlib/log.h>
#include <dmsdk/sdk.h>

#include "firebase_remoteconfig_private.h"
#include "firebase_remoteconfig_callback.h"

#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)

namespace dmFirebaseRemoteConfig {

static int Lua_Initialize(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    Initialize();
    return 0;
}

static int Lua_SetCallback(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    SetLuaCallback(L, 1);
    return 0;
}


static int Lua_SetDefaults(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    luaL_checktype(L, 1, LUA_TTABLE);
    char* json = 0;
    size_t json_length = 0;
    if (dmScript::LuaToJson(L, &json, &json_length))
    {
        SetDefaults((const char*) json);
        free(json);
    }
    return 0;
}

static int Lua_SetMinimumFetchInterval(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    double interval = luaL_checknumber(L, 1);
    SetMinimumFetchInterval(interval);
    return 0;
}

static int Lua_SetTimeout(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    double timeout = luaL_checknumber(L, 1);
    SetTimeout(timeout);
    return 0;
}

    
static int Lua_GetBoolean(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    const char* key = luaL_checkstring(L, 1);
    bool value = GetBoolean(key);
    lua_pushboolean(L, value);
    return 1;
}

static int Lua_GetData(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    const char* key = luaL_checkstring(L, 1);
    char* value = GetData(key);
    lua_pushstring(L, value);
    free(value);
    return 1;
}

static int Lua_GetKeys(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    char* jsonStr = GetKeys();
    dmScript::JsonToLua(L, jsonStr, strlen(jsonStr));
    free(jsonStr);
    return 1;
}

static int Lua_GetNumber(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    const char* key = luaL_checkstring(L, 1);
    double value = GetNumber(key);
    lua_pushnumber(L, value);
    return 1;
}

static int Lua_GetString(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    const char* key = luaL_checkstring(L, 1);
    char* value = GetString(key);
    lua_pushstring(L, value);
    free(value);
    return 1;
}

static int Lua_Fetch(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    Fetch();
    return 0;
}

static int Lua_Activate(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    Activate();
    return 0;
}

static int Lua_FetchAndActivate(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    FetchAndActivate();
    return 0;
}

static const luaL_reg Module_methods[] =
{
    {"initialize", Lua_Initialize},
    {"set_callback", Lua_SetCallback},

    {"set_defaults", Lua_SetDefaults},
    {"set_minimum_fetch_interval", Lua_SetMinimumFetchInterval},
    {"set_timeout", Lua_SetTimeout},
    
    {"get_boolean", Lua_GetBoolean},
    {"get_data", Lua_GetData},
    {"get_number", Lua_GetNumber},
    {"get_string", Lua_GetString},
    {"get_keys", Lua_GetKeys},

    {"fetch", Lua_Fetch},
    {"activate", Lua_Activate},
    {"fetch_and_activate", Lua_FetchAndActivate},
    {0, 0}
};

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

    luaL_register(L, NULL, Module_methods);
    

    #define SETCONSTANT(name) \
    lua_pushnumber(L, (lua_Number) name); \
    lua_setfield(L, -2, #name); \

    SETCONSTANT(MSG_ERROR)
    SETCONSTANT(MSG_INITIALIZED)
    SETCONSTANT(MSG_DEFAULTS_SET)
    SETCONSTANT(MSG_SETTINGS_UPDATED)
    SETCONSTANT(MSG_FETCHED)
    SETCONSTANT(MSG_ACTIVATED)

    #undef SETCONSTANT

    lua_settable(L, -3);

    lua_pop(L, 1);
}


//--------------------------- Extension callbacks -------------------------//

dmExtension::Result AppInitializeFirebaseExtension(dmExtension::AppParams* params) {
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeFirebaseExtension(dmExtension::Params* params) {
    dmLogInfo("Initialize Firebase Remote Config");

    LuaInit(params->m_L);
    Initialize_Ext();
    InitializeCallback();

    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeFirebaseExtension(dmExtension::AppParams* params) {
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeFirebaseExtension(dmExtension::Params* params) {
    FinalizeCallback();
    return dmExtension::RESULT_OK;
}

dmExtension::Result UpdateFirebaseExtension(dmExtension::Params* params) {
    UpdateCallback();
    return dmExtension::RESULT_OK;
}

} // namespace dmFirebaseRemoteConfig

DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, dmFirebaseRemoteConfig::AppInitializeFirebaseExtension, dmFirebaseRemoteConfig::AppFinalizeFirebaseExtension, dmFirebaseRemoteConfig::InitializeFirebaseExtension, dmFirebaseRemoteConfig::UpdateFirebaseExtension, 0, dmFirebaseRemoteConfig::FinalizeFirebaseExtension)

#else // defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS) 

//-------------------- Not supported for other platforms --------------------//

static  dmExtension::Result InitializeFirebase(dmExtension::Params* params)
{
    dmLogInfo("Registered extension Firebase Remote Config (null)");
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeFirebase(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, 0, 0, InitializeFirebase, 0, 0, FinalizeFirebase)

#endif // defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS) 

