package com.chaoxing.epub;

import android.arch.lifecycle.LiveData;
import android.arch.lifecycle.MediatorLiveData;
import android.support.annotation.MainThread;
import android.support.annotation.NonNull;
import android.support.annotation.WorkerThread;

import com.chaoxing.epub.util.EpubUtils;

import io.reactivex.Observable;
import io.reactivex.ObservableEmitter;
import io.reactivex.ObservableOnSubscribe;
import io.reactivex.Observer;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.disposables.Disposable;
import io.reactivex.schedulers.Schedulers;

/**
 * Created by HUWEI on 2018/4/18.
 */
public abstract class ExecuteBoundResource<Params, Result> {

    private Params params;

    private final MediatorLiveData<Resource<Result>> result = new MediatorLiveData<>();

    private Observable observable = Observable.create(new ObservableOnSubscribe<Resource<Result>>() {
        @Override
        public void subscribe(ObservableEmitter<Resource<Result>> emitter) throws Exception {
            emitter.onNext(onExecute(params));
        }
    }).subscribeOn(Schedulers.computation()).observeOn(AndroidSchedulers.mainThread());

    public ExecuteBoundResource() {
    }

    @MainThread
    protected void setValue(Resource<Result> newValue) {
        if (!EpubUtils.equals(result.getValue(), newValue)) {
            result.setValue(newValue);
        }
    }

    protected void onReady(Params params) {
        setValue(Resource.loading(null));
    }

    public final LiveData<Resource<Result>> execute(Params params) {
        this.params = params;
        onReady(params);
        observable.subscribe(new Observer<Resource<Result>>() {
            @Override
            public void onSubscribe(Disposable d) {
            }

            @Override
            public void onNext(Resource<Result> result) {
                setValue(result);
            }

            @Override
            public void onError(Throwable e) {
                setValue(Resource.error(e.getMessage(), null));
            }

            @Override
            public void onComplete() {
            }
        });

        return result;
    }

    public void cancel() {

    }

    @WorkerThread
    protected abstract @NonNull
    Resource<Result> onExecute(Params params);

}
