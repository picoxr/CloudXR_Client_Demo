package com.picovr.cloudxrclientdemo;

/**
 * @author Admin
 */
public class JniInterface {

    public static native void init();
    public static native void connect();
    public static native void latchFrame();
    public static native void releaseFrame();
    public static native void teardownReceiver();

    public static native void setHmdMatrix(float[] matrix);

    public static native void setLeftControllerMat(float[] mat4);
    public static native void setRightControllerMat(float[] mat4);

    public static native int getTextureId(int eyeType);

    public static native float[] getTextureHmdMatrix();

    public static native void processControllerEvent(int deviceType, int inputId, int eventType);

    public static native void processJoystick(float leftX, float leftY, float rightX, float rightY);

    public static native float[] getHapticData();
}
