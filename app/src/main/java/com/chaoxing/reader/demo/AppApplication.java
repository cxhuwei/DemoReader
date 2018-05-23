package com.chaoxing.reader.demo;

import android.app.Application;

/**
 * Created by HUWEI on 2018/5/23.
 */
public class AppApplication extends Application {

    @Override
    public void onCreate() {
        super.onCreate();
        CrashHandler.get().init(this);
    }

}
