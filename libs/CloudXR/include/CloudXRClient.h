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

#ifndef __CLOUDXR_CLIENT_H__
#define __CLOUDXR_CLIENT_H__

#include "CloudXRCommon.h"
#include "CloudXRInputEvents.h"

#ifdef __cplusplus
extern "C"
{
#endif


/*
    A typical main loop might be something like this:
    @code
        set up clientDesc
        set up clientCallbacks
            // >> if connecting asynchronously, implement UpdateClientState
        set up receiverDesc
        @ref cxrCreateReceiver()
        @ref cxrConnect()
        while (!quitting)
            if (state==connected)
                @ref cxrLatchFrame()
                @ref cxrBlitFrame() // Called per eye, Android only.
                @ref cxrReleaseFrame()
            else if (state==connecting)
                // >> render connection progress
            else if (state==failed)
                // >> handle notifying user of connection error
                quitting = true
        @ref cxrDestroyReceiver()
    @endcode
*/

/** @defgroup groupStruct
    @{
 */

/**
	@brief CloudXR SDK client interface

    The *Receiver* is the primary interface handle for a client of the CloudXR SDK to
     connect to a server, describe client device properties, send position and input
     updates, stream live video/audio, and acquire video frames to render out locally.
*/
typedef struct cxrReceiver* cxrReceiverHandle;

/**
    Callback functions for the *Receiver* to interact with client code.

    These functions are used to either request something from the client or to provide
     data or notifications to the client. 
*/
typedef struct _cxrClientCallbacks
{
    /// Return HMD and controller states and poses. Called at a fixed frequency by the Receiver,
    /// as specified by cxrDeviceDesc#posePollFreq.  This callback must be implemented for the
    /// server to render changing position and orientation of client device and controller(s).
    void (*GetTrackingState)(void* context, cxrVRTrackingState* trackingState);

    /// Notify the client it should trigger controller vibration
    void (*TriggerHaptic)(void* context, const cxrHapticFeedback* haptic);

    /// Notify the client to play out the passed audio buffer.
    /// @return cxrTrue if data consumed/played, cxrFalse if cannot play or otherwise discarded
    cxrBool (*RenderAudio)(void* context, const cxrAudioFrame* audioFrame);

    /// ``Reserved for Future Use``
    void (*ReceiveUserData)(void* context, const void* data, uint32_t size);

    /// Notify the client of underlying state changes and why the changed occurred,
    /// for both abnormal and expected cases. This callback must be implemented if you
    /// call cxrConnect() with the `cxrConnectionFlags_ConnectAsync` flag set in order to be notified of
    /// connection state changes in the background thread.
    void (*UpdateClientState)(void* context, cxrClientState state, cxrStateReason reason);
} cxrClientCallbacks;


/**
    This contains details of the client device and overall session characteristics.  It is
     passed to @ref cxrCreateReceiver for construction of the @ref cxrReceiverHandle instance
     and connection to server for streaming.
*/
typedef struct _cxrReceiverDesc
{
    uint32_t requestedVersion;              ///< Must set to CLOUDXR_VERSION_DWORD

    cxrDeviceDesc deviceDesc;               ///< Describes the client device hardware
    cxrClientCallbacks clientCallbacks;     ///< Pointers to client callback functions (NULL for unsupported callbacks)
    void* clientContext;                    ///< App-specific pointer passed back to client in all callback functions.
    const cxrGraphicsContext* shareContext; ///< The API-specific client graphics context to be shared with CloudXR.
                                            ///< @note On certain graphics APIs, such as DirectX, the share context may be NULL.
                                            ///< However, the share context cannot be NULL on OpenGL ES on Android devices.
    
    uint32_t numStreams;                    ///< Number of video streams to open. Ignored for XR which is always 2.
    cxrStreamingMode receiverMode;          ///< Request XR (VR/AR) mode (usually 2 streams), or generic mode (1 or more streams)

    uint32_t debugFlags;                    ///< Features to aid debugging (see @ref cxrDebugFlags)
    int32_t logMaxSizeKB;                   ///< Maximum size for the client log in KB. -1 for default, 0 to disable size limit.
    int32_t logMaxAgeDays;                  ///< Delete log and diagnostic files older than this. -1 for default, 0 to disable.
} cxrReceiverDesc;

/** @} */ // end of groupStruct

/** @defgroup groupFunc
    @{
 */


/**
    @brief Initialize the CloudXR client and create a Receiver handle to be used by all other API calls.

    @param[in] description      A filled-out client receiver descriptor struct (see @ref cxrReceiverDesc).
    @param[out] receiver        A pointer to return back to the client an opaque handle representing the Receiver interface.
    @note						The returned @ref cxrReceiverHandle must be passed as the first parameter to all other CloudXR client API calls.
*/
CLOUDXR_PUBLIC_API cxrError cxrCreateReceiver(const cxrReceiverDesc* description, cxrReceiverHandle* receiver);

/**
    Establish a connection to a CloudXR server.
    @param[in] receiver   cxrReceiverHandle handle that was obtained from cxrCreateReceiver().
    @param[in] serverAddr IP address of the CloudXR server.
    @param[in] connectionFlags Optional flags that control connection behavior (see @ref cxrConnectionFlags).
 */
CLOUDXR_PUBLIC_API cxrError cxrConnect(cxrReceiverHandle receiver, const char* serverAddr, uint64_t connectionFlags);

/**
    @brief Terminate a streaming session and free the associated CloudXR resources.
    @note The Receiver handle is invalid upon return from this call.
    @param[in] receiver   cxrReceiverHandle handle from cxrCreateReceiver()
*/
CLOUDXR_PUBLIC_API void cxrDestroyReceiver(cxrReceiverHandle receiver);



/**
    @brief Acquire the next available decoded video frames from network stream
	This call attempts to acquire the next available decoded video frames for all streams
	in `streamMask`, together in lock-step.  It can be called repeatedly with different
    masks in order to acquire frames without concurrent locking.
    @param[in] receiver   cxrReceiverHandle handle from cxrCreateReceiver()
    @param[in,out] framesLatched  pointer to a cxrFramesLatched struct with a lifetime until cxrReleaseFrame.
    @param[in] frameMask  a bitfield mask set to 1<<[frame number] for generic frames, or use special values like @ref cxrFrameMask_All.
    @param[in] timeoutMs number of milliseconds to wait for frame(s) before returning in error.
    @retval cxrError_Parameter_Invalid      An empty mask was passed in
    @retval cxrError_Frame_Not_Released     A prior call to cxrLatchFrame was not ended with a call to cxrReleaseFrame
    @retval cxrError_Frame_Not_Ready        The timeoutMs elapsed without any frame(s) available
    @retval cxrError_Receiver_Not_Running   The Receiver object is not yet connected and streaming
    @retval cxrError_Decoder_No_Texture     No texture surface is available to return data back to the application
    @retval cxrError_Frame_Not_Latched      A frame was available but a low-level failure occurred during data copy
*/
CLOUDXR_PUBLIC_API cxrError cxrLatchFrame(cxrReceiverHandle receiver, cxrFramesLatched* framesLatched, uint32_t frameMask, uint32_t timeoutMs);

#if defined(ANDROID)
/**
    @brief `ANDROID-ONLY` Render a masked set of the latched video frame(s) to the currently bound target surface.
    @param[in] receiver   cxrReceiverHandle handle from cxrCreateReceiver()
    @param[in,out] framesLatched  pointer to a cxrFramesLatched struct with a lifetime until cxrReleaseFrame.
    @param[in] frameMask  a bitfield mask set to 1<<[frame number] for generic frames, or use cxrFrameMask special values.
*/
CLOUDXR_PUBLIC_API cxrBool cxrBlitFrame(cxrReceiverHandle receiver, cxrFramesLatched* framesLatched, uint32_t frameMask);
#endif

/**
    @brief Release a previously latched set of video frames from cxrLatchFrame.
    @param[in] receiver   cxrReceiverHandle handle from cxrCreateReceiver()
    @param[in,out] framesLatched  pointer to a cxrFramesLatched struct that was filled by cxrLatchFrame.
    @note The cxrFramesLatched data is invalid upon return from this call.
*/
CLOUDXR_PUBLIC_API cxrError cxrReleaseFrame(cxrReceiverHandle receiver, cxrFramesLatched* framesLatched);



/**
    @brief Provide estimated world lighting properties to the server.

    This is used to send ARCore or ARKit lighting estimates to the server, for live integration into
     3D rendered scene lighting.  It supports a primary light source as well as ambient spherical harmonics.
     On the server side, apps can query this data using:

            IVRSystem::GetArrayTrackedDeviceProperty(cxrTrackedDeviceProperty)
*/
CLOUDXR_PUBLIC_API cxrError cxrSendLightProperties(cxrReceiverHandle receiver, const cxrLightProperties* lightProps);

/**
    @brief Send non-VR-controller input events to the server for app-specific handling.
*/
CLOUDXR_PUBLIC_API cxrError cxrSendInputEvent(cxrReceiverHandle receiver, const cxrInputEvent* inputEvent);

/**
    @brief Add an event to the trace timeline.
    @param[in] name   name of the event to add to the timeline
    @param[in] eventId a custom Id value that can be used to identify the event in the timeline
    @param[in] begin true indicates the beginning of an event, false indicates the end of the event.
 
    This function allows for the insertion of custom events into the trace timeline of CloudXR. Note that to log
    an event this function must be called twice.
    
    Example of logging an event:
    void foo()
    {
       cxrTraceEvent("ButtonPressed", 0x23, cxrTrue);
       cxrTraceEvent("ButtonPressed", 0x23, cxrFalse);
    }
*/
CLOUDXR_PUBLIC_API void cxrTraceEvent(char* name, uint32_t eventId, cxrBool begin);

/**
    @brief Send client input audio (i.e. microphone) to the server.

    @note To use this API set the sendAudio boolean (see @ref cxrDeviceDesc).
*/
CLOUDXR_PUBLIC_API cxrError cxrSendAudio(cxrReceiverHandle receiver, const cxrAudioFrame* audioFrame);

/**
    @brief Explicitly push XR tracking state.
*/
CLOUDXR_PUBLIC_API cxrError cxrSendPose(cxrReceiverHandle handle, const cxrVRTrackingState* trackingState);

/** @} */ // end of groupFunc

#ifdef __cplusplus
}
#endif

#endif
