#include <jni.h>
#include <string>
#include "CloudXRClient.h"
#include "pvr_types.h"

CloudXRClient *cloudXrClient = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_init(JNIEnv *env, jclass clazz) {
    cloudXrClient = new CloudXRClient;
    cloudXrClient->Init();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_connect(JNIEnv *env, jclass clazz) {
    if (cloudXrClient != nullptr) {
        cloudXrClient->CreateReceiver();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_latchFrame(JNIEnv *env, jclass clazz) {
    if (cloudXrClient != nullptr) {
        cloudXrClient->LatchFrame();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_releaseFrame(JNIEnv *env, jclass clazz) {
    if (cloudXrClient != nullptr) {
        cloudXrClient->ReleaseFrame();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_teardownReceiver(JNIEnv *env, jclass clazz) {
    if (cloudXrClient != nullptr) {
        cloudXrClient->TeardownReceiver();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_setHmdMatrix(JNIEnv *env, jclass clazz,
                                                            jfloatArray matrix) {
    auto *matrix_ = (jfloat *) env->GetFloatArrayElements(matrix, nullptr);
    if (cloudXrClient != nullptr) {
        cloudXrClient->setHmdMatrix(matrix_);
    }
    free(matrix_);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_getTextureId(JNIEnv *env, jclass clazz,
                                                            jint eye_type) {
    return cloudXrClient->getTextureId(eye_type);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_setLeftControllerMat(JNIEnv *env, jclass clazz,
                                                                    jfloatArray mat4_) {
    jfloat *pVector = env->GetFloatArrayElements(mat4_, nullptr);
    if (cloudXrClient != nullptr) {
        cloudXrClient->setLeftControllerMat4(pVector);
    }
    free(pVector);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_setRightControllerMat(JNIEnv *env, jclass clazz,
                                                                     jfloatArray mat4_) {
    jfloat *pVector = env->GetFloatArrayElements(mat4_, nullptr);
    if (cloudXrClient != nullptr) {
        cloudXrClient->setRightControllerMat4(pVector);
    }
    free(pVector);
}

extern "C"
JNIEXPORT jfloatArray JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_getTextureHmdMatrix(JNIEnv *env, jclass clazz) {
    jfloatArray matrixJni = env->NewFloatArray(12);

    float matrix[12] = {0};
    cloudXrClient->getTextureHmdMatrix34(matrix);
    env->SetFloatArrayRegion(matrixJni, 0, 12, matrix);
    return matrixJni;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_processControllerEvent(JNIEnv *env, jclass clazz,
                                                                      jint device_type,
                                                                      jint input_id,
                                                                      jint event_type) {
    if (cloudXrClient != nullptr) {
        PVR_InputEvent_ inputEvent = {static_cast<PVR_DeviceType>(device_type),
                                      static_cast<PVR_InputId>(input_id),
                                      static_cast<PVR_Button_EventType>(event_type)};
        cloudXrClient->ProcessControllerEvent(inputEvent);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_processJoystick(JNIEnv *env, jclass clazz,
                                                               jfloat left_x,
                                                               jfloat left_y, jfloat right_x,
                                                               jfloat right_y) {
    if (cloudXrClient != nullptr) {
        cloudXrClient->ProcessJoystick(left_x, left_y, right_x, right_y);
    }
}

extern "C"
JNIEXPORT jfloatArray JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_getHapticData(JNIEnv *env, jclass clazz) {
    jfloatArray dataJni = env->NewFloatArray(3);
    float data[3] = {-1};
    cloudXrClient->GetHapticData(data);
    env->SetFloatArrayRegion(dataJni, 0, 3, data);
    return dataJni;
}