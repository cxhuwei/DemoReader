package com.chaoxing.epub;

import android.app.Application;
import android.arch.lifecycle.LiveData;
import android.content.Context;
import android.support.annotation.NonNull;
import android.util.Log;

import com.chaoxing.epub.nativeapi.EpubDocument;
import com.chaoxing.epub.nativeapi.EpubInfo;
import com.chaoxing.epub.util.EpubUtils;

import java.io.File;

/**
 * Created by HUWEI on 2018/5/14.
 */
public class EpubLoader {

    public LiveData<Resource<DocumentBinding>> initDocument(String documentPath) {
        ExecuteBoundResource<String, DocumentBinding> execute = new ExecuteBoundResource<String, DocumentBinding>() {
            @NonNull
            @Override
            protected Resource<DocumentBinding> onExecute(String path) {
                Resource result = null;
                try {
                    File file = new File(path);
                    if (!file.exists() || !file.isFile()) {
                        result = Resource.error("文件不存在", null);
                    } else {
                        DocumentBinding binding = new DocumentBinding();
                        binding.setPath(path);
                        String md5 = EpubUtils.md5(file);
                        binding.setMd5(md5);
                        result = Resource.success(binding);
                    }
                } catch (Throwable e) {
                    e.printStackTrace();
                    result = Resource.error(e.getMessage(), null);
                }
                return result;
            }
        };
        return execute.execute(documentPath);
    }

    public LiveData<Resource<Void>> openDocument(final Application application, DocumentBinding documentBinding) {
        ExecuteBoundResource<DocumentBinding, Void> execute = new ExecuteBoundResource<DocumentBinding, Void>() {
            @NonNull
            @Override
            protected Resource<Void> onExecute(DocumentBinding binding) {
                Resource result = null;
                try {
//                    String fontPath = getFontPath(application);
//                    int setFontResource = -1;
//                    if (fontPath != null) {
//                        setFontResource = EpubDocument.get().nativeSetFontResource(new String[]{fontPath, fontPath});
//                    }
//                    int layout = EpubDocument.get().nativeLayout(binding.getWidth(), binding.getHeight(), 0, 0, binding.getWidth(), binding.getHeight(), binding.getDensity());
//                    int background = EpubDocument.get().nativeSetBackgroundColor(Color.WHITE);
//                    int textLevel = EpubDocument.get().nativeSetTextLevel(0);
//                    EpubInfo info = null;
//                    if (setFontResource == 0 && layout == 0 && background == 0 && textLevel == 0) {
//                        info = EpubDocument.get().nativeOpenDocument(binding.getPath());
//                    }

                    EpubInfo info = EpubDocument.get().nativeOpenDocument(binding.getPath());
                    if (info != null) {
                        Log.i(EpubActivity.TAG, info.toString());
                        result = Resource.success(null);
                    } else {
                        result = Resource.error("打开失败", null);
                    }
                } catch (Throwable e) {
                    e.printStackTrace();
                    result = Resource.error(e.getMessage(), null);
                }
                return result;
            }
        };
        return execute.execute(documentBinding);
    }

    private String getFontPath(Context context) throws Throwable {
        String fontPath = null;
        File fontDir = new File(context.getExternalCacheDir(), File.separator + "epub" + File.separator + "font");
        if (!fontDir.exists()) {
            fontDir.mkdirs();
        }
        File fontFile = new File(fontDir, "fzlthk.ttf");
        String assetsFontFile = "font" + File.separator + "fzlthk.ttf";
        if (fontFile.exists()) {
            String fontMd5 = EpubUtils.md5(fontFile);
            String assetsFontMd5 = EpubUtils.md5(context.getAssets().open(assetsFontFile));
            if (!EpubUtils.equals(fontMd5, assetsFontMd5)) {
                if (fontFile.delete()) {
                    EpubUtils.assetsCopy(context, assetsFontFile, fontFile.getAbsolutePath());
                    fontPath = fontFile.getAbsolutePath();
                }
            } else {
                fontPath = fontFile.getAbsolutePath();
            }
        } else {
            EpubUtils.assetsCopy(context, assetsFontFile, fontFile.getAbsolutePath());
            fontPath = fontFile.getAbsolutePath();
        }
        return fontPath;
    }

    public LiveData<Resource<Integer>> loadFileCount(DocumentBinding documentBinding) {
        ExecuteBoundResource<DocumentBinding, Integer> execute = new ExecuteBoundResource<DocumentBinding, Integer>() {
            @NonNull
            @Override
            protected Resource<Integer> onExecute(DocumentBinding documentBinding) {
                Resource<Integer> result = null;
                try {
                    int fileCount = EpubDocument.get().nativeGetFileCount();
                    result = Resource.success(fileCount);
                } catch (Throwable e) {
                    e.printStackTrace();
                    result = Resource.error(e.getMessage(), null);
                }
                return result;
            }
        };
        return execute.execute(documentBinding);
    }

    public LiveData<Resource<EpubFile>> loadPageCountByFile(int fileId) {
        ExecuteBoundResource<Integer, EpubFile> exexute = new ExecuteBoundResource<Integer, EpubFile>() {

            @Override
            protected void onReady(Integer id) {
                setValue(Resource.loading(new EpubFile(id)));
            }

            @NonNull
            @Override
            protected Resource<EpubFile> onExecute(Integer id) {
                Resource<EpubFile> result = null;
                EpubFile epubFile = new EpubFile();
                epubFile.setId(id);
                try {
                    int pageCount = EpubDocument.get().nativeGetPageCountByFile(id);
                    if (pageCount >= 0) {
                        epubFile.setPageCount(pageCount);
                        result = Resource.success(epubFile);
                    } else {
                        result = Resource.error("文件页面计算错误(FILE_ID=" + id + ")", epubFile);
                    }
                } catch (Throwable e) {
                    e.printStackTrace();
                    result = Resource.error(e.getMessage(), epubFile);
                }
                return result;
            }
        };
        return exexute.execute(fileId);
    }


}
