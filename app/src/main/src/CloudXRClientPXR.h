// Copyright (2021-2023) Bytedance Ltd. and/or its affiliates 
#ifndef CLIENT_APP_PXR_MAIN_H
#define CLIENT_APP_PXR_MAIN_H

#include <CloudXRClient.h>
#include <math.h>
#include <stdio.h>
#include <mutex>
#include <oboe/Oboe.h>
#include <unordered_map>
#include <PxrInput.h>
#include "util.h"
#include "PxrTypes.h"
#include "PxrHelper.h"

class CloudXRClientPXR : public oboe::AudioStreamDataCallback {

public:

    CloudXRClientPXR();

    ~CloudXRClientPXR();

    void Initialize();

    void SetPaused(bool pause);

    bool Start();

    bool Stop();

    void HandleStateChanges();

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) override;

    void GetTrackingState(cxrVRTrackingState *trackingState);

    void TriggerHaptic(const cxrHapticFeedback *);

    cxrBool RenderAudio(const cxrAudioFrame *);

    cxrError CreateReceiver();

    cxrError Connect();

    void TeardownReceiver();

    void UpdateClientState();

    bool LatchFrame(cxrFramesLatched *framesLatched);

    void BlitFrame(cxrFramesLatched *framesLatched, bool frameValid, int eye);

    void ReleaseFrame(cxrFramesLatched *framesLatched);

    void ProcessControllers();

    cxrTrackedDevicePose ConvertPose(const PxrSensorState &pose, float rotationX = 0);

    void GetDeviceDesc(cxrDeviceDesc *params) const;

    void DoTracking();

    static cxrError QueryChaperone(cxrDeviceDesc *deviceDesc);

    void FillBackground() const;

    void SetPoseData(pxrPose pose);

    void GetConnectionStats(uint64_t timeMs);

    PxrQuaternionf cxrToQuaternion(const cxrMatrix34 &m);

    PxrVector3f cxrGetTranslation(const cxrMatrix34 &m);

    bool SetupFramebuffer(GLuint colorTexture, uint32_t eye);

protected:
    bool mRefreshChanged = false;
    float_t mTargetDisplayRefresh = 0;

    std::shared_ptr<oboe::AudioStream> recordingStream{};
    std::shared_ptr<oboe::AudioStream> playbackStream{};

    cxrVRTrackingState TrackingState = {};
    cxrReceiverHandle Receiver = nullptr;
    cxrClientState mClientState = cxrClientState_ReadyToConnect;
    cxrDeviceDesc mDeviceDesc = {};
    cxrConnectionDesc mConnectionDesc = {};
    PxrSensorState headPose = {};
    PxrSensorState leftControllerPose = {};
    PxrSensorState rightControllerPose = {};

    bool mIsPaused;
    bool mWasPaused;

    uint32_t mDefaultBGColor = 0xFF000000; // black to start until we set around OnResume.
    uint32_t mBGColor = mDefaultBGColor;

    struct PxrCxrButtonMapping {
        unsigned int pxrId;
        cxrButtonId cxrId;
        char nameStr[32];
    };

    GLuint Framebuffers[2];

    bool setBooleanButton(cxrControllerTrackingState &ctl, const uint64_t &inBitfield, const PxrCxrButtonMapping &mapping);

};

#endif //CLIENT_APP_PXR_MAIN_H