#include "graphicsplugin.h"
#include "PxrApi.h"
#include "PxrInput.h"
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include "CloudXRClientPXR.h"
#include "CloudXRClientOptions.h"
#include <oboe/Oboe.h>
#include <CloudXRMatrixHelpers.h>
#include "PxrHelper.h"
#include <unistd.h>

const int MaxEventCount = 20;

struct AndroidAppState {
    bool resumed = false;
    int recommendW{};
    int recommendH{};
    int eyeLayerId = 0;
    uint64_t layerImages[PXR_EYE_MAX][3] = {0};
    PxrEventDataBuffer *eventDataPointer[MaxEventCount]{};
    pxrPose pose{};
    CloudXRClientPXR *cloudxr = nullptr;
};

/**
 * Process the next main command.
 */
void app_handle_cmd(struct android_app *app, int32_t cmd) {
    auto *appState = (AndroidAppState *) app->userData;

    switch (cmd) {
        // There is no APP_CMD_CREATE. The ANativeActivity creates the
        // application thread from onCreate(). The application thread
        // then calls android_main().
        case APP_CMD_START: {
            LOGI("onStart()");
            LOGI("    APP_CMD_START");
            break;
        }
        case APP_CMD_RESUME: {
            LOGI("onResume()");
            LOGI("    APP_CMD_RESUME");
            appState->resumed = true;
            if (appState->cloudxr) {
                appState->cloudxr->SetPaused(false);
            }
            break;
        }
        case APP_CMD_PAUSE: {
            LOGI("onPause()");
            LOGI("    APP_CMD_PAUSE");
            appState->resumed = false;
            if (appState->cloudxr) {
                appState->cloudxr->SetPaused(true);
            }
            break;
        }
        case APP_CMD_STOP: {
            LOGI("onStop()");
            LOGI("    APP_CMD_STOP");
            break;
        }
        case APP_CMD_DESTROY: {
            LOGI("onDestroy()");
            LOGI("    APP_CMD_DESTROY");
            break;
        }
        case APP_CMD_INIT_WINDOW: {
            LOGI("surfaceCreated()");
            LOGI("    APP_CMD_INIT_WINDOW");
            break;
        }
        case APP_CMD_TERM_WINDOW: {
            LOGI("surfaceDestroyed()");
            LOGI("    APP_CMD_TERM_WINDOW");
            break;
            default:
                break;
        }
    }
}

void pxrapi_init_common(struct android_app *app) {
    PxrInitParamData initParamData = {};
    initParamData.activity = app->activity->clazz;
    initParamData.vm = app->activity->vm;
    initParamData.controllerdof = 1;
    initParamData.headdof = 1;
    Pxr_SetInitializeData(&initParamData);
    Pxr_Initialize();
}

void pxrapi_init_layers(struct android_app *app) {
    auto *s = (AndroidAppState *) app->userData;

    uint32_t maxW, maxH, recommendW, recommendH;
    int layerId = s->eyeLayerId;
    Pxr_GetConfigViewsInfos(&maxW, &maxH, &recommendW, &recommendH);

    s->recommendW = recommendW;
    s->recommendH = recommendH;

    PxrLayerParam layerParam = {};
    layerParam.layerId = layerId;
    layerParam.layerShape = PXR_LAYER_PROJECTION;
    layerParam.layerLayout = PXR_LAYER_LAYOUT_STEREO;
    layerParam.width = recommendW;
    layerParam.height = recommendH;
    layerParam.faceCount = 1;
    layerParam.mipmapCount = 1;
    layerParam.sampleCount = 1;
    layerParam.arraySize = 1;
    layerParam.format = GL_RGBA8;
    int ret = Pxr_CreateLayer(&layerParam);
    LOGI("Pxr_CreateLayer ret = %d", ret);

    int eyeCounts = 2;
    for (int i = 0; i < eyeCounts; i++) {
        uint32_t imageCounts = 0;
        Pxr_GetLayerImageCount(layerId, (PxrEyeType) i, &imageCounts);
        for (int j = 0; j < imageCounts; j++) {
            Pxr_GetLayerImage(layerId, (PxrEyeType) i, j, &s->layerImages[i][j]);
        }
    }
}

void pxrapi_init_events(struct android_app *app) {
    auto *s = (AndroidAppState *) app->userData;
    for (auto &i: s->eventDataPointer) {
        i = new PxrEventDataBuffer;
    }
}

void pxrapi_init(struct android_app *app) {
    // init pxr
    pxrapi_init_common(app);
    // create eye layer
    pxrapi_init_layers(app);
    pxrapi_init_events(app);
}

void pxrapi_deinit(struct android_app *app) {
    auto *s = (AndroidAppState *) app->userData;
    //destroy eye layer
    Pxr_DestroyLayer(s->eyeLayerId);
    for (auto &i: s->eventDataPointer) {
        delete i;
    }
    Pxr_Shutdown();
}

void dispatch_events(struct android_app *app) {
    int eventCount = 0;
    auto *s = (AndroidAppState *) app->userData;

    if (Pxr_PollEvent(MaxEventCount, &eventCount, s->eventDataPointer)) {
        for (int i = 0; i < eventCount; i++) {
            if (s->eventDataPointer[i]->type == PXR_TYPE_EVENT_DATA_SESSION_STATE_READY) {
                Pxr_BeginXr();
            } else if (s->eventDataPointer[i]->type == PXR_TYPE_EVENT_DATA_SESSION_STATE_STOPPING) {
                Pxr_EndXr();
            } else if (s->eventDataPointer[i]->type == PXR_TYPE_EVENT_DATA_CONTROLLER) {
                PxrEventDataControllerChanged *data = ((PxrEventDataControllerChanged*)s->eventDataPointer[i]);
                LOGI("EVENT_DATA_CONTROLLER type:%d, eventLevel:%d, eventtype:%d, controller:%d, status:%d", 
                    data->type, data->eventLevel, data->eventtype, data->controller, data->status);
            }
        }
    }
}

std::shared_ptr<IGraphicsPlugin> graphicsPlugin;

void init_scene(struct android_app *app) {
    graphicsPlugin = CreateGraphicsPlugin_OpenGLES();
    graphicsPlugin->InitializeDevice();
}

void render_frame(android_app *app, CloudXRClientPXR *cloudXR) {
    if (!Pxr_IsRunning()) {
        return;
    }

    auto *s = (AndroidAppState *) app->userData;
    int sensorFrameIndex;
    PxrSensorState sensorState = {};
    double predictedDisplayTimeMs = 0.0f;

    Pxr_BeginFrame();

    Pxr_GetPredictedDisplayTime(&predictedDisplayTimeMs);
    Pxr_GetPredictedMainSensorState(predictedDisplayTimeMs, &sensorState, &sensorFrameIndex);
    s->pose.headPose = sensorState;

    for (int i = 0; i < PXR_CONTROLLER_COUNT; i++) {
        if (Pxr_GetControllerConnectStatus(i) == 1) {
            PxrControllerTracking tracking;
            float sensorController[7];
            sensorController[0] = sensorState.pose.orientation.x;
            sensorController[1] = sensorState.pose.orientation.y;
            sensorController[2] = sensorState.pose.orientation.z;
            sensorController[3] = sensorState.pose.orientation.w;
            sensorController[4] = sensorState.pose.position.x;
            sensorController[5] = sensorState.pose.position.y;
            sensorController[6] = sensorState.pose.position.z;
            Pxr_GetControllerTrackingState(i, predictedDisplayTimeMs, sensorController, &tracking);

            if (i == PXR_CONTROLLER_LEFT) {
                s->pose.leftControllerPose = tracking.localControllerPose;
            } else {
                s->pose.rightControllerPose = tracking.localControllerPose;
            }
        }
    }

    cloudXR->SetPoseData(s->pose);
    cloudXR->GetConnectionStats(uint64_t(predictedDisplayTimeMs));

    cxrFramesLatched framesLatched;
    bool frameValid = cloudXR->LatchFrame(&framesLatched);

    int imageIndex = 0;
    Pxr_GetLayerNextImageIndex(0, &imageIndex);

    graphicsPlugin->RenderView(frameValid, s->recommendW, s->recommendH, s->layerImages[PXR_EYE_LEFT][imageIndex], reinterpret_cast<uintptr_t>(framesLatched.frames[PXR_EYE_LEFT].texture));
    graphicsPlugin->RenderView(frameValid, s->recommendW, s->recommendH, s->layerImages[PXR_EYE_RIGHT][imageIndex], reinterpret_cast<uintptr_t>(framesLatched.frames[PXR_EYE_RIGHT].texture));

    PxrLayerProjection layerProjection = {};
    layerProjection.header.layerId = s->eyeLayerId;
    layerProjection.header.layerFlags |= PXR_LAYER_FLAG_USE_EXTERNAL_HEAD_POSE;
    layerProjection.header.colorScale[0] = 1.0f;
    layerProjection.header.colorScale[1] = 1.0f;
    layerProjection.header.colorScale[2] = 1.0f;
    layerProjection.header.colorScale[3] = 1.0f;
    layerProjection.header.sensorFrameIndex = sensorFrameIndex;

    if (frameValid) {
        layerProjection.header.headPose.orientation = cloudXR->cxrToQuaternion(framesLatched.poseMatrix);
        layerProjection.header.headPose.position = cloudXR->cxrGetTranslation(framesLatched.poseMatrix);
        cloudXR->ReleaseFrame(&framesLatched);
    } else {
        layerProjection.header.layerFlags = 0;
    }

    Pxr_SubmitLayer((PxrLayerHeader *) &layerProjection);
    Pxr_EndFrame();
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app *app) {

    JNIEnv *Env;
    AndroidAppState appState = {};
    app->activity->vm->AttachCurrentThread(&Env, nullptr);
    app->userData = &appState;
    app->onAppCmd = app_handle_cmd;

    auto *cloudXR = new CloudXRClientPXR();
    appState.cloudxr = cloudXR;

    init_scene(app);
    pxrapi_init(app);

    while (app->destroyRequested == 0) {
        // Read all pending events.
        for (;;) {
            int events;
            struct android_poll_source *source;
            // If the timeout is zero, returns immediately without blocking.
            // If the timeout is negative, waits indefinitely until an event appears.
            const int timeoutMilliseconds = (!appState.resumed && !Pxr_IsRunning() && app->destroyRequested == 0) ? 100 : 0;
            if (ALooper_pollAll(timeoutMilliseconds, nullptr, &events, (void **) &source) < 0) {
                break;
            }
            // Process this event.
            if (source != nullptr) {
                source->process(app, source);
            }
        }

        dispatch_events(app);
        cloudXR->UpdateClientState();
        cloudXR->HandleStateChanges();
        render_frame(app, cloudXR);
    }
    LOGE("thread exit app->destroyRequested:%d", app->destroyRequested);
    pxrapi_deinit(app);
    sleep(1);
    //exit needed to release so resouces
    exit(0);
}