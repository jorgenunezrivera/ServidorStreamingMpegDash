/*
 * Class:     ffmpeg_jni_VideoDash
 * Method:    getVideoMp4
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
#include <jni.h>

JNIEXPORT jint JNICALL Java_ffmpeg_1jni_VideoDash_getVideoMp4
  (JNIEnv *, jobject, jstring, jstring,jint);
JNIEXPORT jstring JNICALL Java_ffmpeg_1jni_VideoDash_getVideoInfo(JNIEnv *env, jobject obj, jstring jFilename);
int get_num_streams(char* filename);
int remux_mp4(char* infilename,char* outfilename ,int stream_index);
