/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class ffmpeg_jni_VideoThumbnail */

#ifndef _Included_ffmpeg_jni_VideoThumbnail
#define _Included_ffmpeg_jni_VideoThumbnail
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     ffmpeg_jni_VideoThumbnail
 * Method:    getVideoThumb
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_ffmpeg_1jni_VideoThumbnail_getVideoThumb
  (JNIEnv *, jobject, jstring);
static void logging(const char *fmt, ...);
int getVideoThumb(const char* inputFileName);
#ifdef __cplusplus
}
#endif
#endif