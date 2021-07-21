//
// Created by Welch
//

#ifndef CLOUDXRPHONE_CLOUDXRCLIENT_H
#define CLOUDXRPHONE_CLOUDXRCLIENT_H
#include <CloudXRClient.h>
#include <math.h>
#include <stdio.h>
#include <mutex>
#include <oboe/Oboe.h>
#include "util.h"
#include "pvr_types.h"


static inline cxrVector3 ToAngleAxis(float yaw, float pitch, float roll)
{
        const float cy = cosf(yaw * 0.5f); // z
        const float sy = sinf(yaw * 0.5f);
        const float cp = cosf(pitch * 0.5f);
        const float sp = sinf(pitch * 0.5f); // y
        const float cr = cosf(roll * 0.5f);
        const float sr = sinf(roll * 0.5f); // x

        const float w = cy * cp * cr + sy * sp * sr;
        const float x = cy * cp * sr - sy * sp * cr;
        const float y = sy * cp * sr + cy * sp * cr;
        const float z = sy * cp * cr - cy * sp * sr;

        const float len2 = x*x + y*y + z*z;

        if (len2 < 0.0001f)
        {
                return {0.f, 0.f, 0.f};
        }

        const float len = sqrtf(len2);
        const float angle = 2.f*acosf(w);

        const float norm = angle/len;

        return {x*norm, y*norm, z*norm};
}

class CloudXRClient : public oboe::AudioStreamDataCallback{
public:
    void Init();
    void GetDeviceDesc(cxrDeviceDesc *params);
    cxrError QueryChaperone(cxrDeviceDesc *deviceDesc) const;

    bool LatchFrame();
    void ReleaseFrame();

    void setHmdMatrix(float matrix[16]);

    void setLeftControllerMat4(float mat4[16]);
    void setRightControllerMat4(float mat4[16]);

    void GetTrackingState(cxrVRTrackingState* state);
    void ProcessControllerEvent(const PVR_InputEvent_ & event);
    cxrBool RenderAudio(const cxrAudioFrame *);
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *oboeStream,
                                          void *audioData, int32_t numFrames);

    void ProcessJoystick(float leftX, float leftY, float rightX, float rightY);

    int getTextureId(int eye_type);

    void getTextureHmdMatrix34(float *hmdMatrix);

    cxrError CreateReceiver();

    void TeardownReceiver();

private:
    cxrReceiverHandle mReceiver = nullptr;
    cxrDeviceDesc mDeviceDesc = {};
    void FillBackground() const;
    void UpdateClientState();
    bool mAutoReconnect = true;

    cxrFramesLatched framesLatched = {};
    bool latched_ = false;

    float hmdMatrix44[16];

    float leftControllerMat4[16];
    float rightControllerMat4[16];

    cxrClientState mClientState = cxrClientState_ReadyToConnect;
    cxrStateReason mClientStateReason = cxrStateReason_NoError;
    uint64_t mConnectionFlags = cxrConnectionFlags_ConnectAsync;

    cxrVRTrackingState mTrackingState;
    std::mutex mTrackingMutex;
    uint32_t mDefaultBGColor = 0xFF000000;
    uint32_t mBGColor = mDefaultBGColor;

    struct PvrCxrButtonMapping{
        PVR_InputId pvrId;
        cxrButtonId cxrId;
        char nameStr[32];
    };

    bool HandleButtonRemap(uint32_t idx, cxrControllerTrackingState &ctl, uint64_t &inputMask,
                           PVR_InputId inId, PVR_Button_EventType evType, const PvrCxrButtonMapping mappingSet[], int mapSize,
                           bool left6dof);
};

#endif //CLOUDXRPHONE_CLOUDXRCLIENT_H