LOCAL_PATH := $(call my-dir)

# Add prebuilt Oboe library
include $(CLEAR_VARS)
LOCAL_MODULE := Oboe
LOCAL_SRC_FILES := $(OBOE_SDK_ROOT)/prefab/modules/oboe/libs/android.$(TARGET_ARCH_ABI)/liboboe.so
include $(PREBUILT_SHARED_LIBRARY)

# Add prebuilt CloudXR client library
include $(CLEAR_VARS)
LOCAL_MODULE := CloudXRClient
LOCAL_SRC_FILES := $(CLOUDXR_SDK_ROOT)/jni/$(TARGET_ARCH_ABI)/libCloudXRClient.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := Grid
LOCAL_SRC_FILES := $(CLOUDXR_SDK_ROOT)/jni/$(TARGET_ARCH_ABI)/libgrid.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := Poco
LOCAL_SRC_FILES := $(CLOUDXR_SDK_ROOT)/jni/$(TARGET_ARCH_ABI)/libPoco.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := GsAudioWebRTC
LOCAL_SRC_FILES := $(CLOUDXR_SDK_ROOT)/jni/$(TARGET_ARCH_ABI)/libGsAudioWebRTC.so
include $(PREBUILT_SHARED_LIBRARY)

# Add prebuilt pxr_api library
include $(CLEAR_VARS)
LOCAL_MODULE := PxrApi
LOCAL_SRC_FILES := $(PXR_SDK_ROOT)/libs/$(TARGET_ARCH_ABI)/libpxr_api.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := CloudXRClientPXR

LOCAL_C_INCLUDES := $(PXR_SDK_ROOT)/include \
                    $(OBOE_SDK_ROOT)/prefab/modules/oboe/include \
                    $(C_SHARED_INCLUDE) \
                    $(CLOUDXR_SDK_ROOT)/include \
                    ../src

LOCAL_SRC_FILES := ../src/main.cpp \
				   ../src/CloudXRClientPXR.cpp \
                   ../src/graphicsplugin_opengles.cpp \
                   ../src/GLUtils.cpp \

LOCAL_LDLIBS := -llog -landroid -lGLESv3 -lEGL
LOCAL_STATIC_LIBRARIES	:= android_native_app_glue
LOCAL_SHARED_LIBRARIES := PxrApi Oboe CloudXRClient Grid Poco GsAudioWebRTC

include $(BUILD_SHARED_LIBRARY)


$(call import-module, android/native_app_glue)
$(call import-add-path, $(PXR_SDK_ROOT))