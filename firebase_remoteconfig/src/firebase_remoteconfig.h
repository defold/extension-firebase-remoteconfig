#pragma once

#include <dmsdk/sdk.h>

enum FirebaseRemoteConfigEvent
{
    CONFIG_INITIALIZED = 0,
    CONFIG_ERROR = 1,
    CONFIG_DEFAULTS_SET = 2,
    CONFIG_FETCHED = 3,
    CONFIG_ACTIVATED = 4,
};
