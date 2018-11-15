#include <jni.h>
#include <string>
#include "AudioResample.h"
#include "AILog.h"

extern "C" JNIEXPORT jstring

JNICALL
Java_com_example_administrator_audiodecoder_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_administrator_audiodecoder_NativeTranscode_nativeInitResampler(JNIEnv *env,
                                                                                jobject instance,
                                                                                jstring outFilepath_,
                                                                                jint samples,
                                                                                jint inChannels,
                                                                                jint inSampleRate,
                                                                                jint sampleFmt,
                                                                                jint outChannnels,
                                                                                jint outSampleRate,
                                                                                jint outSampleFmt) {
    AILog::getInstance()->setLogLevel(AILog::AIDEBUG);
    const char *outFilepath = env->GetStringUTFChars(outFilepath_, 0);
    LOGI("nativeInitResampler");
    // TODO
    AudioResample *resample = new AudioResample;
    resample->initResample(outFilepath, samples, inChannels, inSampleRate, sampleFmt, outChannnels, outSampleRate, outSampleFmt);
    env->ReleaseStringUTFChars(outFilepath_, outFilepath);
    return (long)resample;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_audiodecoder_NativeTranscode_nativePushAudioData(JNIEnv *env,
                                                                                jobject instance,
                                                                                jlong obj,
                                                                                jobject buffer,
                                                                                jint bytelen) {

    // TODO
    AudioResample *resample = (AudioResample*)obj;
    uint8_t * inData = (uint8_t *) env->GetDirectBufferAddress(buffer);
    resample->pushAudioData(inData, bytelen);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_audiodecoder_NativeTranscode_endResample__J(JNIEnv *env,
                                                                           jobject instance,
                                                                           jlong obj) {

    // TODO
    AudioResample *resample = (AudioResample*)obj;
    resample->end();
}