package com.picovr.cloudxr;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
//import android.support.v4.app.ActivityCompat;
//import android.support.v4.content.ContextCompat;
import android.util.Log;
import java.util.Arrays;

/**
 * @author Admin
 */
public class MainActivity extends android.app.NativeActivity {
    public static final String TAG = "CloudXR";

    private static String cmdlineOptions = "";
    private static Boolean resumeReady = false;
    private static Boolean permissionDone = false;
    private static Boolean didResume = false;

    static {
        System.loadLibrary("pxr_api");
        System.loadLibrary("CloudXRClientPXR");
    }

    private final int PERMISSION_REQUEST_CODE = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // do super first, as that sets up some native things.
        super.onCreate(savedInstanceState);

        // check for any data passed to our activity that we want to handle
//        cmdlineOptions = getIntent().getStringExtra("args");

        // check for permission for any 'dangerous' class features.
        // Note that INTERNET is normal and pre-granted, and READ_EXTERNAL is implicitly granted when accepting WRITE.
        /*if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED ||
                ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{
                    Manifest.permission.WRITE_EXTERNAL_STORAGE,
                    Manifest.permission.RECORD_AUDIO
            }, PERMISSION_REQUEST_CODE);
            Log.w(TAG, "Waiting for permissions from user...");
        } else {
            permissionDone = true;
        }*/
    }

    @Override
    protected void onResume() {
        Log.d(TAG, this + " onResume()");
        super.onResume();

        /*resumeReady = true;
        if (Boolean.TRUE.equals(permissionDone) && Boolean.TRUE.equals(!didResume)) {
            doResume();
        }*/
    }

    protected void doResume() {
        didResume = true;

        // send down to native any runtime options now that we're past permissions...
//        nativeHandleLaunchOptions(cmdlineOptions);
    }

//    static native void nativeHandleLaunchOptions(String jcmdline);

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        /*if (requestCode == PERMISSION_REQUEST_CODE && grantResults != null && grantResults.length > 0) {
            if (grantResults[0] != PackageManager.PERMISSION_GRANTED) {
                Log.e(TAG, "Error: external storage permission has not been granted.  It is required to read launch options file or write logs.");
            }

            if (grantResults[1] != PackageManager.PERMISSION_GRANTED) {
                Log.e(TAG, "Warning: Record audio permission not granted, cannot use microphone.");
            }
        } else {
            Log.e(TAG, "Bad return for RequestPermissions: [" + requestCode + "] {" + Arrays.toString(permissions) + "} {" + Arrays.toString(grantResults) + "}");
        }

        // we don't currently treat any of these permissions as required/fatal, so continue on...
        permissionDone = true;
        if (Boolean.TRUE.equals(!didResume) && Boolean.TRUE.equals(resumeReady)) {
            doResume();
        }*/
    }
}