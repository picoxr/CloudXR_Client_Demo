package com.picovr.cloudxrclientdemo;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.opengl.GLES30;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;

import com.picovr.cloudxrclientdemo.math.CXRSensor;
import com.picovr.cloudxrclientdemo.test.TextureRect;
import com.picovr.cloudxrclientdemo.util.MatrixUtil;
import com.picovr.vractivity.Eye;
import com.picovr.vractivity.HmdState;
import com.picovr.vractivity.RenderInterface;
import com.picovr.vractivity.VRActivity;
import com.psmart.vrlib.PicovrSDK;

import java.util.Arrays;

import static com.picovr.cloudxrclientdemo.util.MatrixUtil.quternion2Matrix;

/**
 * @author Welch
 */
public class MainActivity extends VRActivity implements RenderInterface, SensorEventListener {

    private static final String TAG = "MainActivity";
    private static final String TAG_TH = "PVRTreasureHunter";
    private static final String DBG_LC = "LifeCycle :";
    private static final int UER_EVENT = 100;

    static {
        System.loadLibrary("native-lib");
    }

    TextureRect mTextureRect;
    CXRSensor cxrSensor;
    float[] poseData = new float[7];
    long[] poseTime = new long[3];
    float[] hmdMatrix = new float[12];
    float[] mQuaternion = new float[4];
    private float[] headView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        getWindow().setFlags(
                WindowManager.LayoutParams.FLAG_FULLSCREEN
                        | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_FULLSCREEN
                        | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        super.onCreate(savedInstanceState);

        headView = new float[16];

        cxrSensor = new CXRSensor(this);
        cxrSensor.start();

    }

    @Override
    protected void onPause() {
        super.RenderEventPush(UER_EVENT + 1);
        super.onPause();

        com.picovr.cloudxrclientdemo.JniInterface.teardownReceiver();
    }

    @Override
    protected void onResume() {
        super.onResume();
        PicovrSDK.setTrackingOriginType(1);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        com.picovr.cloudxrclientdemo.JniInterface.teardownReceiver();
    }

    @Override
    public void onFrameBegin(HmdState headTransform) {
        float predict = PicovrSDK.getPredictedDisplayTime();
        PicovrSDK.getPredictedHeadPoseState(predict, poseData, poseTime);
        cxrSensor.setSensorPos(poseData);
        quternion2Matrix(poseData, headView);
        com.picovr.cloudxrclientdemo.JniInterface.setHmdMatrix(headView);
        com.picovr.cloudxrclientdemo.JniInterface.latchFrame();
    }

    @Override
    public void onDrawEye(Eye eye) {
        GLES30.glDisable(GLES30.GL_CULL_FACE);
        GLES30.glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT | GLES30.GL_DEPTH_BUFFER_BIT);
        int textureId = com.picovr.cloudxrclientdemo.JniInterface.getTextureId(eye.getType());
        mTextureRect.drawSelf(textureId);
        GLES30.glFinish();
    }

    @Override
    public void onFrameEnd() {
        com.picovr.cloudxrclientdemo.JniInterface.setTextureHmdMatrix(this);
        MatrixUtil.matrix2Quternion(hmdMatrix, mQuaternion);

        com.picovr.cloudxrclientdemo.JniInterface.releaseFrame();
        poseData[0] = -mQuaternion[0];
        poseData[1] = -mQuaternion[1];
        poseData[2] = -mQuaternion[2];
        poseData[3] = -mQuaternion[3];
        poseData[4] = hmdMatrix[3];
        poseData[5] = hmdMatrix[7];
        poseData[6] = hmdMatrix[11];

        poseTime[0] = 0;
        poseTime[1] = 0;
        poseTime[2] = 0;

        nativeSetRenderPose(nativePtr, poseData, poseTime);
    }

    @Override
    public void onTouchEvent() {
    }

    @Override
    public void onRenderPause() {
    }

    @Override
    public void onRenderResume() {
    }

    @Override
    public void onRendererShutdown() {
        Log.d(TAG_TH, DBG_LC + " onRendererShutdown CALLED");
    }

    @Override
    public void initGL(int w, int h) {

        mTextureRect = new TextureRect(this, 1, 1);

        com.picovr.cloudxrclientdemo.JniInterface.init();
        com.picovr.cloudxrclientdemo.JniInterface.connect();
    }

    @Override
    public void renderEventCallBack(int var) {
    }

    @Override
    public void surfaceChangedCallBack(int i, int i1) {
    }


    @Override
    public void onSensorChanged(SensorEvent event) {
        /*if (event.sensor.getType() == TYPE_GYROSCOPE) {

        } else if (event.sensor.getType() == TYPE_POSE_6DOF) {

        }*/
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }

    public void setTextureHmdMatrix(float[] matrix) {
        Log.e(TAG, "setTextureHmdMatrix: " + Arrays.toString(matrix));
        hmdMatrix = matrix;
    }
}