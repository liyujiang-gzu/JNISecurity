package com.github.gzuliyujiang.jni.demo;

import android.Manifest;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import com.github.gzuliyujiang.jni.JNISecurity;

/**
 * 使用 so 文件存储私密数据，并增加签名防盗机制
 * <p>
 * https://rockycoder.cn/android%20ndk/2018/11/18/Android-NDK-DecryptKey.html
 */
public class MainActivity extends AppCompatActivity {
    private static final String TAG = "liyujiang-java";
    private static final int REQUEST_EXTERNAL_STORAGE = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        checkPermission();
        setContentView(R.layout.activity_main);
        TextView textView = findViewById(R.id.value);
        boolean flag = JNISecurity.verify();
        Log.w(TAG, String.valueOf(flag));
        String key = JNISecurity.getApiKey();
        Log.w(TAG, "API key: " + key);
        textView.setText(String.format("%s\n%s\n\n", flag, key));
        String result = JNISecurity.getDeviceInfo();
        textView.append(result);
        Log.w(TAG, result);
    }

    private void checkPermission() {
        int permission = ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE);
        if (permission != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                this, new String[]{
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE
                }, REQUEST_EXTERNAL_STORAGE
            );
        }
    }

}
