package com.chaoxing.epub.util;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.provider.DocumentsContract;
import android.provider.MediaStore;

/**
 * Created by HUWEI on 2018/4/18.
 */
public class UriUtils {

    public static String getRealPath(Context context, Uri uri) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            return getUriRealPathAboveKitkat(context, uri);
        } else {
            return getRealPath(context.getContentResolver(), uri, null);
        }
    }

    private static String getUriRealPathAboveKitkat(Context context, Uri uri) {
        if (context == null || uri == null) {
            return null;
        }

        String path = null;

        if (isFileUri(uri)) {
            path = uri.getPath();
        } else if (isDocumentUri(context, uri)) {
            String documentId = "";
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
                documentId = DocumentsContract.getDocumentId(uri);
            }
            String uriAuthority = uri.getAuthority();

            if (isMediaDoc(uriAuthority)) {
                String idArr[] = documentId.split(":");
                if (idArr.length == 2) {
                    String docType = idArr[0];
                    String realDocId = idArr[1];

                    Uri mediaContentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;

                    if ("image".equals(docType)) {
                        mediaContentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
                    } else if ("video".equals(docType)) {
                        mediaContentUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
                    } else if ("audio".equals(docType)) {
                        mediaContentUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
                    }

                    String whereClause = MediaStore.Images.Media._ID + " = " + realDocId;

                    path = getRealPath(context.getContentResolver(), mediaContentUri, whereClause);
                }
            } else if (isDownloadDoc(uriAuthority)) {
                Uri downloadUri = Uri.parse("content://downloads/public_downloads");
                Uri downloadUriAppendId = ContentUris.withAppendedId(downloadUri, Long.valueOf(documentId));
                path = getRealPath(context.getContentResolver(), downloadUriAppendId, null);
            } else if (isExternalStoreDoc(uriAuthority)) {
                String idArr[] = documentId.split(":");
                if (idArr.length == 2) {
                    String type = idArr[0];
                    String realDocId = idArr[1];

                    if ("primary".equalsIgnoreCase(type)) {
                        path = Environment.getExternalStorageDirectory() + "/" + realDocId;
                    }

                }
            }
        } else if (isContentUri(uri)) {
            if (isGooglePhotoDoc(uri.getAuthority())) {
                path = uri.getLastPathSegment();
            } else {
                path = getRealPath(context.getContentResolver(), uri, null);
            }
        }

        return path;
    }

    private static String getRealPath(ContentResolver contentResolver, Uri uri, String whereClause) {
        String path = null;

        String columnName = MediaStore.Files.FileColumns.DATA;
        if (isFileUri(uri)) {
            path = uri.getPath();
        } else if (uri == MediaStore.Images.Media.EXTERNAL_CONTENT_URI) {
            columnName = MediaStore.Images.Media.DATA;
        } else if (uri == MediaStore.Audio.Media.EXTERNAL_CONTENT_URI) {
            columnName = MediaStore.Audio.Media.DATA;
        } else if (uri == MediaStore.Video.Media.EXTERNAL_CONTENT_URI) {
            columnName = MediaStore.Video.Media.DATA;
        }

        Cursor cursor = contentResolver.query(uri, new String[]{columnName}, whereClause, null, null);

        if (cursor != null) {
            if (cursor.moveToFirst()) {
                int columnIndex = cursor.getColumnIndex(columnName);
                if (columnIndex >= 0) {
                    path = cursor.getString(columnIndex);
                }
            }
            cursor.close();
        }

        return path;
    }

    private static boolean isDocumentUri(Context context, Uri uri) {
        if (context == null || uri == null) {
            return false;
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            return DocumentsContract.isDocumentUri(context, uri);
        }
        return false;
    }

    private static boolean isContentUri(Uri uri) {
        if (uri == null) {
            return false;
        }

        return "content".equalsIgnoreCase(uri.getScheme());
    }

    private static boolean isFileUri(Uri uri) {
        if (uri == null) {
            return false;
        }
        return "file".equalsIgnoreCase(uri.getScheme());

    }

    private static boolean isExternalStoreDoc(String uriAuthority) {
        if (uriAuthority == null || uriAuthority.trim().length() == 0) {
            return false;
        }

        return "com.android.externalstorage.documents".equals(uriAuthority);
    }

    private static boolean isDownloadDoc(String uriAuthority) {
        if (uriAuthority == null || uriAuthority.trim().length() == 0) {
            return false;
        }

        return "com.android.providers.downloads.documents".equals(uriAuthority);
    }

    private static boolean isMediaDoc(String uriAuthority) {
        if (uriAuthority == null || uriAuthority.trim().length() == 0) {
            return false;
        }

        return "com.android.providers.media.documents".equals(uriAuthority);
    }

    private static boolean isGooglePhotoDoc(String uriAuthority) {
        return "com.google.android.apps.photos.content".equals(uriAuthority);
    }

}
