//
// Created by Welch on 2021/7/20.
//

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "CloudXRClient.h"
#include "CloudXRClientOptions.h"
#include <oboe/Oboe.h>

static CloudXR::ClientOptions GOptions;
std::shared_ptr<oboe::AudioStream> recordingStream{};
std::shared_ptr<oboe::AudioStream> playbackStream{};

void CloudXRClient::Init() {
    GOptions.ParseFile("/sdcard/CloudXRLaunchOptions.txt");
}

cxrError CloudXRClient::CreateReceiver() {

    if (mReceiver)
        return cxrError_Success;

    if (GOptions.mServerIP.empty()) {
        LOGE("No Server specified.");
        return cxrError_No_Addr;
    }

    LOGE("CreateReceiver ip %s", GOptions.mServerIP.c_str());

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

    cxrClientCallbacks clientProxy = {nullptr};
    clientProxy.GetTrackingState = [](void *context, cxrVRTrackingState *trackingState) {
        return reinterpret_cast<CloudXRClient *>(context)->GetTrackingState(trackingState);
    };

    clientProxy.TriggerHaptic = [](void *context, const cxrHapticFeedback *haptic) {
        auto &haptic1 = const_cast<cxrHapticFeedback &>(*haptic);
        return reinterpret_cast<CloudXRClient *>(context)->TriggerHaptic(haptic);
    };

    clientProxy.RenderAudio = [](void *context, const cxrAudioFrame *audioFrame) {
        return reinterpret_cast<CloudXRClient *>(context)->RenderAudio(audioFrame);
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
                LOGE("Connection attempt failed. [%u]", cxrClientState(reason));
                break;
            case cxrClientState_Disconnected:
                LOGE("Server disconnected with reason: [%u]", cxrClientState(reason));
                break;
            default:
                LOGE("Client state updated: %u, reason: %u", cxrClientState(state),
                     cxrClientState(reason));
                break;
        }
        reinterpret_cast<CloudXRClient *>(context)->mClientState = state;
        reinterpret_cast<CloudXRClient *>(context)->mClientStateReason = reason;
        LOGE("Client state updated: %u, reason: %u", cxrClientState(state), cxrClientState(reason));
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
    desc.logMaxSizeKB = CLOUDXR_LOG_MAX_DEFAULT;

    desc.logMaxAgeDays = CLOUDXR_LOG_MAX_DEFAULT;

    cxrError err = cxrCreateReceiver(&desc, &mReceiver);
    if (err != cxrError_Success) {
        LOGE("Failed to create CloudXR receiver. Error %d, %s.", err, cxrErrorString(err));
        return err;
    }

    LOGE("Receiver created!");

//    cxrSendLightProperties(mReceiver, )

    err = cxrConnect(mReceiver, GOptions.mServerIP.c_str(), mConnectionFlags);
    if (!(mConnectionFlags & cxrConnectionFlags_ConnectAsync)) {
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

void CloudXRClient::TeardownReceiver() {
    if (playbackStream != nullptr) {
        playbackStream->close();
    }

    if (recordingStream != nullptr) {
        recordingStream->close();
    }

    if (mReceiver != nullptr) {
        cxrDestroyReceiver(mReceiver);
        mReceiver = nullptr;
    }
}

void CloudXRClient::UpdateClientState() {
    if (mClientState == cxrClientState_Exiting) {
        return;
    }

    if (mConnectionFlags & cxrConnectionFlags_ConnectAsync) {
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
                if (!mAutoReconnect || mClientStateReason == cxrStateReason_VersionMismatch ||
                    mClientStateReason == cxrStateReason_HEVCUnsupported ||
                    mClientStateReason == cxrStateReason_DisabledFeature) {

                    mClientState = cxrClientState_Disconnected;
                } else {
                    mClientState = cxrClientState_ConnectionAttemptInProgress;
                    cxrConnect(mReceiver, GOptions.mServerIP.c_str(), mConnectionFlags);
                }
                break;
            case cxrClientState_Disconnected:
                break;
        }
    }

    if (mClientState == cxrClientState_Disconnected) {
        if (!mAutoReconnect) {
            mClientState = cxrClientState_Exiting;
        } else {
            TeardownReceiver();
            mClientState = cxrClientState_ReadyToConnect;
            CreateReceiver();
        }
    }
}

void CloudXRClient::GetDeviceDesc(cxrDeviceDesc *params) const {
    params->deliveryType = cxrDeliveryType_Stereo_RGB;
    params->width = 1832;
    params->height = 1920;
    params->fps = 72;
    params->ipd = 0.064f;
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

void CloudXRClient::GetTrackingState(cxrVRTrackingState *state) {
    std::lock_guard<std::mutex> lock(mTrackingMutex);
    *state = {};

    state->hmd.pose.poseIsValid = cxrTrue;
    state->hmd.pose.deviceIsConnected = true;
    state->hmd.pose.trackingResult = cxrTrackingResult_Running_OK;

    state->hmd.pose.deviceToAbsoluteTracking = cxrMatrix34{hmdMatrix44[0], hmdMatrix44[1],
                                                           hmdMatrix44[2], hmdMatrix44[3],
                                                           hmdMatrix44[4], hmdMatrix44[5],
                                                           hmdMatrix44[6], hmdMatrix44[7],
                                                           hmdMatrix44[8], hmdMatrix44[9],
                                                           hmdMatrix44[10], hmdMatrix44[11]};

    state->controller[0].pose.deviceIsConnected = true;
    state->controller[0].pose.poseIsValid = true;
    state->controller[0].pose.trackingResult = cxrTrackingResult_Running_OK;
    state->controller[0].pose.deviceToAbsoluteTracking = cxrMatrix34{leftControllerMat4[0],
                                                                     leftControllerMat4[1],
                                                                     leftControllerMat4[2],
                                                                     leftControllerMat4[3],
                                                                     leftControllerMat4[4],
                                                                     leftControllerMat4[5],
                                                                     leftControllerMat4[6],
                                                                     leftControllerMat4[7],
                                                                     leftControllerMat4[8],
                                                                     leftControllerMat4[9],
                                                                     leftControllerMat4[10],
                                                                     leftControllerMat4[11]};

    state->controller[1].pose.deviceIsConnected = true;
    state->controller[1].pose.poseIsValid = true;
    state->controller[1].pose.trackingResult = cxrTrackingResult_Running_OK;
    state->controller[1].pose.deviceToAbsoluteTracking = cxrMatrix34{rightControllerMat4[0],
                                                                     rightControllerMat4[1],
                                                                     rightControllerMat4[2],
                                                                     rightControllerMat4[3],
                                                                     rightControllerMat4[4],
                                                                     rightControllerMat4[5],
                                                                     rightControllerMat4[6],
                                                                     rightControllerMat4[7],
                                                                     rightControllerMat4[8],
                                                                     rightControllerMat4[9],
                                                                     rightControllerMat4[10],
                                                                     rightControllerMat4[11]};

    state->controller[0].booleanComps = mTrackingState.controller[0].booleanComps;
    for (int i = 0; i < cxrAnalog_Num; ++i) {
        state->controller[0].scalarComps[i] = mTrackingState.controller[0].scalarComps[i];
    }
    state->controller[0].booleanCompsChanged = mTrackingState.controller[0].booleanCompsChanged;

    state->controller[1].booleanComps = mTrackingState.controller[1].booleanComps;
    for (int i = 0; i < cxrAnalog_Num; ++i) {
        state->controller[1].scalarComps[i] = mTrackingState.controller[1].scalarComps[i];
    }
    state->controller[1].booleanCompsChanged = mTrackingState.controller[1].booleanCompsChanged;
}

cxrBool CloudXRClient::RenderAudio(const cxrAudioFrame *audioFrame) {
    if (!playbackStream) {
        return cxrFalse;
    }

    const uint32_t timeout = audioFrame->streamSizeBytes / CXR_AUDIO_BYTES_PER_MS;
    const uint32_t numFrames = timeout * CXR_AUDIO_SAMPLING_RATE / 1000;
    playbackStream->write(audioFrame->streamBuffer, numFrames,
                          timeout * oboe::kNanosPerMillisecond);

    return cxrTrue;
}

oboe::DataCallbackResult CloudXRClient::onAudioReady(oboe::AudioStream *oboeStream,
                                                     void *audioData, int32_t numFrames) {
    cxrAudioFrame recordedFrame{};
    recordedFrame.streamBuffer = (int16_t *) audioData;
    recordedFrame.streamSizeBytes = numFrames * CXR_AUDIO_CHANNEL_COUNT * CXR_AUDIO_SAMPLE_SIZE;
    cxrSendAudio(mReceiver, &recordedFrame);

    return oboe::DataCallbackResult::Continue;
}

void CloudXRClient::ProcessControllerEvent(const PVR_InputEvent_ &event) {
    const static PvrCxrButtonMapping PCButtonRemaps[] = {
            {PXR_App,        cxrButton_System,         "Menu"},
            {PXR_Trigger,    cxrButton_Trigger_Click,  "Trig"},
            {PXR_ButtonGrip, cxrButton_Grip_Click,     "Grip"},
            {PXR_Click,      cxrButton_Touchpad_Click, "Touch-Click"},
            {PXR_ButtonA,    cxrButton_A,              "Btn_A"},
            {PXR_ButtonB,    cxrButton_B,              "Btn_B"},
            {PXR_ButtonX,    cxrButton_X,              "Btn_X"},
            {PXR_ButtonY,    cxrButton_Y,              "Btn_Y"},
    };

    const size_t totalRemaps = sizeof(PCButtonRemaps) / sizeof(*PCButtonRemaps);
    uint32_t idx = (event.pvrDeviceType == PXR_ControllerLeft) ? 0 : 1;
    auto &controllerState = mTrackingState.controller[idx];

    uint64_t inputMask = 0;
    const uint64_t prevComps = controllerState.booleanComps;
    int setSize = totalRemaps;

    bool handled = HandleButtonRemap(idx, controllerState, inputMask, event.pvrInputId,
                                     event.eventType,
                                     PCButtonRemaps, setSize, idx == 0);

    if (!handled)return;
    if (prevComps != controllerState.booleanComps) {
        controllerState.booleanCompsChanged |= inputMask;
    } else {
        controllerState.booleanCompsChanged &= ~inputMask;
    }
}

bool
CloudXRClient::HandleButtonRemap(uint32_t idx, cxrControllerTrackingState &ctl, uint64_t &inputMask,
                                 PVR_InputId inId, PVR_Button_EventType evType,
                                 const PvrCxrButtonMapping mappingSet[],
                                 int mapSize, bool left6dof) {

    for (int i = 0; i < mapSize; ++i) {
        const PvrCxrButtonMapping &map = mappingSet[i];
        if (inId != map.pvrId) {
            continue;
        }

        inputMask = 1ULL << map.cxrId;

        if (evType == PXR_EventType_Button_Down) {
            if (map.cxrId == cxrButton_Trigger_Click) {
                ctl.scalarComps[cxrAnalog_Trigger] = 1.0f;
                LOGE("Button cxrButton_Trigger_Click down");
            } else if (map.cxrId == cxrButton_Grip_Click) {
                ctl.scalarComps[cxrAnalog_Grip] = 1.0f;
            }
            ctl.booleanComps |= inputMask;
        } else if (evType == PXR_EventType_Button_Up) {
            if (map.cxrId == cxrButton_Trigger_Click) {
                ctl.scalarComps[cxrAnalog_Trigger] = 0.0f;
                LOGE("Button cxrButton_Trigger_Click up");
            } else if (map.cxrId == cxrButton_Grip_Click) {
                ctl.scalarComps[cxrAnalog_Grip] = 0.0f;
            }
            ctl.booleanComps &= ~inputMask;
        }
        return true;
    }
    return false;
}

void CloudXRClient::ProcessJoystick(float leftX, float leftY, float rightX, float rightY) {
    mTrackingState.controller[0].scalarComps[cxrAnalog_JoystickX] = leftX;
    mTrackingState.controller[0].scalarComps[cxrAnalog_JoystickY] = leftY;

    mTrackingState.controller[1].scalarComps[cxrAnalog_JoystickX] = rightX;
    mTrackingState.controller[1].scalarComps[cxrAnalog_JoystickY] = rightY;
}

bool CloudXRClient::LatchFrame() {
    if (latched_) {
        return true;
    }
    // Fetch a CloudXR frame
    const uint32_t timeoutMs = 500;
    bool frameValid = false;

    if (mReceiver) {
        if (mClientState == cxrClientState_StreamingSessionInProgress) {
            cxrError frameErr = cxrLatchFrame(mReceiver, &framesLatched, cxrFrameMask_All,
                                              timeoutMs);
            frameValid = (frameErr == cxrError_Success);
            if (!frameValid) {
                if (frameErr == cxrError_Frame_Not_Ready) {
                    LOGE("Error in LatchFrame, frame not ready for %d ms", timeoutMs);
                } else {
                    LOGE("Error in LatchFrame [%0d] = %s", frameErr, cxrErrorString(frameErr));
                    if (frameErr == cxrError_Receiver_Not_Running) {
                        TeardownReceiver();
                        CreateReceiver();
                    }
                }
            }
        }
    } else {
        return cxrError_Failed;
    }

    for (int eye = 0; eye < 2; eye++) {
        if (frameValid) {
            cxrBlitFrame(mReceiver, &framesLatched, 1 << eye);
        } else {
            FillBackground();
        }
    }

    if (frameValid) {
        cxrReleaseFrame(mReceiver, &framesLatched);
    }

    latched_ = true;

    return cxrError_Success;
}

void CloudXRClient::ReleaseFrame() {
    if (!latched_) {
        return;
    }
    cxrReleaseFrame(mReceiver, &framesLatched);
    latched_ = false;
}

void CloudXRClient::setHmdMatrix(float *matrix) {
    std::lock_guard<std::mutex> lock(mTrackingMutex);
    for (int i = 0; i < 16; ++i) {
        hmdMatrix44[i] = matrix[i];
    }
}

int CloudXRClient::getTextureId(int eye_type) {
    if (latched_) {
        return reinterpret_cast<uintptr_t>(framesLatched.frames[eye_type].texture);
    }
    return -1;
}

void CloudXRClient::getTextureHmdMatrix34(float *hmdMatrix) {
    if (latched_) {
        hmdMatrix[0] = framesLatched.poseMatrix.m[0][0];
        hmdMatrix[1] = framesLatched.poseMatrix.m[0][1];
        hmdMatrix[2] = framesLatched.poseMatrix.m[0][2];
        hmdMatrix[3] = framesLatched.poseMatrix.m[0][3];

        hmdMatrix[4] = framesLatched.poseMatrix.m[1][0];
        hmdMatrix[5] = framesLatched.poseMatrix.m[1][1];
        hmdMatrix[6] = framesLatched.poseMatrix.m[1][2];
        hmdMatrix[7] = framesLatched.poseMatrix.m[1][3];

        hmdMatrix[8] = framesLatched.poseMatrix.m[2][0];
        hmdMatrix[9] = framesLatched.poseMatrix.m[2][1];
        hmdMatrix[10] = framesLatched.poseMatrix.m[2][2];
        hmdMatrix[11] = framesLatched.poseMatrix.m[2][3];

        LOGE("onFrame Ndk cxr %f %f %f %f %f %f", framesLatched.poseMatrix.m[0][0],
             framesLatched.poseMatrix.m[0][1], framesLatched.poseMatrix.m[0][2],
             framesLatched.poseMatrix.m[0][2], framesLatched.poseMatrix.m[0][3],
             framesLatched.poseMatrix.m[1][0],
             framesLatched.poseMatrix.m[1][1]);
    }
}

void CloudXRClient::setLeftControllerMat4(float *mat4) {
    std::lock_guard<std::mutex> lock(mTrackingMutex);
    for (int i = 0; i < 16; ++i) {
        leftControllerMat4[i] = mat4[i];
    }
}

void CloudXRClient::setRightControllerMat4(float *mat4) {
    std::lock_guard<std::mutex> lock(mTrackingMutex);
    for (int i = 0; i < 16; ++i) {
        rightControllerMat4[i] = mat4[i];
    }
}

cxrError CloudXRClient::QueryChaperone(cxrDeviceDesc *deviceDesc) const {
    // cxrChaperone chap;
    deviceDesc->chaperone.universe = cxrUniverseOrigin_Standing;
    deviceDesc->chaperone.origin.m[0][0] = deviceDesc->chaperone.origin.m[1][1] = deviceDesc->chaperone.origin.m[2][2] = 1;
    deviceDesc->chaperone.origin.m[0][1] = deviceDesc->chaperone.origin.m[0][2] = deviceDesc->chaperone.origin.m[0][3] = 0;
    deviceDesc->chaperone.origin.m[1][0] = deviceDesc->chaperone.origin.m[1][2] = deviceDesc->chaperone.origin.m[1][3] = 0;
    deviceDesc->chaperone.origin.m[2][0] = deviceDesc->chaperone.origin.m[2][1] = deviceDesc->chaperone.origin.m[2][3] = 0;
    deviceDesc->chaperone.playArea.v[0] = 2.f * 1.5f * 0.5f;
    deviceDesc->chaperone.playArea.v[1] = 2.f * 1.5f * 0.5f;
    LOGE("Setting play area to %0.2f x %0.2f", deviceDesc->chaperone.playArea.v[0],
         deviceDesc->chaperone.playArea.v[1]);

    return cxrError_Success;
}

void CloudXRClient::FillBackground() const {
    float cr = ((mBGColor & 0x00FF0000) >> 16) / 255.0f;
    float cg = ((mBGColor & 0x0000FF00) >> 8) / 255.0f;
    float cb = ((mBGColor & 0x000000FF)) / 255.0f;
    float ca = ((mBGColor & 0xFF000000) >> 24) / 255.0f;
    glClearColor(cr, cg, cb, ca);
    glClear(GL_COLOR_BUFFER_BIT);
}

void CloudXRClient::TriggerHaptic(const cxrHapticFeedback *hapticFeedback) {
    const cxrHapticFeedback &haptic = *hapticFeedback;

    if (haptic.seconds > 0) {
        hapticData[0] = haptic.amplitude;
        hapticData[1] = haptic.seconds * 10000;
        hapticData[2] = haptic.controllerIdx;
    }
}

void CloudXRClient::GetHapticData(float *data) {
    data[0] = hapticData[0];
    data[1] = hapticData[1];
    data[2] = hapticData[2];
    memset(hapticData, -1, sizeof(hapticData));
}