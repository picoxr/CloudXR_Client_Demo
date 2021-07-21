//
// Created by Administrator on 2021/3/9.
//

#ifndef CXRNATIVESDK_PVR_TYPES_H
#define CXRNATIVESDK_PVR_TYPES_H

typedef enum {
    PXR_Home = 0,
    PXR_App = 1,
    PXR_Click = 2,
    PXR_VolumeUp = 3,
    PXR_VolumeDown = 4,
    PXR_ButtonA = 5,
    PXR_ButtonB = 6,
    PXR_ButtonX = 7,
    PXR_ButtonY = 8,
    PXR_ButtonGrip = 9,
    PXR_Trigger = 10
}PVR_InputId;

typedef enum {
    PXR_EventType_Button_Down = 0,
    PXR_EventType_Button_Up = 1,
}PVR_Button_EventType;

typedef enum {
    PXR_ControllerLeft = 0,
    PXR_ControllerRight = 1,
}PVR_DeviceType;

typedef struct PVR_InputEvent{
    PVR_DeviceType pvrDeviceType;
    PVR_InputId pvrInputId;
    PVR_Button_EventType eventType;
}PVR_InputEvent_;

#endif //CXRNATIVESDK_PVR_TYPES_H
