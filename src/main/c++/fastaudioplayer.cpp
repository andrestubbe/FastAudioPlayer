/**
 * @file fastaudioplayer.cpp
 * @brief Native Windows audio playback using WASAPI
 * 
 * Stub implementation for v1.0.0
 * Full WASAPI integration coming in v1.1
 */

#include <jni.h>
#include <windows.h>
#include <string>

// Helper: Convert UTF-8 to wide string
std::wstring UTF8ToWString(JNIEnv* env, jstring str) {
    if (!str) return L"";
    const char* chars = env->GetStringUTFChars(str, nullptr);
    if (!chars) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, chars, -1, nullptr, 0);
    std::wstring result(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, chars, -1, &result[0], len);
    env->ReleaseStringUTFChars(str, chars);
    return result;
}

extern "C" {

JNIEXPORT jlong JNICALL Java_fastaudio_FastAudioPlayer_createPlayer(JNIEnv* env, jclass clazz) {
    // Stub: Return dummy handle
    return 1;
}

JNIEXPORT void JNICALL Java_fastaudio_FastAudioPlayer_destroyPlayer(JNIEnv* env, jclass clazz, jlong handle) {
    // Stub: Nothing to do
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_loadFile(JNIEnv* env, jclass clazz, jlong handle, jstring filePath) {
    // Stub: Always return true
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_play(JNIEnv* env, jclass clazz, jlong handle) {
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_pause(JNIEnv* env, jclass clazz, jlong handle) {
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_resume(JNIEnv* env, jclass clazz, jlong handle) {
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_stop(JNIEnv* env, jclass clazz, jlong handle) {
    return JNI_TRUE;
}

JNIEXPORT void JNICALL Java_fastaudio_FastAudioPlayer_setVolume(JNIEnv* env, jclass clazz, jlong handle, jfloat volume) {
    // Stub: Nothing to do
}

JNIEXPORT jfloat JNICALL Java_fastaudio_FastAudioPlayer_getVolume(JNIEnv* env, jclass clazz, jlong handle) {
    return 1.0f;
}

JNIEXPORT jlong JNICALL Java_fastaudio_FastAudioPlayer_getPosition(JNIEnv* env, jclass clazz, jlong handle) {
    return 0;
}

JNIEXPORT jlong JNICALL Java_fastaudio_FastAudioPlayer_getDuration(JNIEnv* env, jclass clazz, jlong handle) {
    return 0;
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_isPlaying(JNIEnv* env, jclass clazz, jlong handle) {
    return JNI_FALSE;
}

JNIEXPORT jobjectArray JNICALL Java_fastaudio_FastAudioPlayer_getDevices(JNIEnv* env, jclass clazz) {
    jobjectArray result = env->NewObjectArray(1, env->FindClass("java/lang/String"), nullptr);
    jstring defaultDevice = env->NewStringUTF("Default");
    env->SetObjectArrayElement(result, 0, defaultDevice);
    env->DeleteLocalRef(defaultDevice);
    return result;
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_setDevice(JNIEnv* env, jclass clazz, jlong handle, jstring deviceId) {
    return JNI_TRUE;
}

} // extern "C"
