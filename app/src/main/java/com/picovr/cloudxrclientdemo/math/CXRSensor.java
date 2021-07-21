package com.picovr.cloudxrclientdemo.math;

import android.content.Context;
import android.util.Log;

import com.picovr.cloudxrclientdemo.JniInterface;
import com.picovr.cloudxrclientdemo.controller.ControllerManager;

import java.util.Arrays;

public class CXRSensor {

    private static final String TAG = "CXRSensor";
    private final float[] leftControllPos = new float[16];
    private final float[] rightControllPos = new float[16];
    private final float[] sensorPos = new float[7];
    ControllerManager mControllerManager = new ControllerManager();

    public CXRSensor(Context context) {
        mControllerManager.init(context);
    }

    public void setSensorPos(float[] poseData) {
        System.arraycopy(poseData, 0, sensorPos, 0, sensorPos.length);
    }

    public void start() {
        Thread mThread = new Thread(new Runnable() {
            @Override
            public void run() {
                submitSensor();
            }
        });
        mThread.start();
    }


    public void submitSensor() {

        while (true) {
            if (sensorPos != null) {
                mControllerManager.updateControllerData(sensorPos);
                Log.d(TAG, "submitSensor: sensorPos" + Arrays.toString(sensorPos));
            }
            Log.d(TAG, "submitSensor: sensorPos  null");
            mControllerManager.getLeftMat4(leftControllPos);
            mControllerManager.getRightMat4(rightControllPos);
            Log.d(TAG, "submitSensor: " + Arrays.toString(rightControllPos));

            JniInterface.setRightControllerMat(rightControllPos);
            JniInterface.setLeftControllerMat(leftControllPos);

            mControllerManager.processControllerEvent();

            try {
                Thread.sleep(5);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}
