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
    cloudXrClient->CreateReceiver();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_latchFrame(JNIEnv *env, jclass clazz) {
    cloudXrClient->LatchFrame();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_releaseFrame(JNIEnv *env, jclass clazz) {
    cloudXrClient->ReleaseFrame();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_teardownReceiver(JNIEnv *env, jclass clazz) {
    cloudXrClient->TeardownReceiver();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_setHmdMatrix(JNIEnv *env, jclass clazz,
                                                       jfloatArray matrix) {
    if (cloudXrClient != nullptr) {
        jfloat *matrix_ = (jfloat *) env->GetFloatArrayElements(matrix, 0);
        cloudXrClient->setHmdMatrix(matrix_);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_getTextureId(JNIEnv *env, jclass clazz, jint eye_type) {
    return cloudXrClient->getTextureId(eye_type);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_getTextureHmdMatrix34(JNIEnv *env, jclass clazz,
                                                                jfloatArray hmd_matrix34) {
    jfloat *hmd_matrix34_ = (jfloat *) env->GetFloatArrayElements(hmd_matrix34,
                                                                  0);
    cloudXrClient->getTextureHmdMatrix34(hmd_matrix34_);
    LOGI("onFrame Ndk %f %f %f %f %f %f", hmd_matrix34_[0], hmd_matrix34_[1], hmd_matrix34_[2],
         hmd_matrix34_[3], hmd_matrix34_[4], hmd_matrix34_[5], hmd_matrix34_[6]);
//    env->ReleaseFloatArrayElements(hmd_matrix34, hmd_matrix34_,0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_setLeftControllerMat(JNIEnv *env, jclass clazz,
                                                               jfloatArray mat4_) {
    jfloat *pVector = env->GetFloatArrayElements(mat4_, 0);
    if (cloudXrClient != NULL) {
        cloudXrClient->setLeftControllerMat4(pVector);
    }
    env->ReleaseFloatArrayElements(mat4_, pVector, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_setRightControllerMat(JNIEnv *env, jclass clazz,
                                                                jfloatArray mat4_) {
    jfloat *pVector = env->GetFloatArrayElements(mat4_, 0);
    if (cloudXrClient != NULL) {
        cloudXrClient->setRightControllerMat4(pVector);
    }
    env->ReleaseFloatArrayElements(mat4_, pVector, 0);
}

extern "C"
JNIEXPORT jfloatArray JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_getTextureReturnHmdMatrix(JNIEnv *env, jclass clazz) {
    float matrix[12] = {0};
    cloudXrClient->getTextureHmdMatrix34(matrix);
    return reinterpret_cast<jfloatArray>(matrix);
}

jmethodID methodId = NULL;

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_setTextureHmdMatrix(JNIEnv *env, jclass clazz,
                                                              jobject main_activity) {
    jclass clazz_ = env->FindClass("com/picovr/cloudxrclientdemo/MainActivity");
    if (clazz_ == NULL) {
        LOGI("Cannot find MainActivity class");
        return;
    }

    if (methodId == NULL) {
        methodId = env->GetMethodID(clazz_, "setTextureHmdMatrix", "([F)V");
        if (methodId == NULL) {
            LOGI("Cannot find the setTextureHmdMatrix method");
            return;
        }
    }
    jfloatArray matrixJni = env->NewFloatArray(12);

    float matrix[12] = {0};
    cloudXrClient->getTextureHmdMatrix34(matrix);

    env->SetFloatArrayRegion(matrixJni, 0, 12, matrix);
    env->CallVoidMethod(main_activity, methodId, matrixJni);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_picovr_cloudxrclientdemo_JniInterface_processControllerEvent(JNIEnv *env, jclass clazz,
                                                                 jint device_type, jint input_id,
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
Java_com_picovr_cloudxrclientdemo_JniInterface_processJoystick(JNIEnv *env, jclass clazz, jfloat left_x,
                                                          jfloat left_y, jfloat right_x,
                                                          jfloat right_y) {
    if (cloudXrClient != nullptr) {
        cloudXrClient->ProcessJoystick(left_x, left_y, right_x, right_y);
    }
}