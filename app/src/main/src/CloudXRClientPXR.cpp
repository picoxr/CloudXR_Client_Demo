// Created by Welch on 2021/7/20.

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "CloudXRClientPXR.h"
#include "CloudXRClientOptions.h"
#include "CloudXRCommon.h"
#include <oboe/Oboe.h>
#include <CloudXRMatrixHelpers.h>
#include <PxrApi.h>
#include <PxrInput.h>
#include "PxrHelper.h"

static CloudXR::ClientOptions GOptions;

#define CASE(x) \
case x:     \
return #x

const char *ClientStateEnumToString(cxrClientState state) {
    switch (state) {
        CASE(cxrClientState_ReadyToConnect);
        CASE(cxrClientState_ConnectionAttemptInProgress);
        CASE(cxrClientState_ConnectionAttemptFailed);
        CASE(cxrClientState_StreamingSessionInProgress);
        CASE(cxrClientState_Disconnected);
        CASE(cxrClientState_Exiting);
        default:
            return "";
    }
}

const char *StateReasonEnumToString(cxrStateReason reason) {
    switch (reason) {
        CASE(cxrStateReason_HEVCUnsupported);
        CASE(cxrStateReason_VersionMismatch);
        CASE(cxrStateReason_DisabledFeature);
        CASE(cxrStateReason_RTSPCannotConnect);
        CASE(cxrStateReason_HolePunchFailed);
        CASE(cxrStateReason_NetworkError);
        CASE(cxrStateReason_AuthorizationFailed);
        CASE(cxrStateReason_DisconnectedExpected);
        CASE(cxrStateReason_DisconnectedUnexpected);
        default:
            return "";
    }
}

PxrQuaternionf CloudXRClientPXR::cxrToQuaternion(const cxrMatrix34 &m) {
    PxrQuaternionf q;
    const float trace = m.m[0][0] + m.m[1][1] + m.m[2][2];

    if (trace > 0.f) {
        float s = 0.5f / sqrtf(trace + 1.0f);
        q.w = 0.25f / s;
        q.x = (m.m[2][1] - m.m[1][2]) * s;
        q.y = (m.m[0][2] - m.m[2][0]) * s;
        q.z = (m.m[1][0] - m.m[0][1]) * s;
    } else {
        if (m.m[0][0] > m.m[1][1] && m.m[0][0] > m.m[2][2]) {
            float s = 2.0f * sqrtf(1.0f + m.m[0][0] - m.m[1][1] - m.m[2][2]);
            q.w = (m.m[2][1] - m.m[1][2]) / s;
            q.x = 0.25f * s;
            q.y = (m.m[0][1] + m.m[1][0]) / s;
            q.z = (m.m[0][2] + m.m[2][0]) / s;
        } else if (m.m[1][1] > m.m[2][2]) {
            float s = 2.0f * sqrtf(1.0f + m.m[1][1] - m.m[0][0] - m.m[2][2]);
            q.w = (m.m[0][2] - m.m[2][0]) / s;
            q.x = (m.m[0][1] + m.m[1][0]) / s;
            q.y = 0.25f * s;
            q.z = (m.m[1][2] + m.m[2][1]) / s;
        } else {
            float s = 2.0f * sqrtf(1.0f + m.m[2][2] - m.m[0][0] - m.m[1][1]);
            q.w = (m.m[1][0] - m.m[0][1]) / s;
            q.x = (m.m[0][2] + m.m[2][0]) / s;
            q.y = (m.m[1][2] + m.m[2][1]) / s;
            q.z = 0.25f * s;
        }
    }

    return q;
}

PxrVector3f CloudXRClientPXR::cxrGetTranslation(const cxrMatrix34 &m) {
    return {m.m[0][3], m.m[1][3], m.m[2][3]};
}

cxrMatrix34 cxrConvert(const pxrMatrix4f &m) {
    cxrMatrix34 out{};
    // The matrices are compatible so doing a memcpy() here
    //  noting that we are a [3][4] and ovr uses [4][4]
    memcpy(&out, &m, sizeof(out));
    return out;
}

cxrVector3 cxrConvert(const PxrVector3f &v) {
    return {{v.x / 1000, v.y / 1000, v.z / 1000}};
}

cxrTrackedDevicePose CloudXRClientPXR::ConvertPose(const PxrSensorState &pose, float rotationX) {
    pxrMatrix4f transform = GetTransformFromPose(&pose.pose);

    if (rotationX) {
        const pxrMatrix4f rotation = CreateRotation(rotationX, 0, 0);
        transform = Matrix4f_Multiply(&transform, &rotation);
    }

    cxrTrackedDevicePose TrackedPose{};
    cxrMatrix34 m = cxrConvert(transform);
    cxrMatrixToVecQuat(&m, &TrackedPose.position, &TrackedPose.rotation);
    TrackedPose.velocity = cxrConvert(pose.linearVelocity);
    TrackedPose.angularVelocity = cxrConvert(pose.angularVelocity);

    TrackedPose.poseIsValid = cxrTrue;

    return TrackedPose;
}

void CloudXRClientPXR::Initialize() {
    GOptions.ParseFile("/sdcard/CloudXRLaunchOptions.txt");
}

cxrError CloudXRClientPXR::CreateReceiver() {

    if (Receiver) {
        return cxrError_Success;
    }

//    GOptions.mServerIP = "192.168.1.110";
    if (GOptions.mServerIP.empty()) {
        LOGE("No Server specified.");
        return cxrError_No_Addr;
    }

    GetDeviceDesc(&mDeviceDesc);

    if (mDeviceDesc.receiveAudio) {
        // Initialize audio playback
        oboe::AudioStreamBuilder playbackStreamBuilder;
        playbackStreamBuilder.setDirection(oboe::Direction::Output);
        playbackStreamBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
        playbackStreamBuilder.setSharingMode(oboe::SharingMode::Exclusive);
        playbackStreamBuilder.setFormat(oboe::AudioFormat::I16);
        playbackStreamBuilder.setChannelCount(oboe::ChannelCount::Stereo);
        playbackStreamBuilder.setSampleRate(CXR_AUDIO_SAMPLING_RATE);

        oboe::Result r = playbackStreamBuilder.openStream(playbackStream);
        if (r != oboe::Result::OK) {
            LOGE("Failed to open playback stream. Error: %s", oboe::convertToText(r));
            return cxrError_Failed;
        }

        int bufferSizeFrames = playbackStream->getFramesPerBurst() * 2;
        r = playbackStream->setBufferSizeInFrames(bufferSizeFrames);
        if (r != oboe::Result::OK) {
            LOGE("Failed to set playback stream buffer size to: %d. Error: %s", bufferSizeFrames,
                 oboe::convertToText(r));
            return cxrError_Failed;
        }

        r = playbackStream->start();
        if (r != oboe::Result::OK) {
            LOGE("Failed to start playback stream. Error: %s", oboe::convertToText(r));
            return cxrError_Failed;
        }
    }

    if (mDeviceDesc.sendAudio) {
        // Initialize audio recording
        oboe::AudioStreamBuilder recordingStreamBuilder;
        recordingStreamBuilder.setDirection(oboe::Direction::Input);
        recordingStreamBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
        recordingStreamBuilder.setSharingMode(oboe::SharingMode::Exclusive);
        recordingStreamBuilder.setFormat(oboe::AudioFormat::I16);
        recordingStreamBuilder.setChannelCount(oboe::ChannelCount::Stereo);
        recordingStreamBuilder.setSampleRate(CXR_AUDIO_SAMPLING_RATE);
        recordingStreamBuilder.setInputPreset(oboe::InputPreset::VoiceCommunication);
        recordingStreamBuilder.setDataCallback(this);

        oboe::Result r = recordingStreamBuilder.openStream(recordingStream);
        if (r != oboe::Result::OK) {
            LOGE("Failed to open recording stream. Error: %s", oboe::convertToText(r));
            return cxrError_Failed;
        }

        r = recordingStream->start();
        if (r != oboe::Result::OK) {
            LOGE("Failed to start recording stream. Error: %s", oboe::convertToText(r));
            return cxrError_Failed;
        }
    }

    LOGE("Trying to create Receiver at %s.", GOptions.mServerIP.c_str());
    cxrGraphicsContext context{cxrGraphicsContext_GLES};
    context.egl.display = eglGetCurrentDisplay();
    context.egl.context = eglGetCurrentContext();

    if (context.egl.context == nullptr) {
        LOGE("Error, null context");
    }
    if (context.egl.display == nullptr) {
        LOGE("Error, null display");
    }

    cxrClientCallbacks clientProxy = {nullptr};
    clientProxy.GetTrackingState = [](void *context, cxrVRTrackingState *trackingState) {
        return reinterpret_cast<CloudXRClientPXR *>(context)->GetTrackingState(trackingState);
    };

    clientProxy.TriggerHaptic = [](void *context, const cxrHapticFeedback *haptic) {
        auto &haptic1 = const_cast<cxrHapticFeedback &>(*haptic);
        return reinterpret_cast<CloudXRClientPXR *>(context)->TriggerHaptic(haptic);
    };

    clientProxy.RenderAudio = [](void *context, const cxrAudioFrame *audioFrame) {
        return reinterpret_cast<CloudXRClientPXR *>(context)->RenderAudio(audioFrame);
    };

    //the client_lib calls into here when the async connection status changes
    clientProxy.UpdateClientState = [](void *context, cxrClientState state, cxrStateReason reason) {
        switch (state) {
            case cxrClientState_ConnectionAttemptInProgress:
                LOGE("Connection attempt in progress...");
                break;
            case cxrClientState_StreamingSessionInProgress:
                LOGE("Async connection succeeded.");
                break;
            case cxrClientState_ConnectionAttemptFailed:
                LOGE("Connection attempt failed. [%i]", reason);
                break;
            case cxrClientState_Disconnected:
                LOGE("Server disconnected with reason: [%s]", StateReasonEnumToString(reason));
                break;
            default:
                LOGE("Client state updated: %s, reason: %s", ClientStateEnumToString(state), StateReasonEnumToString(reason));
                break;
        }
        reinterpret_cast<CloudXRClientPXR *>(context)->mClientState = state;
        LOGE("Client state updated: %s, reason: %s", ClientStateEnumToString(state), StateReasonEnumToString(reason));
    };

    cxrReceiverDesc desc = {0};
    desc.requestedVersion = CLOUDXR_VERSION_DWORD;
    desc.deviceDesc = mDeviceDesc;
    desc.clientCallbacks = clientProxy;
    desc.clientContext = this;
    desc.shareContext = &context;
    desc.numStreams = 2;
    desc.receiverMode = cxrStreamingMode_XR;
    desc.debugFlags = GOptions.mDebugFlags;
    desc.debugFlags |= cxrDebugFlags_EnableAImageReaderDecoder;
    desc.logMaxSizeKB = CLOUDXR_LOG_MAX_DEFAULT;
    desc.logMaxAgeDays = CLOUDXR_LOG_MAX_DEFAULT;

    cxrError err = cxrCreateReceiver(&desc, &Receiver);
    if (err != cxrError_Success) {
        LOGE("Failed to create CloudXR receiver. Error %d, %s.", err, cxrErrorString(err));
        return err;
    }

    LOGE("Receiver created!");
    return cxrError_Success;
}

cxrError CloudXRClientPXR::Connect() {
    mConnectionDesc.async = cxrTrue;
    mConnectionDesc.maxVideoBitrateKbps = GOptions.mMaxVideoBitrate;
    mConnectionDesc.clientNetwork = GOptions.mClientNetwork;
    mConnectionDesc.topology = GOptions.mTopology;
    cxrError err = cxrConnect(Receiver, GOptions.mServerIP.c_str(), &mConnectionDesc);
    if (!mConnectionDesc.async) {
        if (err != cxrError_Success) {
            LOGE("Failed to connect to CloudXR server at %s. Error %d, %s.",
                 GOptions.mServerIP.c_str(), (int) err, cxrErrorString(err));
            TeardownReceiver();
            return err;
        } else {
            mClientState = cxrClientState_StreamingSessionInProgress;
            LOGE("Receiver created for server: %s", GOptions.mServerIP.c_str());
        }
    }
    return cxrError_Success;
}

void CloudXRClientPXR::TeardownReceiver() {
    if (playbackStream != nullptr) {
        playbackStream->close();
    }

    if (recordingStream != nullptr) {
        recordingStream->close();
    }

    if (Receiver != nullptr) {
        cxrDestroyReceiver(Receiver);
        Receiver = nullptr;
    }
}

void CloudXRClientPXR::UpdateClientState() {
    if (mClientState == cxrClientState_Exiting) {
        return;
    }

    if (mConnectionDesc.async) {
        switch (mClientState) {
            case cxrClientState_ConnectionAttemptInProgress: {
                static int32_t attemptCount = 0;
                if (++attemptCount % 60 == 0) {
                    LOGE("..... waiting for server connection .....");
                }
                break;
            }
            case cxrClientState_StreamingSessionInProgress:
                break;
            case cxrClientState_ConnectionAttemptFailed:
            case cxrClientState_Disconnected:
                break;
        }
    }
    if (mClientState == cxrClientState_Disconnected ||
        mClientState == cxrClientState_ConnectionAttemptFailed) {
        TeardownReceiver();
    }
}

void CloudXRClientPXR::GetDeviceDesc(cxrDeviceDesc *params) const {

    uint32_t maxW, maxH, recommendW, recommendH;
    Pxr_GetConfigViewsInfos(&maxW, &maxH, &recommendW, &recommendH);

    params->deliveryType = cxrDeliveryType_Stereo_RGB;
    params->width = recommendW;
    params->height = recommendH;
    params->fps = 90;
    params->ipd = Pxr_GetIPD();
    params->predOffset = -0.02f;
    params->receiveAudio = true;
    params->sendAudio = false;
    params->posePollFreq = 0;
    params->ctrlType = cxrControllerType_OculusTouch;
    params->disablePosePrediction = false;
    params->angularVelocityInDeviceSpace = false;
    params->foveatedScaleFactor = (GOptions.mFoveation < 100) ? GOptions.mFoveation : 0;
    params->maxResFactor = 1.0f;

    params->proj[0][0] = -1.25;
    params->proj[0][1] = 1.25;
    params->proj[0][2] = -1.25;
    params->proj[0][3] = 1.25;

    params->proj[1][0] = params->proj[0][0];
    params->proj[1][1] = params->proj[0][1];

    params->proj[1][2] = params->proj[0][2];
    params->proj[1][3] = params->proj[0][3];
    QueryChaperone(params);
}

void CloudXRClientPXR::GetTrackingState(cxrVRTrackingState *trackingState) {
    DoTracking();
    if (trackingState != nullptr) {
        *trackingState = TrackingState;
    }
}

void CloudXRClientPXR::SetPoseData(pxrPose pose) {
    // +1.7 height
    headPose = pose.headPose;
    headPose.pose.position.y += 1.7;
    leftControllerPose = pose.leftControllerPose;
    leftControllerPose.pose.position.y += 1.7;
    rightControllerPose = pose.rightControllerPose;
    rightControllerPose.pose.position.y += 1.7;
}

void CloudXRClientPXR::DoTracking() {
    ProcessControllers();

    TrackingState.hmd.ipd = Pxr_GetIPD();
    // so we truncate the value to 5 decimal places (sub-millimeter precision)
    TrackingState.hmd.ipd = truncf(TrackingState.hmd.ipd * 10000.0f) / 10000.0f;
    TrackingState.hmd.flags = 0; // reset dynamic flags every frame
    TrackingState.hmd.flags |= cxrHmdTrackingFlags_HasIPD;

    if (mRefreshChanged) {
        TrackingState.hmd.displayRefresh = std::fminf(mTargetDisplayRefresh, 90.0f);
        TrackingState.hmd.flags |= cxrHmdTrackingFlags_HasRefresh;
        mRefreshChanged = false;
    }

    TrackingState.hmd.pose = ConvertPose(headPose);
    TrackingState.hmd.pose.poseIsValid = (headPose.status > 0) ? cxrTrue : cxrFalse;
    TrackingState.hmd.pose.deviceIsConnected = (headPose.status > 0) ? cxrTrue : cxrFalse;
    TrackingState.hmd.pose.trackingResult = cxrTrackingResult_Running_OK;
}

void CloudXRClientPXR::ProcessControllers() {
    // these are Neo3-specific button mappings
    const static PxrCxrButtonMapping Neo3LeftRemaps[] =
            {
                    {PXR_BtnX, cxrButton_X, "BtnX"},
                    {PXR_BtnY, cxrButton_Y, "BtnY"},
            };

    const static PxrCxrButtonMapping Neo3RightRemaps[] =
            {
                    {PXR_BtnA, cxrButton_A, "BtnA"},
                    {PXR_BtnB, cxrButton_B, "BtnB"},
            };


    // these are mappings referenced BY INDEX for manual remap logic
    const static PxrCxrButtonMapping extraRemaps[] =
            {
                    {PXR_Home,          cxrButton_System,         "BtnSystem"},
                    {PXR_Touch_Trigger, cxrButton_Trigger_Touch,  "TouchTrig"},
                    {PXR_Trigger,       cxrButton_Trigger_Click,  "BtnTrig"},
                    {PXR_Touchpad,      cxrButton_Touchpad_Click, "TouchTrack"},
                    {PXR_Grip,          cxrButton_Grip_Click,     "BtnGrip"}
            };

    if (Pxr_IsRunning()) {
        PxrControllerCapability cap;
        for (auto hand: {PXR_CONTROLLER_LEFT, PXR_CONTROLLER_RIGHT}) {
            Pxr_GetControllerCapabilities(hand, &cap);
            if (Pxr_GetControllerConnectStatus(hand) == 1) {

                TrackingState.controller[hand].pose = ConvertPose((hand == PXR_CONTROLLER_LEFT) ? leftControllerPose : rightControllerPose, 0.45f);

                TrackingState.controller[hand].pose.deviceIsConnected = cxrTrue;
                TrackingState.controller[hand].pose.trackingResult = cxrTrackingResult_Running_OK;

                // stash current state of booleanComps, to evaluate at end of fn for changes in state this frame.
                const uint64_t priorCompsState = TrackingState.controller[hand].booleanComps;

                // clear changed flags for this pass
                TrackingState.controller[hand].booleanCompsChanged = 0;

                PxrControllerInputState state;
                Pxr_GetControllerInputState(hand, &state);

                PxrInputId btnId = GetInputId(hand, state);

                if (hand == PXR_CONTROLLER_LEFT) {
                    for (auto &mapping: Neo3LeftRemaps) {
                        if (mapping.cxrId == cxrButton_Num) continue;
                        setBooleanButton(TrackingState.controller[hand], btnId, mapping);
                    }
                } else {
                    for (auto &mapping: Neo3RightRemaps) {
                        if (mapping.cxrId == cxrButton_Num) continue;
                        setBooleanButton(TrackingState.controller[hand], btnId, mapping);
                    }
                }

                TrackingState.controller[hand].scalarComps[cxrButton_System] = state.homeValue;
                setBooleanButton(TrackingState.controller[hand], btnId, extraRemaps[0]);

                TrackingState.controller[hand].scalarComps[cxrAnalog_Trigger] = state.triggerValue;
                setBooleanButton(TrackingState.controller[hand], btnId, extraRemaps[2]);

                TrackingState.controller[hand].scalarComps[cxrButton_Trigger_Touch] = state.triggerTouchValue;
                setBooleanButton(TrackingState.controller[hand], btnId, extraRemaps[1]);

                TrackingState.controller[hand].scalarComps[cxrButton_Touchpad_Click] = state.touchpadValue;
                setBooleanButton(TrackingState.controller[hand], btnId, extraRemaps[3]);

                TrackingState.controller[hand].scalarComps[cxrAnalog_Grip] = state.gripValue;
                setBooleanButton(TrackingState.controller[hand], btnId, extraRemaps[4]);
                TrackingState.controller[hand].scalarComps[cxrAnalog_JoystickX] = state.Joystick.x;
                TrackingState.controller[hand].scalarComps[cxrAnalog_JoystickY] = state.Joystick.y;
                TrackingState.controller[hand].booleanCompsChanged = priorCompsState ^ TrackingState.controller[hand].booleanComps;
            }
        }
    }
}

bool CloudXRClientPXR::setBooleanButton(cxrControllerTrackingState &ctl, const uint64_t &inBitfield, const PxrCxrButtonMapping &mapping) {
    const uint64_t prevComps = ctl.booleanComps;
    const uint64_t btnMask = 1ULL << mapping.cxrId;

    if (inBitfield & mapping.pxrId) {
        ctl.booleanComps |= btnMask;
    } else {
        ctl.booleanComps &= ~btnMask;
    }

    if (prevComps != ctl.booleanComps) {
        return true; // we return true when button goes 'down' first frame
    }

    return false;
}

cxrBool CloudXRClientPXR::RenderAudio(const cxrAudioFrame *audioFrame) {
    if (!playbackStream) {
        return cxrFalse;
    }

    const uint32_t timeout = audioFrame->streamSizeBytes / CXR_AUDIO_BYTES_PER_MS;
    const uint32_t numFrames = timeout * CXR_AUDIO_SAMPLING_RATE / 1000;
    playbackStream->write(audioFrame->streamBuffer, numFrames, timeout * oboe::kNanosPerMillisecond);

    return cxrTrue;
}

oboe::DataCallbackResult CloudXRClientPXR::onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    cxrAudioFrame recordedFrame{};
    recordedFrame.streamBuffer = (int16_t *) audioData;
    recordedFrame.streamSizeBytes = numFrames * CXR_AUDIO_CHANNEL_COUNT * CXR_AUDIO_SAMPLE_SIZE;
    cxrSendAudio(Receiver, &recordedFrame);

    return oboe::DataCallbackResult::Continue;
}

bool CloudXRClientPXR::LatchFrame(cxrFramesLatched *framesLatched) {
    // Fetch a CloudXR frame
    const uint32_t timeoutMs = 500;
    bool frameValid = false;

    if (Receiver) {
        if (mClientState == cxrClientState_StreamingSessionInProgress) {
            cxrError frameErr = cxrLatchFrame(Receiver, framesLatched, cxrFrameMask_All, timeoutMs);
            frameValid = (frameErr == cxrError_Success);
            if (!frameValid) {
                if (frameErr == cxrError_Frame_Not_Ready) {
                    LOGE("Error in LatchFrame, frame not ready for %d ms", timeoutMs);
                } else {
                    LOGE("Error in LatchFrame [%0d] = %s", frameErr, cxrErrorString(frameErr));
                }
            }
        }
    } else {
        return cxrError_Failed;
    }

    return frameValid;
}

void CloudXRClientPXR::BlitFrame(cxrFramesLatched *framesLatched, bool frameValid) {
    for (int eye = 0; eye < PXR_EYE_MAX; eye++) {
        if (frameValid) {
            cxrBlitFrame(Receiver, framesLatched, 1 << eye);
        } else {
            FillBackground();
        }
    }
}

void CloudXRClientPXR::ReleaseFrame(cxrFramesLatched *framesLatched) {
    cxrReleaseFrame(Receiver, framesLatched);
}

cxrError CloudXRClientPXR::QueryChaperone(cxrDeviceDesc *deviceDesc) {
    // cxrChaperone chap;
    deviceDesc->chaperone.universe = cxrUniverseOrigin_Standing;
    deviceDesc->chaperone.origin.m[0][0] = deviceDesc->chaperone.origin.m[1][1] = deviceDesc->chaperone.origin.m[2][2] = 1;
    deviceDesc->chaperone.origin.m[0][1] = deviceDesc->chaperone.origin.m[0][2] = deviceDesc->chaperone.origin.m[0][3] = 0;
    deviceDesc->chaperone.origin.m[1][0] = deviceDesc->chaperone.origin.m[1][2] = deviceDesc->chaperone.origin.m[1][3] = 0;
    deviceDesc->chaperone.origin.m[2][0] = deviceDesc->chaperone.origin.m[2][1] = deviceDesc->chaperone.origin.m[2][3] = 0;
    deviceDesc->chaperone.playArea.v[0] = 2.f * 1.5f * 0.5f;
    deviceDesc->chaperone.playArea.v[1] = 2.f * 1.5f * 0.5f;
    LOGE("Setting play area to %0.2f x %0.2f", deviceDesc->chaperone.playArea.v[0], deviceDesc->chaperone.playArea.v[1]);

    return cxrError_Success;
}

void CloudXRClientPXR::FillBackground() const {
    float cr = ((mBGColor & 0x00FF0000) >> 16) / 255.0f;
    float cg = ((mBGColor & 0x0000FF00) >> 8) / 255.0f;
    float cb = ((mBGColor & 0x000000FF)) / 255.0f;
    float ca = ((mBGColor & 0xFF000000) >> 24) / 255.0f;
    glClearColor(cr, cg, cb, ca);
    glClear(GL_COLOR_BUFFER_BIT);
}

void CloudXRClientPXR::TriggerHaptic(const cxrHapticFeedback *hapticFeedback) {
    const cxrHapticFeedback &haptic = *hapticFeedback;
    if (haptic.seconds <= 0) {
        return;
    }

    PxrControllerCapability cap;
    Pxr_GetControllerCapabilities(haptic.controllerIdx, &cap);
    if (Pxr_GetControllerConnectStatus(haptic.controllerIdx) == 1) {
        Pxr_SetControllerVibration(haptic.controllerIdx, haptic.amplitude, haptic.seconds * 1000);
    }
}

CloudXRClientPXR::CloudXRClientPXR() {
    Initialize();
    CreateReceiver();
    Connect();
}