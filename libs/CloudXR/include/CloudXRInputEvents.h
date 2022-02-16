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

#ifndef __CLOUDXR_INPUT_EVENTS_H__
#define __CLOUDXR_INPUT_EVENTS_H__

#include "CloudXRCommon.h"


#ifdef __cplusplus
extern "C"
{
#endif


#pragma pack(push, 4)

typedef enum
{
    cxrKEY_NONE = 0,

    cxrKEY_APOSTROPHE = 0x0027,
    cxrKEY_COMMA = 0x002c,
    cxrKEY_MINUS = 0x002d,
    cxrKEY_PERIOD = 0x002e,
    cxrKEY_SLASH = 0x002f,
    cxrKEY_SEMICOLON = 0x003b,
    cxrKEY_EQUAL = 0x003d,
    cxrKEY_BRACKETLEFT = 0x005b,
    cxrKEY_BACKSLASH = 0x005c,
    cxrKEY_BRACKETRIGHT = 0x005d,
    cxrKEY_NONUS_BACKSLASH = 0x005e,
    cxrKEY_YEN = 0x005f, // This key is present on Japanese Keyboard [Windows ScanCode:7D]
    cxrKEY_HANGUL = 0x0060, // This key is present on Korean Keyboard [Windows ScanCode:72]
    cxrKEY_HANJA = 0x0061, // This key is present on Korean Keyboard [Windows ScanCode:71]

    cxrKEY_SPACE = 0x0020,

    cxrKEY_0 = 0x0030,
    cxrKEY_1 = 0x0031,
    cxrKEY_2 = 0x0032,
    cxrKEY_3 = 0x0033,
    cxrKEY_4 = 0x0034,
    cxrKEY_5 = 0x0035,
    cxrKEY_6 = 0x0036,
    cxrKEY_7 = 0x0037,
    cxrKEY_8 = 0x0038,
    cxrKEY_9 = 0x0039,

    cxrKEY_A = 0x0041,
    cxrKEY_B = 0x0042,
    cxrKEY_C = 0x0043,
    cxrKEY_D = 0x0044,
    cxrKEY_E = 0x0045,
    cxrKEY_F = 0x0046,
    cxrKEY_G = 0x0047,
    cxrKEY_H = 0x0048,
    cxrKEY_I = 0x0049,
    cxrKEY_J = 0x004a,
    cxrKEY_K = 0x004b,
    cxrKEY_L = 0x004c,
    cxrKEY_M = 0x004d,
    cxrKEY_N = 0x004e,
    cxrKEY_O = 0x004f,
    cxrKEY_P = 0x0050,
    cxrKEY_Q = 0x0051,
    cxrKEY_R = 0x0052,
    cxrKEY_S = 0x0053,
    cxrKEY_T = 0x0054,
    cxrKEY_U = 0x0055,
    cxrKEY_V = 0x0056,
    cxrKEY_W = 0x0057,
    cxrKEY_X = 0x0058,
    cxrKEY_Y = 0x0059,
    cxrKEY_Z = 0x005a,

    cxrKEY_AGRAVE = 0x00c0,
    cxrKEY_MULTIPLY = 0x00d7,
    cxrKEY_DIVISION = 0x00f7,

    cxrKEY_ESCAPE = 0x0100,
    cxrKEY_TAB = 0x0101,
    cxrKEY_BACKTAB = 0x0102,
    cxrKEY_BACKSPACE = 0x0103,
    cxrKEY_RETURN = 0x0104,
    cxrKEY_ENTER = 0x0105,
    cxrKEY_INSERT = 0x0106,
    cxrKEY_DELETE = 0x0107,
    cxrKEY_PAUSE = 0x0108,
    cxrKEY_PRINT = 0x0109,
    cxrKEY_CLEAR = 0x010a,

    cxrKEY_HOME = 0x0200,
    cxrKEY_END = 0x0201,
    cxrKEY_LEFT = 0x0202,
    cxrKEY_UP = 0x0203,
    cxrKEY_RIGHT = 0x0204,
    cxrKEY_DOWN = 0x0205,
    cxrKEY_PAGE_UP = 0x0206,
    cxrKEY_PAGE_DOWN = 0x0207,

    // Modifier keys
    cxrKEY_SHIFT = 0x0301,
    cxrKEY_LSHIFT = 0x0302,
    cxrKEY_RSHIFT = 0x0303,

    cxrKEY_CONTROL = 0x0304,
    cxrKEY_LCONTROL = 0x0305,
    cxrKEY_RCONTROL = 0x0306,

    cxrKEY_ALT = 0x0307,
    cxrKEY_LALT = 0x0308,
    cxrKEY_RALT = 0x0309,

    cxrKEY_META = 0x0310,
    cxrKEY_LMETA = 0x0311,
    cxrKEY_RMETA = 0x0312,

    cxrKEY_F1 = 0x0400,
    cxrKEY_F2 = 0x0401,
    cxrKEY_F3 = 0x0402,
    cxrKEY_F4 = 0x0403,
    cxrKEY_F5 = 0x0404,
    cxrKEY_F6 = 0x0405,
    cxrKEY_F7 = 0x0406,
    cxrKEY_F8 = 0x0407,
    cxrKEY_F9 = 0x0408,
    cxrKEY_F10 = 0x0409,
    cxrKEY_F11 = 0x040a,
    cxrKEY_F12 = 0x040b,
    cxrKEY_F13 = 0x040c,
    cxrKEY_F14 = 0x040d,
    cxrKEY_F15 = 0x040e,
    cxrKEY_F16 = 0x040f,
    cxrKEY_F17 = 0x0410,
    cxrKEY_F18 = 0x0411,
    cxrKEY_F19 = 0x0412,
    cxrKEY_F20 = 0x0413,
    cxrKEY_F21 = 0x0414,
    cxrKEY_F22 = 0x0415,
    cxrKEY_F23 = 0x0416,
    cxrKEY_F24 = 0x0417,

    // Lock keys
    cxrKEY_CAPS_LOCK = 0x0501,
    cxrKEY_NUM_LOCK = 0x0502,
    cxrKEY_SCROLL_LOCK = 0x0503,

    // Numpad keys
    cxrKEY_KP_0 = 0x0600,
    cxrKEY_KP_1 = 0x0601,
    cxrKEY_KP_2 = 0x0602,
    cxrKEY_KP_3 = 0x0603,
    cxrKEY_KP_4 = 0x0604,
    cxrKEY_KP_5 = 0x0605,
    cxrKEY_KP_6 = 0x0606,
    cxrKEY_KP_7 = 0x0607,
    cxrKEY_KP_8 = 0x0608,
    cxrKEY_KP_9 = 0x0609,

    cxrKEY_ADD = 0x060a,
    cxrKEY_SUBTRACT = 0x060b,
    cxrKEY_DECIMAL = 0x060c,

    cxrKEY_KP_INSERT = 0x060d,
    cxrKEY_KP_END = 0x060e,
    cxrKEY_KP_DOWN = 0x060f,
    cxrKEY_KP_PAGE_DOWN = 0x0610,
    cxrKEY_KP_LEFT = 0x0611,
    cxrKEY_KP_CLEAR = 0x0612,
    cxrKEY_KP_RIGHT = 0x0613,
    cxrKEY_KP_HOME = 0x0614,
    cxrKEY_KP_UP = 0x0615,
    cxrKEY_KP_PAGE_UP = 0x0616,
    cxrKEY_KP_DELETE = 0x0617,

    cxrKEY_MAX = 0x0618,

    cxrKEY_ALL = 0xffff,
} cxrKeyboardInput;

typedef enum
{
    cxrKeyEventType_NONE = 0,
    cxrKeyEventType_UP,
    cxrKeyEventType_DOWN
} cxrKeyEventType;

typedef enum
{
    cxrMF_NONE = 0x0000,
    cxrMF_SHIFT = 0x0001,
    cxrMF_CONTROL = 0x0002,
    cxrMF_ALT = 0x0004,
    cxrMF_META = 0x0008,
    cxrMF_SHIFTRIGHT = 0x0010,
    cxrMF_CONTROLRIGHT = 0x0020,
    cxrMF_ALTRIGHT = 0x0040,
    cxrMF_METARIGHT = 0x0080,
} cxrKeyboardModifierFlags;

typedef struct cxrKeyboardEvent
{
    cxrKeyEventType type;
    cxrKeyboardInput keyboardCode;
    cxrKeyboardModifierFlags flags;
} cxrKeyboardEvent;

typedef enum
{
    cxrMouseEventType_NONE = 0,
    cxrMouseEventType_MOVE,
    cxrMouseEventType_WHEEL,
    cxrMouseEventType_BUTTONUP,
    cxrMouseEventType_BUTTONDOWN
} cxrMouseEventType;

typedef enum
{
    cxrMouseButton_NONE = 0,
    cxrMouseButton_LEFT,
    cxrMouseButton_MIDDLE,
    cxrMouseButton_RIGHT,
    cxrMouseButton_THUMB01,
    cxrMouseButton_THUMB02
} cxrMouseButton;

typedef struct cxrMouseMotion
{
    int16_t x;
    int16_t y;
} cxrMouseMotion;

typedef struct cxrMouseWheel
{
    int16_t x;
    int16_t y;
} cxrMouseWheel;

typedef struct cxrMouseEvent
{
    cxrMouseEventType type;
    cxrMouseButton button;
    union
    {
        cxrMouseMotion motion;
        cxrMouseWheel wheel;
    };
    cxrKeyboardModifierFlags keyboardModifierFlags;
} cxrMouseEvent;

typedef enum
{
    cxrTouchEventType_NONE = 0,
    cxrTouchEventType_FINGERUP,
    cxrTouchEventType_FINGERDOWN,
    cxrTouchEventType_FINGERMOTION,
} cxrTouchEventType;

typedef struct cxrTouchEvent
{
    cxrTouchEventType type;
    float x;
    float y;
} cxrTouchEvent;


typedef struct cxrGenericUserInputEvent
{
    uint8_t* data;
    uint32_t sizeInBytes;
} cxrGenericUserInputEvent;


typedef enum
{
    cxrInputEventType_Invalid,
    cxrInputEventType_Keyboard,
    cxrInputEventType_Mouse,
    cxrInputEventType_Touch,
    cxrInputEventType_Generic,
} cxrInputEventType;


typedef union cxrInputEventUnion
{
    cxrKeyboardEvent keyboardEvent;
    cxrMouseEvent mouseEvent;
    cxrTouchEvent touchEvent;
    cxrGenericUserInputEvent genericInputEvent;
} cxrInputEventUnion;

/// Struct representing an input event.
typedef struct cxrInputEvent
{
    cxrInputEventType type;
    cxrInputEventUnion event;
} cxrInputEvent;

#pragma pack(pop)

#ifdef __cplusplus
} // extern "C"
#endif

#endif
