package com.chaoxing.epub;

import android.app.Application;
import android.arch.lifecycle.AndroidViewModel;
import android.arch.lifecycle.LiveData;
import android.arch.lifecycle.MutableLiveData;
import android.arch.lifecycle.Transformations;
import android.support.annotation.NonNull;

/**
 * Created by HUWEI on 2018/5/14.
 */
public class EpubViewModel extends AndroidViewModel {

    private EpubLoader mEpubLoader = new EpubLoader();

    private final MutableLiveData<String> mInitDocument = new MutableLiveData<>();
    private LiveData<Resource<DocumentBinding>> mInitDocumentResult;

    private final MutableLiveData<DocumentBinding> mOpenDocument = new MutableLiveData<>();
    private LiveData<Resource<Void>> mOpenDocumentResult;

    private final MutableLiveData<DocumentBinding> mLoadFileCount = new MutableLiveData<>();
    private LiveData<Resource<Integer>> mLoadFileCountResult;

    private final MutableLiveData<Integer> mLoadPageCountByFile = new MutableLiveData<>();
    private LiveData<Resource<EpubFile>> mLoadPageCountByFileResult;

    public EpubViewModel(@NonNull Application application) {
        super(application);
        mInitDocumentResult = Transformations.switchMap(mInitDocument, documentPath -> {
            return mEpubLoader.initDocument(documentPath);
        });

        mOpenDocumentResult = Transformations.switchMap(mOpenDocument, documentBinding -> {
            return mEpubLoader.openDocument(getApplication(), documentBinding);
        });

        mLoadFileCountResult = Transformations.switchMap(mLoadFileCount, documentBinding -> {
            return mEpubLoader.loadFileCount(documentBinding);
        });

        mLoadPageCountByFileResult = Transformations.switchMap(mLoadPageCountByFile, fileId -> {
            return mEpubLoader.loadPageCountByFile(fileId);
        });
    }

    public DocumentBinding getDocumentBinding() {
        return mInitDocumentResult.getValue() == null ? null : mInitDocumentResult.getValue().getData();
    }

    public LiveData<Resource<DocumentBinding>> getInitDocumentResult() {
        return mInitDocumentResult;
    }

    public void initDocument(String path) {
        mInitDocument.setValue(path);
    }

    public LiveData<Resource<Void>> getOpenDocumentResult() {
        return mOpenDocumentResult;
    }

    public void openDocument() {
        mOpenDocument.setValue(mInitDocumentResult.getValue().getData());
    }

    public LiveData<Resource<Integer>> getLoadFileCountResult() {
        return mLoadFileCountResult;
    }

    public void loadFileCount() {
        mLoadFileCount.setValue(mInitDocumentResult.getValue().getData());
    }

    public LiveData<Resource<EpubFile>> getLoadPageCountByFileResult() {
        return mLoadPageCountByFileResult;
    }

    public void loadPageCountByFile(int fileId) {
        mLoadPageCountByFile.setValue(fileId);
    }

}
