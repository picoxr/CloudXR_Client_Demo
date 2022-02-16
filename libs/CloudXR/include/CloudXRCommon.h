/*
 * Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef __CLOUDXR_COMMON_H__
#define __CLOUDXR_COMMON_H__

#define _TOSTR(x, y, z) #x "." #y "." #z
#define TOSTR(x, y, z) _TOSTR(x, y, z)

#define CLOUDXR_VERSION_MAJOR 3
#define CLOUDXR_VERSION_MINOR 1
#define CLOUDXR_VERSION_PATCH 1
#define CLOUDXR_VERSION TOSTR(CLOUDXR_VERSION_MAJOR, CLOUDXR_VERSION_MINOR, CLOUDXR_VERSION_PATCH)
#define CLOUDXR_VERSION_DWORD ((uint32_t) ((CLOUDXR_VERSION_MAJOR << 16) | (CLOUDXR_VERSION_MINOR << 8) | (CLOUDXR_VERSION_PATCH)))

#include <stdint.h>

#if !defined(CLOUDXR_PUBLIC_API)
#if defined(_WIN32)

#ifdef VISION_RECEIVER_LIB
#define CLOUDXR_PUBLIC_API extern "C" __declspec(dllexport) 
#else
#define CLOUDXR_PUBLIC_API extern "C" __declspec(dllimport)
#endif

#elif defined(__linux__)

#ifdef VISION_RECEIVER_LIB
#define CLOUDXR_PUBLIC_API extern "C" __attribute__((visibility("default")))
#else
#define CLOUDXR_PUBLIC_API extern "C"
#endif

#elif defined(__APPLE__)
#define CLOUDXR_PUBLIC_API 

#else
// as sanity fallback, define blank
#define CLOUDXR_PUBLIC_API

#endif
#endif //!defined(CLOUDXR_PUBLIC_API)

#ifdef __cplusplus
extern "C"
{
#endif

/** @defgroup groupStruct
    @{
 */

typedef uint8_t cxrBool;
#define cxrFalse 0
#define cxrTrue 1

const uint32_t cxrFrameMask_Left  = 0x01; ///< Maps to index/stream 0 for stereo modes
const uint32_t cxrFrameMask_Right = 0x02; ///< Maps to index/stream 1 for stereo modes
const uint32_t cxrFrameMask_Special = 0x80000000; ///< Special-case masks that aren't direct indices are noted by setting this bit.
const uint32_t cxrFrameMask_All   = 0xFFFFFFFF; ///< Requests the system map to all available frames

/**
    CloudXR uses a right-handed system, where:
        +y is up
        +x is to the right
        -z is forward
    Distance units are in meters.
*/
typedef struct cxrMatrix34
{
    float m[3][4];
} cxrMatrix34;

typedef struct cxrVector2
{
    float v[2];
} cxrVector2;

typedef struct cxrVector3
{
    float v[3];
} cxrVector3;

typedef struct cxrQuaternion
{
    float w, x, y, z;
} cxrQuaternion;

const unsigned int CXR_MAX_AMBIENT_LIGHT_SH = 9;

typedef enum
{
    Prop_CloudXRServerState_Int32 = 10100,
    Prop_ArLightColor_Vector3,
    Prop_ArLightDirection_Vector3,
    Prop_ArAmbientLightSh0_Vector3                      // Followed by the remaining components, for a total of CXR_MAX_AMBIENT_LIGHT_SH
} cxrTrackedDeviceProperty;

typedef struct cxrLightProperties
{
    cxrVector3 primaryLightColor;
    cxrVector3 primaryLightDirection;
    cxrVector3 ambientLightSh[CXR_MAX_AMBIENT_LIGHT_SH];
} cxrLightProperties;

/// Device Active/Idle State
typedef enum
{
    cxrDeviceActivityLevel_Unknown = -1,                ///< Unknown state.
    cxrDeviceActivityLevel_Idle = 0,                    ///< No activity for the last 10 seconds
    cxrDeviceActivityLevel_UserInteraction = 1,         ///< Activity is happening now (movement or prox sensor)
    cxrDeviceActivityLevel_UserInteraction_Timeout = 2, ///< No activity for the last 0.5 seconds
    cxrDeviceActivityLevel_Standby = 3,                 ///< Idle for at least 5 seconds (configurable in Settings : Power Management)
} cxrDeviceActivityLevel;

typedef enum
{
    cxrTrackingResult_Uninitialized          = 1,

    cxrTrackingResult_Calibrating_InProgress = 100,
    cxrTrackingResult_Calibrating_OutOfRange = 101,

    cxrTrackingResult_Running_OK             = 200,
    cxrTrackingResult_Running_OutOfRange     = 201,

    cxrTrackingResult_Fallback_RotationOnly  = 300,
} cxrTrackingResult;

typedef enum
{
    cxrButton_System,
    cxrButton_ApplicationMenu,

    cxrButton_Grip_Touch,
    cxrButton_Grip_Click,

    cxrButton_Trigger_Touch,
    cxrButton_Trigger_Click,

    cxrButton_Touchpad_Touch,
    cxrButton_Touchpad_Click,

    cxrButton_Joystick_Touch,
    cxrButton_Joystick_Click,

    cxrButton_A,
    cxrButton_B,

    cxrButton_X = cxrButton_A,
    cxrButton_Y = cxrButton_B,

    cxrButton_Num
} cxrButtonId;

typedef enum
{
    cxrAnalog_Trigger,

    cxrAnalog_TouchpadX,
    cxrAnalog_TouchpadY,

    cxrAnalog_JoystickX,
    cxrAnalog_JoystickY,

    cxrAnalog_Grip,
    cxrAnalog_Grip_Force,

    cxrAnalog_Num
} cxrAnalogId;


inline uint32_t ButtonMaskFromId(cxrButtonId id) { return 1u << id; }

typedef enum
{
    cxrController_Left  = 0,
    cxrController_Right = 1,
} cxrControllerId;

typedef enum
{
    cxrUniverseOrigin_Seated,
    cxrUniverseOrigin_Standing
} cxrUniverseOrigin;

typedef struct cxrChaperone
{
    cxrUniverseOrigin universe;
    cxrMatrix34 origin;
    cxrVector2 playArea;
} cxrChaperone;

typedef struct cxrTrackedDevicePose
{
    cxrMatrix34 deviceToAbsoluteTracking;
    cxrVector3 velocity;                  // velocity in tracker space in m/s
    cxrVector3 angularVelocity;           // angular velocity in radians/s
    cxrTrackingResult trackingResult;
    cxrBool poseIsValid;
    cxrBool deviceIsConnected;
} cxrTrackedDevicePose;

const int CXR_NUM_CONTROLLERS = 2;

typedef struct cxrHmdTrackingState
{
    cxrTrackedDevicePose pose;
    cxrDeviceActivityLevel activityLevel;
} cxrHmdTrackingState;

typedef struct cxrControllerTrackingState
{
    cxrTrackedDevicePose pose;
    uint32_t booleanComps;
    uint32_t booleanCompsChanged;
    float scalarComps[cxrAnalog_Num];
} cxrControllerTrackingState;

typedef struct cxrVRTrackingState
{
    cxrHmdTrackingState hmd;
    cxrControllerTrackingState controller[CXR_NUM_CONTROLLERS];
    float poseTimeOffset;                 // Offset in seconds that affects the amount of pose extrapoation. Default is 0. 
} cxrVRTrackingState;

typedef enum
{
    cxrControllerType_HtcVive,
    cxrControllerType_ValveIndex,
    cxrControllerType_OculusTouch,
    cxrControllerType_None
} cxrControllerType;

typedef enum
{
    cxrDeliveryType_Mono_RGB,
    cxrDeliveryType_Mono_RGBA,
    cxrDeliveryType_Stereo_RGB
} cxrDeliveryType;

typedef enum
{
    cxrStreamingMode_XR,
    cxrStreamingMode_Generic
} cxrStreamingMode;

typedef enum
{
    cxrFoveation_PiecewiseQuadratic = 0,
} cxrFoveationMode;

typedef struct cxrDeviceDesc
{
    cxrDeliveryType   deliveryType;                 ///< Tells server that client expects mono or stereo frames, and RGB or RGBA format.
    uint32_t          width;                        ///< Requested render width, typically this is display width.
    uint32_t          height;                       ///< Requested render height, typically this is display height.
                                                    ///< @note Width and height should be **even** values, ideally a multiple of 32.
    float             maxResFactor;                 ///< Suggested maximum oversampling size on server.  Server downsamples larger res to this.  Valid range 0.5-2.0, with 1.2 as a common value.
    float             fps;                          ///< Device display refresh rate, client needs server to provide stream at this rate.
    float             ipd;                          ///< Inter-pupillary distance
    float             proj[2][4];                   ///< Per eye raw projection extents: tangents of the half-angles from the center view axis. 0|1 for left|right eye, then 0-3 for left, right, top, bottom edges.
    float             predOffset;                   ///< A time offset in seconds to adjust the server time prediction model to account for device-specific delay rendering stream to screen.
    cxrBool           receiveAudio;                 ///< This device can receive and play audio FROM the server (e.g. game/app sounds)
    cxrBool           sendAudio;                    ///< This device can send captured audio TO the server (e.g. microphone)
    cxrBool           embedInfoInVideo;             ///< Request the server embed debug information in each frame prior to encoding.
    cxrBool           disablePosePrediction;        ///< Use the raw client-provided pose without any pose extrapolation
    cxrBool           angularVelocityInDeviceSpace; ///< True if angular velocity in device space, false if in world space
    cxrBool           disableVVSync;                ///< Disables VVSync and instead runs server at fixed frame rate based on client fps
    uint32_t          foveatedScaleFactor;          ///< Request server foveate rendered frames at % of display res.  Values [25-100], zero to disable.  50 is reasonable for most uses.
    uint32_t          posePollFreq;                 ///< Frequency to poll pose updates.  Set 0 for default, or rate per second to invoke polling.  Default is 250, maximum is 1000.  Recommend at least 2x display refresh.
    cxrControllerType ctrlType;                     ///< Which pre-defined controller type should be emulated for this device
    uint32_t          foveationModeCaps;            ///< A bitmask of supported foveation modes (1 bit per cxrFoveationMode enum value)
    cxrChaperone      chaperone;                    ///< SteamVR-equivalent chaperone or 'play space'
    float             maxClientQueueSize;           ///< Maximum size of the decoded frame FIFO queue on the client(unit is frames)
} cxrDeviceDesc;

typedef struct cxrHapticFeedback
{
    int   controllerIdx;
    float amplitude;
    float seconds;
    float frequency;
} cxrHapticFeedback;

typedef void* cxrTextureHandle;

typedef enum
{
    cxrGraphicsContext_D3D11,
    cxrGraphicsContext_D3D12,
    cxrGraphicsContext_GL,
    cxrGraphicsContext_GLES,
    cxrGraphicsContext_Vulkan,
    cxrGraphicsContext_Metal,
    cxrGraphicsContext_Cuda,
    cxrGraphicsContext_NvMedia,
    // force to a qword
    cxr_GraphicsContext_Max = ~0,
} cxrGraphicsContextType;
 
// A low-level graphics context information.
typedef struct cxrGraphicsContext
{
    cxrGraphicsContextType type;

    union
    {
#if defined(__linux__)
        struct
        {
            void* display;
            void* context;
        } egl;
#endif

#if defined(_WIN32)
        void* wglContext;

        struct ID3D11DeviceContext* d3d11Context;

        struct
        {
            struct ID3D12Device* device;
            struct ID3D12CommandQueue* queue;
        } d3d12;

#endif

        struct
        {
            int device;
            void* ctx;
        } cuda;

#if defined(WITH_VULKAN)
        struct
        {
            void* instance;
            void* physicalDevice;
            void* device;
            void* queue;
        } vk;
#endif

#if defined(__APPLE__) && defined(WITH_METAL)
        struct
        {
            id<MTLDevice> device;
            id<MTLCommandQueue> queue;
        } metal;
#endif

        uint8_t _[32];
    };
} cxrGraphicsContext;

/// max number of video streams supported for Receiver_Generic mode
#define CXR_MAX_NUM_VIDEO_STREAMS 6

/// In Receiver_XR mode number of streams is always 2
#define CXR_NUM_VIDEO_STREAMS_XR 2

typedef struct cxrVideoFrame
{
    cxrTextureHandle texture;   ///< This frame's texture handle -- can be cast to a platform texture when needed.

    uint32_t width;             ///< Width of frame in texels
    uint32_t height;            ///< Height of frame in texels
    uint32_t pitch;             ///< Width of frame in bytes

    uint32_t widthFinal;        ///< Final width of frame *after* internal transforms, such as de-foveation.
    uint32_t heightFinal;       ///< Final height of frame *after* internal transforms, such as de-foveation.

    uint32_t streamIdx;         ///< Index of video stream this frame comes from.
    uint64_t timeStamp;         ///< Timestamp when the frame was captured on the server.
} cxrVideoFrame;

/**
    This structure is passed into cxrLatchFrame(), and if the latch succeeds it will contain
    data for the latched frames.  The structure's data must be freed by passing to cxrReleaseFrame()
    when done using it.
 */
typedef struct cxrFramesLatched
{
    uint32_t count;                                     ///< The number of frames latched/acquired
    cxrVideoFrame frames[CXR_MAX_NUM_VIDEO_STREAMS];    ///< The video frame data for each frame latched
    cxrMatrix34 poseMatrix;                             ///< The device position at that moment in time
} cxrFramesLatched;

/**
    These bitfield masks denote various debugging flags handled by either the Receiver
     or by the Server.  Note that many of these may have some significant performance impact
     either constantly or at some interval.
 */
typedef enum
{
    cxrDebugFlags_LogVerbose                = 0x00000001, ///< record very verbose output
    cxrDebugFlags_LogQuiet                  = 0x00000002, ///< do not open normal log files at all
    cxrDebugFlags_LogPrivacyDisabled        = 0x00000010, ///< disable any privacy filtering in output
    cxrDebugFlags_TraceLocalEvents          = 0x00000100, ///< record trace of local event timing
    cxrDebugFlags_TraceStreamEvents         = 0x00000200, ///< record trace of c/s stream event timing
    cxrDebugFlags_TraceQosStats             = 0x00000400, ///< record QoS statistics
    cxrDebugFlags_DumpImages                = 0x00001000, ///< Dump images from stream
    cxrDebugFlags_DumpAudio                 = 0x00002000, ///< Dump audio from stream
    cxrDebugFlags_EmbedServerInfo           = 0x00004000, ///< server embeds stats into frame buffers for streaming
    cxrDebugFlags_EmbedClientInfo           = 0x00008000, ///< client embeds stats into frame buffers or overlays
    cxrDebugFlags_CaptureServerBitstream    = 0x00010000, ///< record the server-sent video bitstream
    cxrDebugFlags_CaptureClientBitstream    = 0x00020000, ///< record the client-received video bitstream
    cxrDebugFlags_FallbackDecoder           = 0x00100000, ///< try to use a fallback decoder for platform
    cxrDebugFlags_EnableSXRDecoder          = 0x00200000, ///< `[ANDROID only]` enable the experimental SXR decoder
    cxrDebugFlags_EnableImageReaderDecoder  = 0x00400000, ///< `[ANDROID only]` enable the experimental ImageReader decoder
} cxrDebugFlags;


const uint32_t CXR_AUDIO_CHANNEL_COUNT = 2;             ///< Audio is currently always stereo
const uint32_t CXR_AUDIO_SAMPLE_SIZE = sizeof(int16_t); ///< Audio is currently singed 16-bit samples (little-endian)
const uint32_t CXR_AUDIO_SAMPLING_RATE = 48000;         ///< Audio is currently always 48khz
const uint32_t CXR_AUDIO_FRAME_LENGTH_MS = 5;           ///< Sent audio has a 5 ms default frame length.  Received audio has 5 or 10 ms frame length, depending on the configuration.
const uint32_t CXR_AUDIO_BYTES_PER_MS = CXR_AUDIO_CHANNEL_COUNT * CXR_AUDIO_SAMPLE_SIZE * CXR_AUDIO_SAMPLING_RATE / 1000; ///< Total bytes of audio per ms


typedef struct cxrAudioFrame
{
    int16_t* streamBuffer;
    uint32_t streamSizeBytes;
} cxrAudioFrame;


typedef enum
{
    cxrServerState_NotRunning = 0,      ///< Initial state, not yet running
    cxrServerState_Connecting = 1,      ///< Server is connecting to client, finished the RTSP handshake
    cxrServerState_Running = 2,         ///< Server is connected and ready to accept video and audio input for streaming     
    cxrServerState_HMD_Active = 3,      ///< `[VR only]` HMD is active
    cxrServerState_HMD_Idle = 4,        ///< `[VR only]` HMD is idle
    cxrServerState_Disconnected = 5,    ///< Server is disconnected from the client
    cxrServerState_Error = 6            ///< Server in an error state
} cxrServerState;

typedef enum
{
    cxrClientState_ReadyToConnect = 0,                  ///< Initial state, no connection attempt pending
    cxrClientState_ConnectionAttemptInProgress = 1,     ///< Attempting to connect to server
    cxrClientState_ConnectionAttemptFailed = 2,         ///< Error occurred during connection attempt
    cxrClientState_StreamingSessionInProgress = 3,      ///< Connected, streaming session in progress
    cxrClientState_Disconnected = 4,                    ///< Streaming session has ended
    cxrClientState_Exiting = 5                          ///< Client is shutting down.
} cxrClientState;

typedef enum
{
    cxrStateReason_NoError = 0,
    
    // cxrClientState_ConnectionAttemptFailed
    cxrStateReason_HEVCUnsupported = 1,
    cxrStateReason_VersionMismatch = 2,
    cxrStateReason_DisabledFeature = 3,
    cxrStateReason_RTSPCannotConnect = 4,
    cxrStateReason_HolePunchFailed = 5,
    cxrStateReason_NetworkError = 6,
    cxrStateReason_AuthorizationFailed = 7,

    // cxrClientState_Disconnected
    cxrStateReason_DisconnectedExpected = 8,        ///< The server signalled it was disconnecting to the client properly before closing the connection.
    cxrStateReason_DisconnectedUnexpected = 9       ///< The connection to the server abruptly terminated without any notice.
} cxrStateReason;

typedef enum
{
    cxrConnectionFlags_Default      = 0x00000000, ///< @ref cxrConnect will run synchronously and block calling thread until completion.
    cxrConnectionFlags_ConnectAsync = 0x00000001, ///< Calls to @ref cxrConnect will spawn a background thread to attempt the connection, and return control to calling thread immediately.
} cxrConnectionFlags;

typedef enum
{
    cxrError_Success = 0,
    cxrError_Failed = 1,
    cxrError_No_Addr = 2,
    cxrError_No_Share_Context = 3,
    cxrError_Incompatible_Share_Context = 4,
    cxrError_Frame_Not_Released = 5,
    cxrError_Frame_Not_Latched = 6,
    cxrError_Receiver_Not_Running = 7,
    cxrError_Decoder_No_Texture = 8,
    cxrError_Decoder_Frame_Not_Ready = 9,
    cxrError_Receiver_Invalid = 10,
    cxrError_Timeout = 11,
    cxrError_Frame_Invalid = 12,
    cxrError_Unsupported_Version = 13,
    cxrError_Chaperone_Invalid = 14,
    cxrError_Not_Implemented = 15,
    cxrError_Parameter_Invalid = 16,
    cxrError_Frame_Not_Ready = 17,
    cxrError_No_Decoder = 18,
    cxrError_Could_Not_Connect_To_Server = 19,
    cxrError_Universe_ID_Invalid = 20,
    cxrError_Streamer_Not_Ready = 21,
    cxrError_Server_Has_No_Client = 22,
    cxrError_Device_Description_Not_Received = 23,
    cxrError_Invalid_Stream_Count = 24,
    cxrError_Module_Load_Failed = 25,
    cxrError_ConnectionAlreadyInProgress = 26,
    cxrError_ClientRequestedExit = 27,
    cxrError_PoseNotInPushMode = 28,
    cxrError_AuthorizationHeader_Invalid = 29,
    cxrError_No_Receiver_Desc = 30,
    cxrError_Audio_Frame_Unsupported_Size = 31,
    cxrError_Max = ~0u
} cxrError;

CLOUDXR_PUBLIC_API const char* cxrErrorString(cxrError E);

/// Constant for logging to use a default setting.
#define CLOUDXR_LOG_MAX_DEFAULT -1

const char* const cxrUserDataFileName = "CloudXR User Data";
const char* const cxrUserDataMutexName = "CloudXR User Data Mutex";
const uint32_t cxrUserDataMaxSize = 8 * 1024;

/** @} */ // end of groupStruct

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __CLOUDXR_COMMON_H__
