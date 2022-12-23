//
// Created by Welch on 22-8-17.
//

#ifndef NATIVEXR_CLOUDXR_CLIENT_DEMO_PXRHELPER_H
#define NATIVEXR_CLOUDXR_CLIENT_DEMO_PXRHELPER_H

// Row-major 4x4 matrix.
typedef struct pxrMatrix4f_ {
    float M[4][4];
} pxrMatrix4f;

typedef struct pxrPose_ {
    PxrSensorState headPose;
    PxrSensorState leftControllerPose;
    PxrSensorState rightControllerPose;
} pxrPose;

typedef enum {
    NONE = -1,
    PXR_Home = 0,
    PXR_BtnX = 1,
    PXR_BtnY = 2,
    PXR_BtnA = 3,
    PXR_BtnB = 4,
    PXR_Trigger = 5,
    PXR_Grip = 6,
    PXR_Joystick = 7,
    PXR_Touch_Trigger = 8,
    PXR_Touchpad = 9,
    PXR_Menu = 10,
} PxrInputId;

static inline PxrInputId GetInputId(PxrControllerHandness hand, PxrControllerInputState state) {
    if (state.homeValue) {
        return PXR_Home;
    }
    if (state.backValue) {

    }
    if (state.touchpadValue) {
        return PXR_Touchpad;
    }
    if (state.triggerValue > 0.65) {
        return PXR_Trigger;
    }
    if (state.AXValue) {
        if (hand == PXR_CONTROLLER_LEFT) {
            return PXR_BtnX;
        } else {
            return PXR_BtnA;
        }
    }
    if (state.BYValue) {
        if (hand == PXR_CONTROLLER_LEFT) {
            return PXR_BtnY;
        } else {
            return PXR_BtnB;
        }
    }
    if (state.gripValue > 0.65) {
        return PXR_Grip;
    }
    if (state.triggerTouchValue) {
        return PXR_Touch_Trigger;
    }
    if (state.Joystick.x != 0 || state.Joystick.y != 0) {
        return PXR_Joystick;
    }
    if (state.backValue) {
        return PXR_Menu;
    }

    return NONE;
}


/// Returns the 4x4 rotation matrix for the given quaternion.
static inline pxrMatrix4f CreateFromQuaternion(const PxrQuaternionf* q) {
    const float ww = q->w * q->w;
    const float xx = q->x * q->x;
    const float yy = q->y * q->y;
    const float zz = q->z * q->z;

    pxrMatrix4f out;
    out.M[0][0] = ww + xx - yy - zz;
    out.M[0][1] = 2 * (q->x * q->y - q->w * q->z);
    out.M[0][2] = 2 * (q->x * q->z + q->w * q->y);
    out.M[0][3] = 0;

    out.M[1][0] = 2 * (q->x * q->y + q->w * q->z);
    out.M[1][1] = ww - xx + yy - zz;
    out.M[1][2] = 2 * (q->y * q->z - q->w * q->x);
    out.M[1][3] = 0;

    out.M[2][0] = 2 * (q->x * q->z - q->w * q->y);
    out.M[2][1] = 2 * (q->y * q->z + q->w * q->x);
    out.M[2][2] = ww - xx - yy + zz;
    out.M[2][3] = 0;

    out.M[3][0] = 0;
    out.M[3][1] = 0;
    out.M[3][2] = 0;
    out.M[3][3] = 1;
    return out;
}

/// Returns a 4x4 homogeneous translation matrix.
static inline pxrMatrix4f CreateTranslation(const float x, const float y, const float z) {
    pxrMatrix4f out;
    out.M[0][0] = 1.0f;
    out.M[0][1] = 0.0f;
    out.M[0][2] = 0.0f;
    out.M[0][3] = x;
    out.M[1][0] = 0.0f;
    out.M[1][1] = 1.0f;
    out.M[1][2] = 0.0f;
    out.M[1][3] = y;
    out.M[2][0] = 0.0f;
    out.M[2][1] = 0.0f;
    out.M[2][2] = 1.0f;
    out.M[2][3] = z;
    out.M[3][0] = 0.0f;
    out.M[3][1] = 0.0f;
    out.M[3][2] = 0.0f;
    out.M[3][3] = 1.0f;
    return out;
}

/// Use left-multiplication to accumulate transformations.
static inline pxrMatrix4f Matrix4f_Multiply(const pxrMatrix4f* a, const pxrMatrix4f* b) {
    pxrMatrix4f out;
    out.M[0][0] = a->M[0][0] * b->M[0][0] + a->M[0][1] * b->M[1][0] + a->M[0][2] * b->M[2][0] +
                  a->M[0][3] * b->M[3][0];
    out.M[1][0] = a->M[1][0] * b->M[0][0] + a->M[1][1] * b->M[1][0] + a->M[1][2] * b->M[2][0] +
                  a->M[1][3] * b->M[3][0];
    out.M[2][0] = a->M[2][0] * b->M[0][0] + a->M[2][1] * b->M[1][0] + a->M[2][2] * b->M[2][0] +
                  a->M[2][3] * b->M[3][0];
    out.M[3][0] = a->M[3][0] * b->M[0][0] + a->M[3][1] * b->M[1][0] + a->M[3][2] * b->M[2][0] +
                  a->M[3][3] * b->M[3][0];

    out.M[0][1] = a->M[0][0] * b->M[0][1] + a->M[0][1] * b->M[1][1] + a->M[0][2] * b->M[2][1] +
                  a->M[0][3] * b->M[3][1];
    out.M[1][1] = a->M[1][0] * b->M[0][1] + a->M[1][1] * b->M[1][1] + a->M[1][2] * b->M[2][1] +
                  a->M[1][3] * b->M[3][1];
    out.M[2][1] = a->M[2][0] * b->M[0][1] + a->M[2][1] * b->M[1][1] + a->M[2][2] * b->M[2][1] +
                  a->M[2][3] * b->M[3][1];
    out.M[3][1] = a->M[3][0] * b->M[0][1] + a->M[3][1] * b->M[1][1] + a->M[3][2] * b->M[2][1] +
                  a->M[3][3] * b->M[3][1];

    out.M[0][2] = a->M[0][0] * b->M[0][2] + a->M[0][1] * b->M[1][2] + a->M[0][2] * b->M[2][2] +
                  a->M[0][3] * b->M[3][2];
    out.M[1][2] = a->M[1][0] * b->M[0][2] + a->M[1][1] * b->M[1][2] + a->M[1][2] * b->M[2][2] +
                  a->M[1][3] * b->M[3][2];
    out.M[2][2] = a->M[2][0] * b->M[0][2] + a->M[2][1] * b->M[1][2] + a->M[2][2] * b->M[2][2] +
                  a->M[2][3] * b->M[3][2];
    out.M[3][2] = a->M[3][0] * b->M[0][2] + a->M[3][1] * b->M[1][2] + a->M[3][2] * b->M[2][2] +
                  a->M[3][3] * b->M[3][2];

    out.M[0][3] = a->M[0][0] * b->M[0][3] + a->M[0][1] * b->M[1][3] + a->M[0][2] * b->M[2][3] +
                  a->M[0][3] * b->M[3][3];
    out.M[1][3] = a->M[1][0] * b->M[0][3] + a->M[1][1] * b->M[1][3] + a->M[1][2] * b->M[2][3] +
                  a->M[1][3] * b->M[3][3];
    out.M[2][3] = a->M[2][0] * b->M[0][3] + a->M[2][1] * b->M[1][3] + a->M[2][2] * b->M[2][3] +
                  a->M[2][3] * b->M[3][3];
    out.M[3][3] = a->M[3][0] * b->M[0][3] + a->M[3][1] * b->M[1][3] + a->M[3][2] * b->M[2][3] +
                  a->M[3][3] * b->M[3][3];
    return out;
}

static inline pxrMatrix4f GetTransformFromPose(const PxrPosef* pose) {
    const pxrMatrix4f rotation = CreateFromQuaternion(&pose->orientation);
    const pxrMatrix4f translation = CreateTranslation(pose->position.x, pose->position.y, pose->position.z);
    return Matrix4f_Multiply(&translation, &rotation);
}

/// Returns a 4x4 homogeneous rotation matrix.
static inline pxrMatrix4f CreateRotation(const float radiansX, const float radiansY, const float radiansZ) {
    const float sinX = sinf(radiansX);
    const float cosX = cosf(radiansX);
    const pxrMatrix4f rotationX = {{{1, 0, 0, 0}, {0, cosX, -sinX, 0}, {0, sinX, cosX, 0}, {0, 0, 0, 1}}};
    const float sinY = sinf(radiansY);
    const float cosY = cosf(radiansY);
    const pxrMatrix4f rotationY = {{{cosY, 0, sinY, 0}, {0, 1, 0, 0}, {-sinY, 0, cosY, 0}, {0, 0, 0, 1}}};
    const float sinZ = sinf(radiansZ);
    const float cosZ = cosf(radiansZ);
    const pxrMatrix4f rotationZ = {{{cosZ, -sinZ, 0, 0}, {sinZ, cosZ, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
    const pxrMatrix4f rotationXY = Matrix4f_Multiply(&rotationY, &rotationX);
    return Matrix4f_Multiply(&rotationZ, &rotationXY);
}

#endif //NATIVEXR_CLOUDXR_CLIENT_DEMO_PXRHELPER_H
