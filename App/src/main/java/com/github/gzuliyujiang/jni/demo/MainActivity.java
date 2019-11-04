package com.github.gzuliyujiang.jni.demo;

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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        TextView value = findViewById(R.id.value);
        Log.w("MainActivity", JNISecurity.getSign(this));
        boolean flag = JNISecurity.init();
        Log.w("MainActivity", String.valueOf(flag));
        String key = JNISecurity.getKey();
        Log.w("MainActivity", key);
        value.setText(String.format("%s%s", flag, key));
    }

}
