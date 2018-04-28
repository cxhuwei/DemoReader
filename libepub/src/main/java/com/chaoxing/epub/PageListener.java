package com.chaoxing.epub;

import android.view.MotionEvent;

/**
 * Created by HUWEI on 2018/4/27.
 */
public interface PageListener {

    void onClicked(MotionEvent e);

    void onDoubleClicked(MotionEvent e);

    void onLongClicked(MotionEvent e);

    void loadPage(EpubPage page);

}
