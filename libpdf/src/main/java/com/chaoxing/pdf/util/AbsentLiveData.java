package com.chaoxing.pdf.util;

import android.arch.lifecycle.LiveData;

/**
 * Created by HUWEI on 2018/4/18.
 */
public class AbsentLiveData extends LiveData {

    private AbsentLiveData() {
        postValue(null);
    }

    public static <T> LiveData<T> create() {
        return new AbsentLiveData();
    }

}