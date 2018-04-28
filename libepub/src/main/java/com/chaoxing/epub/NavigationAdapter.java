package com.chaoxing.epub;

import android.support.annotation.NonNull;
import android.support.v4.view.PagerAdapter;
import android.view.View;
import android.view.ViewGroup;

import java.util.List;

/**
 * Created by HUWEI on 2018/4/27.
 */
public class NavigationAdapter extends PagerAdapter {

    private List<View> mNavigationViewList;

    public NavigationAdapter(List<View> navigationViewList) {
        mNavigationViewList = navigationViewList;
    }

    @Override
    public int getCount() {
        return mNavigationViewList == null ? 0 : mNavigationViewList.size();
    }

    @Override
    public boolean isViewFromObject(@NonNull View view, @NonNull Object object) {
        return view == object;
    }

    @NonNull
    @Override
    public Object instantiateItem(@NonNull ViewGroup container, int position) {
        View childView = mNavigationViewList.get(position);
        container.addView(childView);
        return childView;
    }

    @Override
    public void destroyItem(@NonNull ViewGroup container, int position, @NonNull Object object) {
        container.removeView(mNavigationViewList.get(position));
    }

}
