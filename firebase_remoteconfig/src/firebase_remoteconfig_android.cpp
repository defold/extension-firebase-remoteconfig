#if defined(DM_PLATFORM_ANDROID)

#include <dmsdk/dlib/android.h>
#include "com_defold_firebase_remoteconfg_FirebaseRemoteConfigJNI.h"
#include "firebase_remoteconfig_private.h"
#include "firebase_remoteconfig_callback.h"


JNIEXPORT void JNICALL Java_com_defold_firebase_remoteconfig_FirebaseRemoteConfigJNI_firebaseAddToQueue(JNIEnv * env, jclass cls, jint jmsg, jstring jjson)
{
    const char* json = env->GetStringUTFChars(jjson, 0);
    dmFirebaseRemoteConfig::AddToQueueCallback((dmFirebaseRemoteConfig::Message)jmsg, json);
    env->ReleaseStringUTFChars(jjson, json);
}

namespace dmFirebaseRemoteConfig {

    struct FirebaseRemoteConfigJNI {
        jobject        m_JNI;

        jmethodID      m_Initialize;
        jmethodID      m_SetDefaults;
        jmethodID      m_SetMinimumFetchInterval;
        jmethodID      m_SetTimeout;
        jmethodID      m_GetBoolean;
        jmethodID      m_GetNumber;
        jmethodID      m_GetData;
        jmethodID      m_GetString;
        jmethodID      m_GetKeys;
        jmethodID      m_Fetch;
        jmethodID      m_Activate;
        jmethodID      m_FetchAndActivate;
    };

    static FirebaseRemoteConfigJNI g_firebaseRemoteConfig;

    static void CallVoidMethod(jobject instance, jmethodID method)
    {
        dmAndroid::ThreadAttacher threadAttacher;
        JNIEnv* env = threadAttacher.GetEnv();

        env->CallVoidMethod(instance, method);
    }

    static void CallVoidMethodDouble(jobject instance, jmethodID method, double cdouble)
    {
        dmAndroid::ThreadAttacher threadAttacher;
        JNIEnv* env = threadAttacher.GetEnv();

        env->CallVoidMethod(instance, method, cdouble);
    }

    static bool CallBoolMethodChar(jobject instance, jmethodID method, const char* cstr)
    {
        dmAndroid::ThreadAttacher threadAttacher;
        JNIEnv* env = threadAttacher.GetEnv();

        jstring jstr = env->NewStringUTF(cstr);
        jboolean return_value = (jboolean)env->CallBooleanMethod(instance, method, jstr);
        env->DeleteLocalRef(jstr);
        
        return JNI_TRUE == return_value;
    }

    static double CallDoubleMethodChar(jobject instance, jmethodID method, const char* cstr)
    {
        dmAndroid::ThreadAttacher threadAttacher;
        JNIEnv* env = threadAttacher.GetEnv();

        jstring jstr = env->NewStringUTF(cstr);
        jdouble return_value = env->CallDoubleMethod(instance, method, jstr);
        env->DeleteLocalRef(jstr);
        
        return return_value;
    }

    static char* CallCharMethodChar(jobject instance, jmethodID method, const char* cstr)
    {
        dmAndroid::ThreadAttacher threadAttacher;
        JNIEnv* env = threadAttacher.GetEnv();

        jstring jstr = env->NewStringUTF(cstr);
        jstring return_value = (jstring)env->CallObjectMethod(instance, method, jstr);
        env->DeleteLocalRef(jstr);
        

        const char* result_cstr = env->GetStringUTFChars(return_value, 0);
        char* result_cstr_copy = strdup(result_cstr);
        env->ReleaseStringUTFChars(return_value, result_cstr);
        env->DeleteLocalRef(return_value);

        return result_cstr_copy;
    }

    static char* CallCharMethod(jobject instance, jmethodID method)
    {
        dmAndroid::ThreadAttacher threadAttacher;
        JNIEnv* env = threadAttacher.GetEnv();

        jstring return_value = (jstring)env->CallObjectMethod(instance, method);

        const char* result_cstr = env->GetStringUTFChars(return_value, 0);
        char* result_cstr_copy = strdup(result_cstr);
        env->ReleaseStringUTFChars(return_value, result_cstr);
        env->DeleteLocalRef(return_value);

        return result_cstr_copy;
    }


    static void InitJNIMethods(JNIEnv* env, jclass cls)
    {
        g_firebaseRemoteConfig.m_Initialize = env->GetMethodID(cls, "initialize", "()V");

        g_firebaseRemoteConfig.m_SetDefaults = env->GetMethodID(cls, "setDefaults", "()V");
        g_firebaseRemoteConfig.m_SetMinimumFetchInterval = env->GetMethodID(cls, "setMinimumFetchInterval", "(D)V");
        g_firebaseRemoteConfig.m_SetTimeout = env->GetMethodID(cls, "setTimeout", "(D)V");
        g_firebaseRemoteConfig.m_GetBoolean = env->GetMethodID(cls, "getBoolean", "(Ljava/lang/String;)Z");
        g_firebaseRemoteConfig.m_GetNumber = env->GetMethodID(cls, "getNumber", "(Ljava/lang/String;)D");
        g_firebaseRemoteConfig.m_GetData = env->GetMethodID(cls, "getData", "(Ljava/lang/String;)Ljava/lang/String;");
        g_firebaseRemoteConfig.m_GetString = env->GetMethodID(cls, "getString", "(Ljava/lang/String;)Ljava/lang/String;");
        g_firebaseRemoteConfig.m_GetKeys = env->GetMethodID(cls, "getKeys", "()Ljava/lang/String;");
        g_firebaseRemoteConfig.m_Fetch = env->GetMethodID(cls, "fetch", "()V");
        g_firebaseRemoteConfig.m_Activate = env->GetMethodID(cls, "activate", "()V");
        g_firebaseRemoteConfig.m_FetchAndActivate = env->GetMethodID(cls, "fetchAndActivate", "()V");
    }

    void Initialize_Ext() 
    {
        dmAndroid::ThreadAttacher threadAttacher;
        JNIEnv* env = threadAttacher.GetEnv();

        jclass cls = dmAndroid::LoadClass(env, "com.defold.firebase.remoteconfig.FirebaseRemoteConfigJNI");

        InitJNIMethods(env, cls);

        jmethodID jni_constructor = env->GetMethodID(cls, "<init>", "()V");

        g_firebaseRemoteConfig.m_JNI = env->NewGlobalRef(env->NewObject(cls, jni_constructor, threadAttacher.GetActivity()->clazz));
    }


    void Initialize() 
    {
        CallVoidMethod(g_firebaseRemoteConfig.m_JNI, g_firebaseRemoteConfig.m_Initialize);
    }

    void SetDefaults() 
    {
        CallVoidMethod(g_firebaseRemoteConfig.m_JNI, g_firebaseRemoteConfig.m_SetDefaults);
    }

    void SetMinimumFetchInterval(double fetchInterval)
    {
        CallVoidMethodDouble(g_firebaseRemoteConfig.m_JNI, g_firebaseRemoteConfig.m_SetMinimumFetchInterval, fetchInterval);
    }

    void SetTimeout(double timeOut)
    {
        CallVoidMethodDouble(g_firebaseRemoteConfig.m_JNI, g_firebaseRemoteConfig.m_SetTimeout, timeOut);
    }
    
    bool GetBoolean(const char* key)
    {
        return CallBoolMethodChar(g_firebaseRemoteConfig.m_JNI, g_firebaseRemoteConfig.m_GetBoolean, key);
    }

    double GetNumber(const char* key)
    {
        return CallDoubleMethodChar(g_firebaseRemoteConfig.m_JNI, g_firebaseRemoteConfig.m_GetNumber, key);
    }

    char * GetData(const char* key)
    {
        return CallCharMethodChar(g_firebaseRemoteConfig.m_JNI, g_firebaseRemoteConfig.m_GetData, key);
    }

    char * GetString(const char* key)
    {
        return CallCharMethodChar(g_firebaseRemoteConfig.m_JNI, g_firebaseRemoteConfig.m_GetString, key);
    }

    char * GetKeys()
    {
        return CallCharMethod(g_firebaseRemoteConfig.m_JNI, g_firebaseRemoteConfig.m_GetKeys);
    }

    void Fetch()
    {
        CallVoidMethod(g_firebaseRemoteConfig.m_JNI, g_firebaseRemoteConfig.m_Fetch);
    }

    void Activate()
    {
        CallVoidMethod(g_firebaseRemoteConfig.m_JNI, g_firebaseRemoteConfig.m_Activate);
    }

    void FetchAndActivate()
    {
        CallVoidMethod(g_firebaseRemoteConfig.m_JNI, g_firebaseRemoteConfig.m_FetchAndActivate);        
    }

} //namespace dmAdmob

#endif // DM_PLATFORM_ANDROID
