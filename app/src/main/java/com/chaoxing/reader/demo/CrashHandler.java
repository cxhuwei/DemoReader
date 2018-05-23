package com.chaoxing.reader.demo;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Environment;
import android.os.Looper;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.Writer;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Created by HUWEI on 2018/5/23.
 */
public class CrashHandler implements Thread.UncaughtExceptionHandler {

    private static final CrashHandler sCrashHandler = new CrashHandler();

    private Context mContext;
    private Thread.UncaughtExceptionHandler mDefaultHandler;

    private CrashHandler() {
    }

    public static CrashHandler get() {
        return sCrashHandler;
    }

    public void init(Context context) {
        mContext = context.getApplicationContext();
        mDefaultHandler = Thread.getDefaultUncaughtExceptionHandler();
        Thread.setDefaultUncaughtExceptionHandler(this);
    }

    @Override
    public void uncaughtException(Thread t, Throwable e) {
        if (e == null) {
            if (mDefaultHandler != null) {
                mDefaultHandler.uncaughtException(t, e);
            }
        } else {
            StringBuffer builder = new StringBuffer();
            builder.append(new SimpleDateFormat("yyyy/MM/dd HH:mm:ss").format(new Date())).append("\n")
                    .append(collectDeviceInfo(mContext))
                    .append(getCrashMessage(e))
                    .append("\n\n");
            try {
                File logDir = new File(Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "demo_reader");
                if (!logDir.exists()) {
                    logDir.mkdirs();
                }
                File logFile = new File(logDir, new SimpleDateFormat("yyyyMMdd").format(new Date()) + ".txt");
                FileOutputStream fos = new FileOutputStream(logFile, true);
                fos.write(builder.toString().getBytes());
                fos.close();
                new Thread() {
                    @Override
                    public void run() {
                        Looper.prepare();
                        Toast.makeText(mContext, "挂了", Toast.LENGTH_SHORT).show();
                        Looper.loop();
                    }
                }.start();
            } catch (Exception e2) {
            }

        }
    }

    public String collectDeviceInfo(Context context) {
        StringBuilder builder = new StringBuilder();
        String versionName = null;
        String versionCode = null;
        try {
            PackageManager pm = context.getPackageManager();
            PackageInfo pi = pm.getPackageInfo(context.getPackageName(), PackageManager.GET_ACTIVITIES);
            if (pi != null) {
                versionName = pi.versionName == null ? "null" : pi.versionName;
                versionCode = pi.versionCode + "";
            }
        } catch (PackageManager.NameNotFoundException e) {
        }
        builder.append("versionName=").append(versionName).append("\n")
                .append("versionCode=").append(versionCode).append("\n")
                .append("Brand=").append(Build.BRAND).append(" Model=").append(Build.MODEL).append("\n")
                .append("Android=").append(Build.VERSION.RELEASE).append(" SDK=").append(Build.VERSION.SDK_INT).append("\n")
                .append("width=").append(context.getResources().getDisplayMetrics().widthPixels).append(" height=").append(context.getResources().getDisplayMetrics().heightPixels).append("\n")
                .append("density=").append(context.getResources().getDisplayMetrics().density).append("\n");
        return builder.toString();
    }

    private String getCrashMessage(Throwable e) {
        StringBuffer builder = new StringBuffer();

        Writer writer = new StringWriter();
        PrintWriter printWriter = new PrintWriter(writer);
        e.printStackTrace(printWriter);
        Throwable cause = e.getCause();
        while (cause != null) {
            cause.printStackTrace(printWriter);
            cause = cause.getCause();
        }
        printWriter.close();
        builder.append(writer.toString());

        return builder.toString();
    }

}
