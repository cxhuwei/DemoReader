package com.chaoxing.epub;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.GestureDetector;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import java.io.InputStream;
import java.util.List;

/**
 * Created by HUWEI on 2018/4/27.
 */
public class EpubPagerAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

    private static String TAG = EpubPagerAdapter.class.getSimpleName();

    private List<Resource<EpubPage>> mPageList;

    private PageListener mPageListener;
    private GestureDetector mGestureDetector;

    public EpubPagerAdapter(Context context) {
        mGestureDetector = new GestureDetector(context, mOnGestureListener);
    }

    @NonNull
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        return new PageViewHolder(LayoutInflater.from(parent.getContext()).inflate(R.layout.libepub_item_page, parent, false));
    }

    @Override
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
        PageViewHolder viewHolder = (PageViewHolder) holder;
        EpubPage page = mPageList.get(position).getData();
        if (page.getBitmap() == null || page.getBitmap().isRecycled()) {
            InputStream is = holder.itemView.getResources().openRawResource(R.raw.page_sample);
            page.setBitmap(BitmapFactory.decodeStream(is));
        }
        if (page.getBitmap() != null && !page.getBitmap().isRecycled()) {
            viewHolder.mPageView.setImageBitmap(page.getBitmap());
        }
        viewHolder.mPageView.setOnTouchListener((View v, MotionEvent event) -> {
            if (mGestureDetector != null) {
                return mGestureDetector.onTouchEvent(event);
            }
            return false;
        });
    }

    @Override
    public int getItemCount() {
        return mPageList == null ? 0 : mPageList.size();
    }

    @Override
    public void onViewRecycled(@NonNull RecyclerView.ViewHolder holder) {
        super.onViewRecycled(holder);
        Log.i(TAG, "onPageRecycled : " + (holder.getAdapterPosition() + 1));
        EpubPage page = mPageList.get(holder.getAdapterPosition()).getData();
        Bitmap bitmap = page.getBitmap();
        page.setBitmap(null);
        if (bitmap != null && !bitmap.isRecycled()) {
            bitmap.recycle();
        }
    }

    public void setPageList(List<Resource<EpubPage>> pageList) {
        if (mPageList == null) {
            mPageList = pageList;
            notifyItemRangeRemoved(0, pageList.size());
        } else {
            mPageList = pageList;
            notifyDataSetChanged();
        }
    }

    static class PageViewHolder extends RecyclerView.ViewHolder {

        ImageView mPageView;
        View mLoadingStatus;
        ProgressBar mLoadingView;
        TextView mTvMessage;
        Button mBtnRetry;

        public PageViewHolder(View itemView) {
            super(itemView);
            mPageView = itemView.findViewById(R.id.page_view);
            mLoadingStatus = itemView.findViewById(R.id.loading_status);
            mLoadingView = itemView.findViewById(R.id.loading_view);
            mTvMessage = itemView.findViewById(R.id.tv_message);
            mBtnRetry = itemView.findViewById(R.id.btn_retry);
        }

    }

    public void setPageListener(PageListener pageListener) {
        this.mPageListener = pageListener;
    }

    private GestureDetector.OnGestureListener mOnGestureListener = new GestureDetector.SimpleOnGestureListener() {

        @Override
        public boolean onDown(MotionEvent e) {
            return true;
        }

        @Override
        public boolean onSingleTapConfirmed(MotionEvent e) {
            if (mPageListener != null) {
                mPageListener.onClicked(e);
                return true;
            }
            return false;
        }

        @Override
        public boolean onDoubleTap(MotionEvent e) {
            if (mPageListener != null) {
                mPageListener.onDoubleClicked(e);
                return true;
            }
            return false;
        }

        @Override
        public void onLongPress(MotionEvent e) {
            if (mPageListener != null) {
                mPageListener.onLongClicked(e);
            }
        }

    };

}
