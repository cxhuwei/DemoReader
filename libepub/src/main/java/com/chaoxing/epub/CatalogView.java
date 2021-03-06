package com.chaoxing.epub;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.widget.RecyclerView;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

/**
 * Created by HUWEI on 2018/4/27.
 */
public class CatalogView extends FrameLayout {

    private RecyclerView mRvCatelog;
    private CatelogAdapter mAdapter;
    private View mLoadingStatus;
    private ProgressBar mLoadingView;
    private TextView mTvMessage;
    private Button mBtnRetry;

    public CatalogView(@NonNull Context context) {
        this(context, null);
    }

    public CatalogView(@NonNull Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public CatalogView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initView();
    }

    protected void initView() {
        View contentView = LayoutInflater.from(getContext()).inflate(R.layout.libepub_catelog_view, this);

    }

}
