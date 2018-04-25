package com.chaoxing.pdf;

import android.app.Application;
import android.arch.lifecycle.AndroidViewModel;
import android.arch.lifecycle.LiveData;
import android.arch.lifecycle.MutableLiveData;
import android.arch.lifecycle.Transformations;
import android.support.annotation.NonNull;
import android.util.DisplayMetrics;

/**
 * Created by HUWEI on 2018/4/18.
 */
public class PDFViewModel extends AndroidViewModel {

    private PdfLoader mPdfLoader = new PdfLoader();
    private PdfPageLoader mPageLoader;

    private final MutableLiveData<String> mPath = new MutableLiveData<>();
    private LiveData<Resource<PdfBinding>> mOpenDocumentResult;

    private final MutableLiveData<String> mPassword = new MutableLiveData<>();
    private LiveData<Resource<Boolean>> mCheckPasswordResult;

    private final MutableLiveData<PdfBinding> mLoadDocument = new MutableLiveData<>();
    private LiveData<Resource<PdfBinding>> mLoadDocumentResult;

    private final MutableLiveData<Integer[]> mLoadPage = new MutableLiveData<>();
    private LiveData<Resource<PdfPage>> mLoadPageResult;

    private final MutableLiveData<PdfBinding> mLoadOutline = new MutableLiveData<>();
    private LiveData<Resource<PdfBinding>> mLoadOutlineResult;


    public PDFViewModel(@NonNull Application application) {
        super(application);
        mOpenDocumentResult = Transformations.switchMap(mPath, documentPath -> {
            return mPdfLoader.openDocument(documentPath);
        });

        mCheckPasswordResult = Transformations.switchMap(mPassword, password -> {
            return mPdfLoader.checkPassword(mOpenDocumentResult.getValue().getData().getDocument(), password);
        });

        mLoadDocumentResult = Transformations.switchMap(mLoadDocument, pdfBinding -> {
            DisplayMetrics dm = getApplication().getResources().getDisplayMetrics();
            return mPdfLoader.loadDocument(pdfBinding, dm.widthPixels, dm.heightPixels);
        });

        mLoadPageResult = Transformations.switchMap(mLoadPage, (args) -> {
            if (mPageLoader == null) {
                mPageLoader = new PdfPageLoader(getApplication(), getPdfBinding());
            }
            int pageNumber = args[0];
            int width = args[1];
            return mPageLoader.loadPage(pageNumber, width);
        });

        mLoadOutlineResult = Transformations.switchMap(mLoadOutline, (pdfBinding) -> {
            return mPdfLoader.loadOutline(pdfBinding);
        });
    }


    public void openDocument(String path) {
        mPath.setValue(path);
    }

    public LiveData<Resource<PdfBinding>> getOpenDocumentResult() {
        return mOpenDocumentResult;
    }

    public void checkPassword(final String password) {
        mPassword.setValue(password);
    }

    public LiveData<Resource<Boolean>> getCheckPasswordResult() {
        return mCheckPasswordResult;
    }

    public void loadDocument() {
        mLoadDocument.setValue(getOpenDocumentResult().getValue().getData());
    }

    public LiveData<Resource<PdfBinding>> getLoadDocumentResult() {
        return mLoadDocumentResult;
    }

    public PdfBinding getPdfBinding() {
        return mLoadDocumentResult.getValue() != null ? mLoadDocumentResult.getValue().getData() : null;
    }

    public void loadPage(int pageNumber, int width) {
        mLoadPage.setValue(new Integer[]{pageNumber, width});
    }

    public LiveData<Resource<PdfPage>> getLoadPageResult() {
        return mLoadPageResult;
    }

    public void loadOutline(PdfBinding pdfBinding) {
        mLoadOutline.setValue(pdfBinding);
    }

    public LiveData<Resource<PdfBinding>> getLoadOutlineResult() {
        return mLoadOutlineResult;
    }

}
