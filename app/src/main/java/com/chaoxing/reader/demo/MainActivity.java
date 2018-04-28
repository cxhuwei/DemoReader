package com.chaoxing.reader.demo;

import android.Manifest;
import android.arch.lifecycle.ViewModelProviders;
import android.content.Intent;
import android.graphics.Color;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;
import android.widget.Toast;

import com.chaoxing.epub.EpubActivity;
import com.chaoxing.pdf.PdfActivity;
import com.chaoxing.pdf.util.PdfUtils;
import com.tbruyelle.rxpermissions2.RxPermissions;

import java.io.File;

/**
 * Created by HUWEI on 2018/4/18.
 */
public class MainActivity extends AppCompatActivity {

    private static final String TAG = MainActivity.class.getName();

    private MainViewModel mViewModel;
    private TextView mTvPath;
    private RecyclerView mFileList;
    private FileAdapter mAdapter;
    private File mRootDirectory;
    private File mCurrentDirectory;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            Window window = getWindow();
            window.getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            window.setStatusBarColor(getColor(R.color.colorPrimary));
        }

        Toolbar toolbar = findViewById(R.id.toolbar);
        toolbar.setNavigationIcon(R.mipmap.ic_storage_white_36dp);
        toolbar.setTitle("Files");
        setSupportActionBar(toolbar);

        mViewModel = ViewModelProviders.of(this).get(MainViewModel.class);
        initView();
        initDirectory();
        attemptOpenDirectory();
    }

    private void initView() {
        mTvPath = findViewById(R.id.tv_path);
        mFileList = findViewById(R.id.file_list);
        mFileList.setLayoutManager(new LinearLayoutManager(this, LinearLayoutManager.VERTICAL, false));
        mAdapter = new FileAdapter();
        mFileList.setAdapter(mAdapter);
        mAdapter.setOnItemClickListener(file -> {
            if (file.isDirectory()) {
                openDirectory(file);
            } else {
                openFile(file);
            }
        });
    }

    private void initDirectory() {
        mViewModel.getFiles().observe(this, files -> {
            mAdapter.setFiles(files);
            mAdapter.notifyDataSetChanged();
        });
    }

    private void attemptOpenDirectory() {
        new RxPermissions(this)
                .requestEachCombined(Manifest.permission.READ_EXTERNAL_STORAGE)
                .subscribe(permission -> {
                    if (permission.granted) {
                        openRootDirectory();
                    } else if (permission.shouldShowRequestPermissionRationale) {
                        attemptOpenDirectory();
                    } else {
                        Toast.makeText(MainActivity.this, "获取\"存储空间\"权限失败", Toast.LENGTH_SHORT).show();
                    }
                });
    }

    private void openRootDirectory() {
        mRootDirectory = Environment.getExternalStorageDirectory();
        File docDir = new File(mRootDirectory, "doc");
        if (docDir.exists() && docDir.isDirectory()) {
            openDirectory(docDir);
        } else {
            openDirectory(mRootDirectory);
        }
    }

    private void openDirectory(File directory) {
        if (directory != null && directory.isDirectory()) {
            mCurrentDirectory = directory;
            if (mCurrentDirectory.equals(mRootDirectory)) {
                mTvPath.setText("/");
            } else {
                mTvPath.setText(mCurrentDirectory.getAbsolutePath().substring(mRootDirectory.getAbsolutePath().length()));
            }
            mViewModel.openDirectory(directory);
        }
    }

    private void openFile(File file) {
        if (PdfUtils.getFileExtension(file).equalsIgnoreCase("pdf")) {
            Intent intent = new Intent(this, PdfActivity.class);
            intent.setData(Uri.fromFile(file));
            startActivity(intent);
        } else if (PdfUtils.getFileExtension(file).equalsIgnoreCase("epub")) {
            Intent intent = new Intent(this, EpubActivity.class);
            intent.setData(Uri.fromFile(file));
            startActivity(intent);
        }
    }

    @Override
    public void onBackPressed() {
        File parentFile = null;
        if (mCurrentDirectory != null) {
            parentFile = mCurrentDirectory.getParentFile();
        }

        if (parentFile == null || parentFile.getAbsolutePath().length() < mRootDirectory.getAbsolutePath().length()) {
            super.onBackPressed();
        } else {
            openDirectory(parentFile);
        }
    }

}
