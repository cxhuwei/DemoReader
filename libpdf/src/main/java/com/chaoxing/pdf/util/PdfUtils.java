package com.chaoxing.pdf.util;

import android.content.Context;

import java.io.File;
import java.io.FileInputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/**
 * Created by HUWEI on 2018/4/18.
 */
public class PdfUtils {

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

    public static boolean equals(Object a, Object b) {
        return (a == b) || (a != null && a.equals(b));
    }

}
