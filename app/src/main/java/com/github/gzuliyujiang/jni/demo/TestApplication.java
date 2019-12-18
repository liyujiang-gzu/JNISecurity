package com.github.gzuliyujiang.jni.demo;

import android.app.Application;

import com.github.gzuliyujiang.jni.JNISecurity;

public class TestApplication extends Application {

    @Override
    public void onCreate() {
        super.onCreate();
        JNISecurity.initial(this);
    }

}
