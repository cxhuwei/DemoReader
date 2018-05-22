package com.chaoxing.epub;

import android.app.Application;
import android.arch.lifecycle.AndroidViewModel;
import android.arch.lifecycle.LiveData;
import android.arch.lifecycle.MutableLiveData;
import android.arch.lifecycle.Transformations;
import android.support.annotation.NonNull;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by HUWEI on 2018/5/14.
 */
public class EpubViewModel extends AndroidViewModel {

    private List<Integer> mFileIdList = new ArrayList<>();
    private List<PageKey> mPageKeyList = new ArrayList<>();

    private EpubLoader mEpubLoader = new EpubLoader();
    private PageLoader mPageLoader = new PageLoader();

    private final MutableLiveData<String> mInitDocument = new MutableLiveData<>();
    private LiveData<Resource<DocumentBinding>> mInitDocumentResult;

    private final MutableLiveData<DocumentBinding> mOpenDocument = new MutableLiveData<>();
    private LiveData<Resource<Void>> mOpenDocumentResult;

    private final MutableLiveData<DocumentBinding> mLoadFileCount = new MutableLiveData<>();
    private LiveData<Resource<Integer>> mLoadFileCountResult;

    private final MutableLiveData<Integer> mLoadPageCountByFile = new MutableLiveData<>();
    private LiveData<Resource<EpubFile>> mLoadPageCountByFileResult;

    private final MutableLiveData<EpubPage> mLoadPage = new MutableLiveData<>();
    private LiveData<Resource<EpubPage>> mLoadPageResult;

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

        mLoadPageResult = Transformations.switchMap(mLoadPage, epubPage -> {
            return mPageLoader.loadPage(getDocumentBinding(), epubPage);
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
        for (Integer id : mFileIdList) {
            if (id == fileId) {
                return;
            }
        }
        mFileIdList.add(fileId);
        mLoadPageCountByFile.setValue(fileId);
    }

    public void removeLoadingFileId(int fileId) {
        for (int i = 0; i < mFileIdList.size(); i++) {
            Integer id = mFileIdList.get(i);
            if (id == fileId) {
                mFileIdList.remove(i);
                break;
            }
        }
    }

    public LiveData<Resource<EpubPage>> getLoadPageResult() {
        return mLoadPageResult;
    }

    public void loadPage(EpubPage page) {
        for (PageKey key : mPageKeyList) {
            if (key.getFileId() == page.getFileId() && key.getPageNumber() == page.getPageNumber()) {
                return;
            }
        }
        mPageKeyList.add(new PageKey(page.getFileId(), page.getPageNumber()));
        EpubPage newPage = page.copy();
        mLoadPage.setValue(newPage);
    }

    public void removeLoadingPage(PageKey pageKey) {
        for (PageKey key : mPageKeyList) {
            if (key.getFileId() == pageKey.getFileId() && key.getPageNumber() == pageKey.getPageNumber()) {
                mPageKeyList.remove(key);
                break;
            }
        }
    }
}
