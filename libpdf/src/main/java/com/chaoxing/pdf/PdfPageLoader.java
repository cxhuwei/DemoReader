package com.chaoxing.pdf;

import android.arch.lifecycle.LiveData;
import android.arch.lifecycle.MediatorLiveData;
import android.content.Context;
import android.graphics.Bitmap;

import com.artifex.mupdf.fitz.Link;
import com.artifex.mupdf.fitz.Matrix;
import com.artifex.mupdf.fitz.Page;
import com.artifex.mupdf.fitz.android.AndroidDrawDevice;

import java.io.File;
import java.io.FileOutputStream;
import java.util.HashSet;
import java.util.Set;

import io.reactivex.Observable;
import io.reactivex.ObservableEmitter;
import io.reactivex.Observer;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.disposables.Disposable;
import io.reactivex.schedulers.Schedulers;

/**
 * Created by HUWEI on 2018/4/18.
 */
public class PdfPageLoader {

    private final MediatorLiveData<Resource<PdfPage>> result = new MediatorLiveData<>();

    private Context mContext;
    private PdfBinding mPdfBinding;
    private File mCacheDirectory;

    private Set<String> mLoading = new HashSet<>();

    public PdfPageLoader(Context context, PdfBinding pdfBinding) {
        mContext = context;
        mPdfBinding = pdfBinding;
        mCacheDirectory = new File(context.getExternalCacheDir(), "pdf");
        if (!mCacheDirectory.exists()) {
            mCacheDirectory.mkdirs();
        }
    }

    public synchronized LiveData<Resource<PdfPage>> loadPage(int pageNumber, int width) {
        String key = String.format("%04x%4d", pageNumber, width);
        if (mLoading.contains(key)) {
            result.setValue(Resource.loading(new PdfPage(pageNumber)));
        } else {

            mLoading.add(key);

            Observable.create((ObservableEmitter<Resource<PdfPage>> emitter) -> {
                emitter.onNext(Resource.loading(new PdfPage(pageNumber)));
                emitter.onNext(drawPage(pageNumber, width));
            }).subscribeOn(Schedulers.io()).observeOn(AndroidSchedulers.mainThread())
                    .subscribe(new Observer<Resource<PdfPage>>() {
                        Disposable d;

                        @Override
                        public void onSubscribe(Disposable d) {
                            this.d = d;
                        }

                        @Override
                        public void onNext(Resource<PdfPage> pageResource) {
                            if (!pageResource.isLoading()) {
                                mLoading.remove(key);
                            }
                            result.setValue(pageResource);
                        }

                        @Override
                        public void onError(Throwable e) {
                            mLoading.remove(key);
                            result.setValue(Resource.error(e.getMessage(), new PdfPage(pageNumber)));
                        }

                        @Override
                        public void onComplete() {

                        }
                    });

        }
        return result;
    }

    private Resource<PdfPage> drawPage(int pageNumber, int fitWidth) {
        Resource<PdfPage> pageResource = null;
        PdfPage pdfPage = new PdfPage(pageNumber);
        try {
            Page page = mPdfBinding.getDocument().loadPage(pageNumber);
            Matrix matrix = AndroidDrawDevice.fitPageWidth(page, fitWidth);

            File pageFile = null;
            File file = getPageFile(pageNumber, fitWidth);
            if (file.exists() && file.isFile()) {
                pageFile = file;
            } else {
                Bitmap bitmap = AndroidDrawDevice.drawPage(page, matrix);
                if (bitmap != null) {
                    FileOutputStream fos = new FileOutputStream(file);
                    bitmap.compress(Bitmap.CompressFormat.JPEG, 80, fos);
                    fos.close();
                    pageFile = file;
                } else {
                    pageResource = Resource.error("页面加载失败", pdfPage);
                }
            }

            if (pageFile != null) {
                pdfPage.setPageFile(pageFile.getAbsolutePath());
                Link[] links = page.getLinks();
                if (links != null) {
                    for (Link link : links) {
                        link.bounds.transform(matrix);
                    }
                }
                pdfPage.setLinks(links);
                page.destroy();
                pageResource = Resource.success(pdfPage);
            }
        } catch (Throwable t) {
            t.printStackTrace();
            pageResource = Resource.error(t.getMessage(), pdfPage);
        }
        return pageResource;
    }

    private File getPageFile(int pageNumber, int width) {
        return new File(mCacheDirectory, mPdfBinding.getMd5() + String.format("%04x%4d", pageNumber, width) + ".tmp");
    }

}
