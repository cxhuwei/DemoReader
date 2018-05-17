package com.chaoxing.epub.util;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Color;
import android.os.Build;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.Window;
import android.view.WindowManager;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/**
 * Created by HUWEI on 2018/4/25.
 */
public class EpubUtils {

    public static boolean isSupportLayoutFullscreen() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.M;
    }

    public static boolean checkDeviceHasNavigationBar(Context context) {
        boolean hasMenuKey = ViewConfiguration.get(context).hasPermanentMenuKey();
        boolean hasBackKey = KeyCharacterMap.deviceHasKey(KeyEvent.KEYCODE_BACK);
        if (!hasMenuKey && !hasBackKey) {
            return true;
        }
        return false;
    }

    public static void setSystemUIVisible(Activity activity, boolean visible) {
        Window window = activity.getWindow();
        View decorView = window.getDecorView();
        if (visible) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
                window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
                window.setStatusBarColor(Color.TRANSPARENT);
                int newUiOptions = View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;

                newUiOptions |= View.SYSTEM_UI_FLAG_LAYOUT_STABLE;
                newUiOptions |= View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN;
                newUiOptions |= View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION;

                decorView.setSystemUiVisibility(newUiOptions);
            } else {
                WindowManager.LayoutParams params = window.getAttributes();
                params.flags |= WindowManager.LayoutParams.FLAG_FULLSCREEN;
                window.setAttributes(params);
            }
        } else {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
                window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
                window.setStatusBarColor(Color.TRANSPARENT);

                int newUiOptions = View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
                newUiOptions |= View.SYSTEM_UI_FLAG_FULLSCREEN;
                newUiOptions |= View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION;
                newUiOptions |= View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;
                newUiOptions |= View.SYSTEM_UI_FLAG_IMMERSIVE;
                decorView.setSystemUiVisibility(newUiOptions);
            } else {

            }
        }
    }

    public static int getStatusBarHeight(Context context) {
        int statusBarHeight = 0;
        Resources res = context.getResources();
        int resourceId = res.getIdentifier("status_bar_height", "dimen", "android");
        if (resourceId > 0) {
            statusBarHeight = res.getDimensionPixelSize(resourceId);
        }
        return statusBarHeight;
    }

    public static int getNavigationBarHeight(Context context) {
        int statusBarHeight = 0;
        Resources res = context.getResources();
        int resourceId = res.getIdentifier("navigation_bar_height", "dimen", "android");
        if (resourceId > 0) {
            statusBarHeight = res.getDimensionPixelSize(resourceId);
        }
        return statusBarHeight;
    }

    public static boolean isBlank(String str) {
        return str == null || str.trim().length() == 0;
    }

    public static boolean isEmpty(String str) {
        return str == null || str.isEmpty();
    }

    public static int dp2px(Context context, int dp) {
        final float density = context.getResources().getDisplayMetrics().density;
        return (int) (dp * density + 0.5f);
    }

    public static String getFileExtension(File file) {
        String fileName = file.getName();
        int index = fileName.lastIndexOf('.');
        if (index >= 0) {
            return fileName.substring(index + 1).trim().toLowerCase();
        }
        return "";
    }

    public static String md5(String str) {
        if (str != null) {
            try {
                MessageDigest md = MessageDigest.getInstance("MD5");
                md.update(str.getBytes());
                byte digest[] = md.digest();
                StringBuilder strBuilder = new StringBuilder();
                for (int i = 0; i < digest.length; i++) {
                    strBuilder.append(String.format("%02x", digest[i]));
                }
                return strBuilder.toString();
            } catch (NoSuchAlgorithmException e) {
                e.printStackTrace();
            }
        }

        return "";
    }

    public static String md5(File file) {
        if (file != null && file.exists() && file.isFile()) {
            try {
                MessageDigest md = MessageDigest.getInstance("MD5");
                FileInputStream fis = new FileInputStream(file);

                int bufferSize = 2 * 1024 * 1024;
                if (file.length() < bufferSize) {
                    bufferSize = (int) file.length();
                }
                byte[] buffer = new byte[bufferSize];

                int count = 0;
                while ((count = fis.read(buffer)) != -1) {
                    md.update(buffer, 0, count);
                }
                fis.close();

                byte[] digest = md.digest();
                StringBuilder strBuilder = new StringBuilder();
                for (int i = 0; i < digest.length; i++) {
                    strBuilder.append(String.format("%02x", digest[i]));
                }
                return strBuilder.toString();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return "";
    }

    public static String md5(InputStream inputStream) {
        try {
            MessageDigest md = MessageDigest.getInstance("MD5");
            int bufferSize = 1 * 1024 * 1024;
            byte[] buffer = new byte[bufferSize];
            int count = 0;
            while ((count = inputStream.read(buffer)) != -1) {
                md.update(buffer, 0, count);
            }
            inputStream.close();

            byte[] digest = md.digest();
            StringBuilder strBuilder = new StringBuilder();
            for (int i = 0; i < digest.length; i++) {
                strBuilder.append(String.format("%02x", digest[i]));
            }
            return strBuilder.toString();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return "";
    }

    public static boolean equals(Object a, Object b) {
        return (a == b) || (a != null && a.equals(b));
    }

    public static void assetsCopy(Context context, String src, String dst) throws Throwable {
        InputStream is = context.getAssets().open(src);
        File file = new File(dst);
        File dir = file.getParentFile();
        if (!dir.exists()) {
            dir.mkdirs();
        }
        FileOutputStream fos = new FileOutputStream(dst);
        byte[] buffer = new byte[1024 * 1024];
        int length = -1;
        while ((length = is.read(buffer)) != -1) {
            fos.write(buffer, 0, length);
        }
        fos.close();
        is.close();
    }

}
