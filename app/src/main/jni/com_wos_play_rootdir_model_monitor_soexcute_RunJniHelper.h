
#include <jni.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>
#include <android/log.h>
#define  TAG "LeePCLibs"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

#ifndef _Included_com_wos_play_rootdir_model_monitor_soexcute_RunJniHelper
#define _Included_com_wos_play_rootdir_model_monitor_soexcute_RunJniHelper
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_wos_play_rootdir_model_monitor_soexcute_RunJniHelper
 * Method:    startMservice
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_startMservice
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     com_wos_play_rootdir_model_monitor_soexcute_RunJniHelper
 * Method:    stopMservice
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_stopMservice
  (JNIEnv *, jobject, jstring);

#ifdef __cplusplus
}
#endif
#endif
