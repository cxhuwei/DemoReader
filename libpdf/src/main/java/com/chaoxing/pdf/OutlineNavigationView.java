package com.chaoxing.pdf;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.widget.DividerItemDecoration;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

import java.util.List;

/**
 * Created by HUWEI on 2018/4/18.
 */
public class OutlineNavigationView extends FrameLayout {

    private RecyclerView mRvOutline;
    private OutlineAdapter mAdapter;
    private View mLoadingStatus;
    private ProgressBar mLoadingView;
    private TextView mTvMessage;
    private Button mBtnRetry;

    private OutlineListener outlineListener;

    public OutlineNavigationView(@NonNull Context context) {
        this(context, null);
    }

    public OutlineNavigationView(@NonNull Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public OutlineNavigationView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init() {
        View contentView = LayoutInflater.from(getContext()).inflate(R.layout.libpdf_outline, this, true);
        mRvOutline = contentView.findViewById(R.id.rv_outline);
        mRvOutline.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        mRvOutline.setHasFixedSize(true);
        DividerItemDecoration itemDecoration = new DividerItemDecoration(getContext(), LinearLayoutManager.VERTICAL);
        itemDecoration.setDrawable(getResources().getDrawable(R.drawable.libpdf_divider));
        mRvOutline.addItemDecoration(itemDecoration);
        mAdapter = new OutlineAdapter();
        mRvOutline.setAdapter(mAdapter);
        mLoadingStatus = contentView.findViewById(R.id.loading_status);
        mLoadingView = contentView.findViewById(R.id.loading_view);
        mTvMessage = contentView.findViewById(R.id.tv_message);
        mBtnRetry = contentView.findViewById(R.id.btn_retry);
        mBtnRetry.setOnClickListener(mOnClickListener);
    }

    private OnClickListener mOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {

        }
    };

    public void setup(List<OutlineItem> outlineItemList) {
        mLoadingView.setVisibility(View.GONE);
        mTvMessage.setVisibility(View.GONE);
        mBtnRetry.setVisibility(View.GONE);
        mLoadingView.setVisibility(View.GONE);
        mAdapter.setOutline(outlineItemList);
    }

    public void loading() {
        mLoadingView.setVisibility(View.VISIBLE);
        mTvMessage.setVisibility(View.GONE);
        mBtnRetry.setVisibility(View.GONE);
        mLoadingView.setVisibility(View.VISIBLE);
    }

    public void error(String message) {
        mLoadingView.setVisibility(View.GONE);
        mTvMessage.setText(message);
        mTvMessage.setVisibility(View.VISIBLE);
        mBtnRetry.setVisibility(View.VISIBLE);
        mLoadingView.setVisibility(View.VISIBLE);
    }

    public interface OutlineListener {
        void toPage(int pageNumber);
    }

    public void setOutlineListener(OutlineListener outlineListener) {
        this.outlineListener = outlineListener;
    }

    private class OutlineAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

        private List<OutlineItem> mOutlineItemList;

        @NonNull
        @Override
        public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            return new OutlineViewHolder(LayoutInflater.from(parent.getContext()).inflate(R.layout.libpdf_item_outline, parent, false));
        }

        @Override
        public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
            OutlineViewHolder viewHolder = (OutlineViewHolder) holder;
            OutlineItem item = mOutlineItemList.get(position);
            viewHolder.mTvTitle.setText(item.getTitle());
            viewHolder.mTvPageNumber.setText((item.getPageNumber() + 1) + "");
            viewHolder.itemView.setOnClickListener(v -> {
                if (outlineListener != null) {
                    outlineListener.toPage(item.getPageNumber());
                }
            });
        }

        @Override
        public int getItemCount() {
            return mOutlineItemList == null ? 0 : mOutlineItemList.size();
        }

        public void setOutline(List<OutlineItem> outline) {
            if (mOutlineItemList != null) {
                mOutlineItemList.clear();
            }
            mOutlineItemList = outline;
            notifyDataSetChanged();
        }

    }

    static class OutlineViewHolder extends RecyclerView.ViewHolder {

        TextView mTvTitle;
        TextView mTvPageNumber;

        public OutlineViewHolder(View itemView) {
            super(itemView);
            mTvTitle = itemView.findViewById(R.id.tv_title);
            mTvPageNumber = itemView.findViewById(R.id.tv_page_number);
        }
    }

}
