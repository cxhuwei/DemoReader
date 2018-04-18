package com.chaoxing.reader.demo;

import android.support.annotation.NonNull;
import android.support.v7.widget.CardView;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import java.io.File;

/**
 * Created by HUWEI on 2018/4/18.
 */
public class FileAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

    private File[] mFiles;
    private OnItemClickListener onItemClickListener;

    @NonNull
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        return new FileViewHolder(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_file, parent, false));
    }

    @Override
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
        FileViewHolder viewHolder = (FileViewHolder) holder;
        final File file = mFiles[position];
        if (file.isDirectory()) {
            viewHolder.mTvFile.setCompoundDrawablesWithIntrinsicBounds(R.mipmap.ic_folder_black_36dp, 0, 0, 0);
        } else {
            viewHolder.mTvFile.setCompoundDrawablesWithIntrinsicBounds(R.mipmap.ic_cloud_black_36dp, 0, 0, 0);
        }
        viewHolder.mTvFile.setText(file.getName());
        viewHolder.mCardView.setOnClickListener(v -> {
            if (onItemClickListener != null) {
                onItemClickListener.onItemClick(file);
            }
        });
    }

    @Override
    public int getItemCount() {
        return mFiles == null ? 0 : mFiles.length;
    }

    public void setFiles(File[] files) {
        mFiles = files;
    }

    static class FileViewHolder extends RecyclerView.ViewHolder {

        CardView mCardView;
        TextView mTvFile;

        public FileViewHolder(View itemView) {
            super(itemView);
            mCardView = itemView.findViewById(R.id.card_view);
            mTvFile = itemView.findViewById(R.id.tv_file);
        }

    }

    public interface OnItemClickListener {
        void onItemClick(File file);
    }

    public void setOnItemClickListener(OnItemClickListener onItemClickListener) {
        this.onItemClickListener = onItemClickListener;
    }

}
