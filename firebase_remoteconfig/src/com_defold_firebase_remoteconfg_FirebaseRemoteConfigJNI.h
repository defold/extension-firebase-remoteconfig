#if defined(DM_PLATFORM_ANDROID)

#include <jni.h>
/* Header for class com_defold_firebase_FirebaseJNI */

#ifndef COM_DEFOLD_FIREBASE_REMOTECONFIG_FIREBASEREMOTECONFIGJNI_H
#define COM_DEFOLD_FIREBASE_REMOTECONFIG_FIREBASEREMOTECONFIGJNI_H
#ifdef __cplusplus
extern "C" {
#endif
    /*
    * Class:     com_defold_firebase_remoteconfig_FirebaseRemoteConfigJNI
    * Method:    firebaseAddToQueue_first_arg
    * Signature: (ILjava/lang/String;I)V
    */
                           
    JNIEXPORT void JNICALL Java_com_defold_firebase_remoteconfig_FirebaseRemoteConfigJNI_firebaseAddToQueue
        (JNIEnv *, jclass, jint, jstring);

#ifdef __cplusplus
}
#endif
#endif

#endif
