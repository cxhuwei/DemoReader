package com.chaoxing.epub;

import android.arch.lifecycle.LiveData;
import android.graphics.Bitmap;
import android.support.annotation.NonNull;

import com.chaoxing.epub.nativeapi.EpubDocument;

/**
 * Created by HUWEI on 2018/5/22.
 */
public class PageLoader {

    public LiveData<Resource<EpubPage>> loadPage(DocumentBinding documentBinding, EpubPage epubPage) {
        int width = documentBinding.getWidth();
        int height = documentBinding.getHeight();
        ExecuteBoundResource<Object[], EpubPage> execute = new ExecuteBoundResource<Object[], EpubPage>() {
            @Override
            protected void onReady(Object[] params) {
                setValue(Resource.loading((EpubPage) params[0]));
            }

            @NonNull
            @Override
            protected Resource<EpubPage> onExecute(Object[] params) {
                EpubPage page = (EpubPage) params[0];
                int width = (int) params[1];
                int height = (int) params[2];
                Resource<EpubPage> result;
                try {
                    Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
                    int r = EpubDocument.get().nativeDrawPage(page.getFileId(), page.getPageNumber(), bitmap);
                    if (r == 0) {
                        page.setBitmap(bitmap);
                        result = Resource.success(page);
                    } else {
                        bitmap.recycle();
                        result = Resource.error(String.format("页面绘制错误(error=%d)", r), page);
                    }
                } catch (Throwable e) {
                    e.printStackTrace();
                    result = Resource.error(e.getMessage(), page);
                }
                return result;
            }
        };
        return execute.execute(new Object[]{epubPage, width, height});
    }


}
