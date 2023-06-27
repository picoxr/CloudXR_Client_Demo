package com.picovr.cloudxr;
import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import java.util.ArrayList;
import java.util.List;

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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getPermission(this);
    }

    @Override
    protected void onResume() {
        Log.d(TAG, this + " onResume()");
        super.onResume();

    }

    private List<String> checkPermission(Context context, String[] checkList) {
        List<String> list = new ArrayList<>();
        for (int i = 0; i < checkList.length; i++) {
            if (PackageManager.PERMISSION_GRANTED != ActivityCompat.checkSelfPermission(context, checkList[i])) {
                list.add(checkList[i]);
            }
        }
        return list;
    }

    private void requestPermission(Activity activity, String requestPermissionList[]) {
        ActivityCompat.requestPermissions(activity, requestPermissionList, 100);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode == 100) {
            for (int i = 0; i < permissions.length; i++) {
                if (permissions[i].equals(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                    if (grantResults[i] == PackageManager.PERMISSION_GRANTED) {
                        Log.i("TAG", "Successfully applied for storage permission!");
                    } else {
                        Log.e("TAG", "Failed to apply for storage permission!");
                    }
                }
            }
        }
    }

    private void getPermission(Activity activity) {
        String[] checkList = new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE};
        List<String> needRequestList = checkPermission(activity, checkList);
        if (needRequestList.isEmpty()) {
            Log.i("TAG", "No need to apply for storage permission!");
        } else {
            requestPermission(activity, needRequestList.toArray(new String[needRequestList.size()]));
        }
    }
}