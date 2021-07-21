package com.picovr.cloudxrclientdemo.controller;

import android.content.Context;
import android.util.Log;

import com.picovr.cloudxrclientdemo.JniInterface;
import com.picovr.cloudxrclientdemo.util.Constants;
import com.picovr.cloudxrclientdemo.util.MatrixUtil;
import com.picovr.cvclient.ButtonNum;
import com.picovr.cvclient.CVController;
import com.picovr.cvclient.CVControllerListener;
import com.picovr.cvclient.CVControllerManager;

import java.util.HashMap;
import java.util.Map;

public class ControllerManager {

    private static final String TAG = "ControllerManager";
    private final Map<Integer, Boolean> leftButtonClickState = new HashMap<>();
    private final Map<Integer, Boolean> rightButtonClickState = new HashMap<>();
    private final float[] leftPos = new float[7];
    private final float[] rightPos = new float[7];
    int triggerNum = 0;
    private CVController leftController;
    private CVController rightController;
    private CVControllerManager cvManager;
    private final CVControllerListener cvListener = new CVControllerListener() {
        @Override
        public void onBindSuccess() {
            Log.d(TAG, "onBindSuccess: ");
        }

        @Override
        public void onBindFail() {
            Log.d(TAG, "onBindFail: ");
        }

        @Override
        public void onThreadStart() {
            Log.d(TAG, "onThreadStart: ");
            leftController = cvManager.getMainController();
            rightController = cvManager.getSubController();
        }

        @Override
        public void onConnectStateChanged(int i, int i1) {

        }

        @Override
        public void onMainControllerChanged(int i) {

        }

        @Override
        public void onChannelChanged(int i, int i1) {

        }
    };

    public void init(Context context) {
        Log.d(TAG, "init: ");
        cvManager = new CVControllerManager(context);
        cvManager.setListener(cvListener);
        leftController = cvManager.getSubController();
        rightController = cvManager.getMainController();
        bindService();

        leftButtonClickState.put(Constants.BUTTON_ID_HOME, false);
        leftButtonClickState.put(Constants.BUTTON_ID_APP, false);
        leftButtonClickState.put(Constants.BUTTON_ID_CLICK, false);
        leftButtonClickState.put(Constants.BUTTON_ID_VOLUMEUP, false);
        leftButtonClickState.put(Constants.BUTTON_ID_VOLUMEDOWN, false);
        leftButtonClickState.put(Constants.BUTTON_ID_BUTTONA, false);
        leftButtonClickState.put(Constants.BUTTON_ID_BUTTONB, false);
        leftButtonClickState.put(Constants.BUTTON_ID_BUTTONX, false);
        leftButtonClickState.put(Constants.BUTTON_ID_BUTTONY, false);
        leftButtonClickState.put(Constants.BUTTON_ID_GRIP, false);
        leftButtonClickState.put(Constants.BUTTON_ID_TRIGGER, false);

        rightButtonClickState.put(Constants.BUTTON_ID_HOME, false);
        rightButtonClickState.put(Constants.BUTTON_ID_APP, false);
        rightButtonClickState.put(Constants.BUTTON_ID_CLICK, false);
        rightButtonClickState.put(Constants.BUTTON_ID_VOLUMEUP, false);
        rightButtonClickState.put(Constants.BUTTON_ID_VOLUMEDOWN, false);
        rightButtonClickState.put(Constants.BUTTON_ID_BUTTONA, false);
        rightButtonClickState.put(Constants.BUTTON_ID_BUTTONB, false);
        rightButtonClickState.put(Constants.BUTTON_ID_BUTTONX, false);
        rightButtonClickState.put(Constants.BUTTON_ID_BUTTONY, false);
        rightButtonClickState.put(Constants.BUTTON_ID_GRIP, false);
        rightButtonClickState.put(Constants.BUTTON_ID_TRIGGER, false);
    }

    public void bindService() {
        cvManager.bindService();
    }

    public void unbindService() {
        cvManager.unbindService();
    }

    public void getLeftMat4(float[] matOut) {
        float[] orientation = leftController.getOrientation();
        float[] position = leftController.getPosition();
        leftPos[0] = -orientation[0];
        leftPos[1] = -orientation[1];
        leftPos[2] = orientation[2];
        leftPos[3] = orientation[3];
        leftPos[4] = position[0];
        leftPos[5] = position[1];
        leftPos[6] = position[2];

        MatrixUtil.quternion2Matrix(leftPos, matOut);
    }

    public void getRightMat4(float[] matOut) {
        float[] orientation = rightController.getOrientation();
        float[] position = rightController.getPosition();
        rightPos[0] = -orientation[0];
        rightPos[1] = -orientation[1];
        rightPos[2] = orientation[2];
        rightPos[3] = orientation[3];
        rightPos[4] = position[0];
        rightPos[5] = position[1];
        rightPos[6] = position[2];

        MatrixUtil.quternion2Matrix(rightPos, matOut);
    }

    public void processControllerEvent() {
        boolean buttonState = false;
        buttonState = leftController.getButtonState(ButtonNum.app);
        changeButtonState(0, Constants.BUTTON_ID_APP, buttonState);

        buttonState = leftController.getButtonState(ButtonNum.buttonRG);
        changeButtonState(0, Constants.BUTTON_ID_GRIP, buttonState);

        buttonState = leftController.getButtonState(ButtonNum.click);
        changeButtonState(0, Constants.BUTTON_ID_CLICK, buttonState);

        buttonState = leftController.getButtonState(ButtonNum.buttonAX);
        changeButtonState(0, Constants.BUTTON_ID_BUTTONX, buttonState);

        buttonState = leftController.getButtonState(ButtonNum.buttonBY);
        changeButtonState(0, Constants.BUTTON_ID_BUTTONY, buttonState);

        triggerNum = leftController.getTriggerNum();
        buttonState = triggerNum == 255;
        changeButtonState(0, Constants.BUTTON_ID_TRIGGER, buttonState);

        buttonState = rightController.getButtonState(ButtonNum.app);
        changeButtonState(1, Constants.BUTTON_ID_APP, buttonState);

        buttonState = rightController.getButtonState(ButtonNum.buttonLG);
        changeButtonState(1, Constants.BUTTON_ID_GRIP, buttonState);

        buttonState = rightController.getButtonState(ButtonNum.click);
        changeButtonState(1, Constants.BUTTON_ID_CLICK, buttonState);

        buttonState = rightController.getButtonState(ButtonNum.buttonAX);
        changeButtonState(1, Constants.BUTTON_ID_BUTTONA, buttonState);

        buttonState = rightController.getButtonState(ButtonNum.buttonBY);
        changeButtonState(1, Constants.BUTTON_ID_BUTTONB, buttonState);

        triggerNum = rightController.getTriggerNum();
        if (triggerNum == 255) {
            buttonState = true;
        } else {
            buttonState = false;
        }
        changeButtonState(1, Constants.BUTTON_ID_TRIGGER, buttonState);

        int[] touchPadL = leftController.getTouchPad();
        int[] touchPadR = rightController.getTouchPad();

        JniInterface.processJoystick(touchPadL[1] / 128.0f - 1.0f, touchPadL[0] / 128.0f - 1.0f,
                touchPadR[1] / 128.0f - 1.0f, touchPadR[0] / 128.0f - 1.0f);
    }

    private void changeButtonState(int deviceType, int buttonId, boolean buttonState) {
        if (deviceType == 0) {
            if (buttonState != leftButtonClickState.get(buttonId)) {
                JniInterface.processControllerEvent(deviceType, buttonId, buttonState ? 0 : 1);
                leftButtonClickState.put(buttonId, buttonState);
            }
        } else if (deviceType == 1) {
            if (buttonState != rightButtonClickState.get(buttonId)) {
                JniInterface.processControllerEvent(deviceType, buttonId, buttonState ? 0 : 1);
                rightButtonClickState.put(buttonId, buttonState);
            }
        }
    }

    public void updateControllerData(float[] sensorPos) {
        cvManager.updateControllerData(sensorPos);
    }
}
