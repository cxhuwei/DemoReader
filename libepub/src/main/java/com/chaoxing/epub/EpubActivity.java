package com.chaoxing.epub;

import android.animation.Animator;
import android.animation.ObjectAnimator;
import android.arch.lifecycle.Observer;
import android.arch.lifecycle.ViewModelProviders;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.Nullable;
import android.support.v4.view.GravityCompat;
import android.support.v4.view.ViewPager;
import android.support.v4.widget.DrawerLayout;
import android.support.v4.widget.ViewDragHelper;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.PagerSnapHelper;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.ImageButton;
import android.widget.PopupMenu;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ViewFlipper;

import com.chaoxing.epub.nativeapi.EpubDocument;
import com.chaoxing.epub.nativeapi.OnEventListener;
import com.chaoxing.epub.util.EpubUtils;
import com.chaoxing.epub.util.UriUtils;

import java.io.File;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by HUWEI on 2018/4/18.
 */
public class EpubActivity extends AppCompatActivity {

    public static final String TAG = "EPUB_READER";

    private EventHandler mEventHandler;
    private EpubViewModel mViewModel;

    private DrawerLayout mDrawerLayout;
    private View mToolbar;
    private ImageButton mIbLeft;
    private TextView mTvTitle;
    private ImageButton mIbRight;
    private View mBottomBar;
    private ViewFlipper mVfBottomPanel;
    private ImageButton mIbNavigation;
    private ImageButton mIbForward;
    private ImageButton mIbReadingStyle;
    private ImageButton mIbTextStyle;
    private View mNavigationView;
    private View mTabCatalog;
    private TextView mTvCatalog;
    private View mTabBookmark;
    private TextView mTvBookmark;
    private ViewPager mVpNavigation;
    private CatalogView mCatalogView;
    private BookmarkView mBookmarkView;
    private View mLoadingView;
    private TextView mTvMessage;

    private View mPageContainer;
    private RecyclerView mDocumentPager;
    private EpubPagerAdapter mPagerAdapter;
    private View mEmptyCover;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        EpubUtils.setSystemUIVisible(this, false);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.libepub_activity_epub);
        mEventHandler = new EventHandler(this);
        mViewModel = ViewModelProviders.of(this).get(EpubViewModel.class);
        initView();
        initDocument();
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
    }

    private void initDocument() {
        List<Resource<EpubPage>> pageList = new ArrayList<>();
        for (int i = 0; i < 60; i++) {
            pageList.add(Resource.success(new EpubPage()));
        }
        mPagerAdapter.setPageList(pageList);

        Uri uri = getIntent().getData();
        String mimetype = getIntent().getType();
        String uriStr = uri.toString();

        final String path = UriUtils.getRealPath(this, uri);

        if (path == null) {
            finish();
            return;
        }

        if (!closeDocument()) {
            return;
        }

        EpubDocument.get().nativeSetBackgroundColor(Color.WHITE);

        mViewModel.getInitDocumentResult().observe(this, mInitDocumentObserver);
        mViewModel.getOpenDocumentResult().observe(this, mOpenDocumentObserver);
        mViewModel.getLoadFileCountResult().observe(this, mLoadFileCountObserver);
        mViewModel.getLoadPageCountByFileResult().observe(this, mLoadPageCountByFileObserver);
        mViewModel.initDocument(path);
    }

    private void openDocument() {
        EpubDocument.get().setOnEventListener(mOnEventListener);
        DocumentBinding binding = mViewModel.getDocumentBinding();
        binding.setDensity(getResources().getDisplayMetrics().density);
        binding.setWidth(mDocumentPager.getWidth());
        binding.setHeight(mDocumentPager.getHeight());
        mViewModel.openDocument();

//        DocumentBinding binding = mViewModel.getDocumentBinding();
//        try {
//            String fontPath = getFontPath(this);
//            int setFontResource = -1;
//            if (fontPath != null) {
//                setFontResource = EpubDocument.get().nativeSetFontResource(new String[]{fontPath, fontPath});
//            }
//            int layout = EpubDocument.get().nativeLayout(binding.getWidth(), binding.getHeight(), 0, 0, binding.getWidth(), binding.getHeight(), binding.getDensity());
//            int background = EpubDocument.get().nativeSetBackgroundColor(Color.WHITE);
//            int textLevel = EpubDocument.get().nativeSetTextLevel(0);
//            EpubInfo info = null;
//            if (setFontResource == 0 && layout == 0 && background == 0 && textLevel == 0) {
//                info = EpubDocument.get().nativeOpenDocument(binding.getPath());
//            }
//            if (info != null) {
//                Log.i(EpubActivity.TAG, info.toString());
//                loadFileCount();
//            } else {
//                Log.i(EpubActivity.TAG, "打开失败");
//            }
//        } catch (Throwable e) {
//            e.printStackTrace();
//        }
    }

    private String getFontPath(Context context) throws Throwable {
        String fontPath = null;
        File fontDir = new File(context.getExternalCacheDir(), File.separator + "epub" + File.separator + "font");
        if (!fontDir.exists()) {
            fontDir.mkdirs();
        }
        File fontFile = new File(fontDir, "fzlthk.ttf");
        String assetsFontFile = "font" + File.separator + "fzlthk.ttf";
        if (fontFile.exists()) {
            String fontMd5 = EpubUtils.md5(fontFile);
            String assetsFontMd5 = EpubUtils.md5(context.getAssets().open(assetsFontFile));
            if (!EpubUtils.equals(fontMd5, assetsFontMd5)) {
                if (fontFile.delete()) {
                    EpubUtils.assetsCopy(context, assetsFontFile, fontFile.getAbsolutePath());
                    fontPath = fontFile.getAbsolutePath();
                }
            } else {
                fontPath = fontFile.getAbsolutePath();
            }
        } else {
            EpubUtils.assetsCopy(context, assetsFontFile, fontFile.getAbsolutePath());
            fontPath = fontFile.getAbsolutePath();
        }
        return fontPath;
    }

    private void loadFileCount() {
        mViewModel.loadFileCount();
    }

    private void loadPageCountByFile(int fileId) {
        mViewModel.loadPageCountByFile(fileId);
    }

    private boolean closeDocument() {
        try {
            EpubDocument.get().closeDocument();
            return true;
        } catch (Throwable e) {
            e.printStackTrace();
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_SHORT).show();
        }
        return false;
    }

    protected void initView() {
        // drawerLayout
        mDrawerLayout = findViewById(R.id.drawer_layout);
        mDrawerLayout.addDrawerListener(new DrawerLayout.SimpleDrawerListener() {

            @Override
            public void onDrawerOpened(View drawerView) {
                int position = mVpNavigation.getCurrentItem();
                if (position < mVpNavigation.getAdapter().getCount() - 1) {
                    mDrawerLayout.setDrawerLockMode(DrawerLayout.LOCK_MODE_LOCKED_OPEN);
                } else {
                    mDrawerLayout.setDrawerLockMode(DrawerLayout.LOCK_MODE_UNLOCKED);
                }
            }

            @Override
            public void onDrawerClosed(View drawerView) {
                mDrawerLayout.setDrawerLockMode(DrawerLayout.LOCK_MODE_UNLOCKED);
            }

            @Override
            public void onDrawerStateChanged(int newState) {
                if (newState == ViewDragHelper.STATE_DRAGGING || newState == ViewDragHelper.STATE_SETTLING) {
                    hideBar();
                }
            }
        });
        mDrawerLayout.setOnTouchListener((v, event) -> {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    mDrawerLayout.closeDrawers();
                    break;
                default:
                    break;
            }
            return false;
        });

        // toolbar
        mToolbar = findViewById(R.id.toolbar);
        mIbLeft = mToolbar.findViewById(R.id.ib_left);
        mIbLeft.setOnClickListener(mOnClickListener);
        mTvTitle = mToolbar.findViewById(R.id.tv_title);
        mIbRight = mToolbar.findViewById(R.id.ib_right);
        mIbRight.setOnClickListener(mOnClickListener);

        // bottomBar
        mBottomBar = findViewById(R.id.bottom_bar);
        mVfBottomPanel = mBottomBar.findViewById(R.id.vf_bottom_panel);
        mIbNavigation = mBottomBar.findViewById(R.id.ib_navigation);
        mIbNavigation.setOnClickListener(mOnClickListener);
        mIbForward = mBottomBar.findViewById(R.id.ib_forward);
        mIbForward.setOnClickListener(mOnClickListener);
        mIbReadingStyle = mBottomBar.findViewById(R.id.ib_reading_style);
        mIbReadingStyle.setOnClickListener(mOnClickListener);
        mIbTextStyle = mBottomBar.findViewById(R.id.ib_text_style);
        mIbTextStyle.setOnClickListener(mOnClickListener);

        // navigation
        mNavigationView = findViewById(R.id.navigation_view);
        mTabCatalog = mNavigationView.findViewById(R.id.tab_catalog);
        mTabCatalog.setOnClickListener(mOnClickListener);
        mTabBookmark = mNavigationView.findViewById(R.id.tab_bookmark);
        mTabBookmark.setOnClickListener(mOnClickListener);
        mTvCatalog = mNavigationView.findViewById(R.id.tv_catalog);
        mTvBookmark = mNavigationView.findViewById(R.id.tv_bookmark);
        mVpNavigation = mNavigationView.findViewById(R.id.vp_navigation);
        mCatalogView = new CatalogView(this);
        mBookmarkView = new BookmarkView(this);
        List<View> navigationViewList = new ArrayList<>();
        navigationViewList.add(mCatalogView);
        navigationViewList.add(mBookmarkView);
        mVpNavigation.setOffscreenPageLimit(2);
        mVpNavigation.setAdapter(new NavigationAdapter(navigationViewList));
        mVpNavigation.addOnPageChangeListener(mOnNavigationPageChangeListener);
        mVpNavigation.setCurrentItem(0);

        // document
        mPageContainer = findViewById(R.id.page_container);
        mDocumentPager = findViewById(R.id.document_pager);

        mDocumentPager.setOnClickListener(mOnClickListener);
        mDocumentPager.setLayoutManager(new LinearLayoutManager(this, LinearLayoutManager.HORIZONTAL, false));
        mDocumentPager.setHasFixedSize(true);
        PagerSnapHelper snapHelper = new PagerSnapHelper();
        snapHelper.attachToRecyclerView(mDocumentPager);
        mDocumentPager.setItemViewCacheSize(1);
        mPagerAdapter = new EpubPagerAdapter(this);
        mPagerAdapter.setPageListener(mPageListener);
        mDocumentPager.setAdapter(mPagerAdapter);
        mDocumentPager.addOnScrollListener(mOnPagerScrollListener);

        mEmptyCover = findViewById(R.id.empty_cover);
        mEmptyCover.setVisibility(View.GONE);
        mEmptyCover.setOnClickListener(mOnClickListener);

        // loading
        mLoadingView = findViewById(R.id.pb_loading);
        mLoadingView.setVisibility(View.GONE);
        mTvMessage = findViewById(R.id.tv_message);
        mTvMessage.setVisibility(View.GONE);

        // window
        if (EpubUtils.isSupportLayoutFullscreen()) {
            mToolbar.setPadding(mToolbar.getPaddingLeft(), mToolbar.getPaddingTop() + EpubUtils.getStatusBarHeight(this), mToolbar.getPaddingRight(), mToolbar.getPaddingBottom());
            if (EpubUtils.checkDeviceHasNavigationBar(this)) {
                mBottomBar.setPadding(mBottomBar.getPaddingLeft(), mBottomBar.getPaddingTop(), mBottomBar.getPaddingRight(), mBottomBar.getPaddingBottom() + EpubUtils.getNavigationBarHeight(this));
            }
        }
        mToolbar.setVisibility(View.GONE);
        mBottomBar.setVisibility(View.GONE);
        getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(mOnSystemUiVisibilityChangeListener);
    }

    private void notifyPageCountChanged() {
        List<EpubPage> pageList = new ArrayList<>();
        DocumentBinding documentBinding = mViewModel.getDocumentBinding();
        if (mPagerAdapter.getItemCount() > 0) {
            LinearLayoutManager layoutManager = (LinearLayoutManager) mDocumentPager.getLayoutManager();
            int position = layoutManager.findFirstVisibleItemPosition();
            EpubPage currentPage = mPagerAdapter.getItemAtPosition(position).getData();
            List<EpubPage> pages = documentBinding.getPagesByFileId(currentPage.getFileId());
            if (pages == null) {
                if (currentPage.getFileId() > 0) {

                }
            } else {

            }
        }
        checkEmptyAdapter();
    }

    private void checkEmptyAdapter() {
        if (mPagerAdapter == null || mPagerAdapter.getItemCount() == 0) {
            mEmptyCover.setVisibility(View.VISIBLE);
        } else {
            mEmptyCover.setVisibility(View.GONE);
        }
    }

    private View.OnSystemUiVisibilityChangeListener mOnSystemUiVisibilityChangeListener = new View.OnSystemUiVisibilityChangeListener() {
        @Override
        public void onSystemUiVisibilityChange(int visibility) {
            mToolbar.setVisibility(visibility);
            mBottomBar.setVisibility(visibility);
        }
    };

    private ViewPager.OnPageChangeListener mOnNavigationPageChangeListener = new ViewPager.SimpleOnPageChangeListener() {
        @Override
        public void onPageSelected(int position) {
            if (position < mVpNavigation.getAdapter().getCount() - 1) {
                mDrawerLayout.setDrawerLockMode(DrawerLayout.LOCK_MODE_LOCKED_OPEN);
            } else {
                mDrawerLayout.setDrawerLockMode(DrawerLayout.LOCK_MODE_UNLOCKED);
            }
            if (position == 0) {
                mTvCatalog.setTextColor(0xFF0099FF);
                mTvCatalog.setBackgroundResource(R.drawable.libepub_navigation_tab_selected);
                mTvBookmark.setTextColor(0xFF333333);
                mTvBookmark.setBackgroundColor(Color.WHITE);
            } else if (position == 1) {
                mTvCatalog.setTextColor(0xFF333333);
                mTvCatalog.setBackgroundColor(Color.WHITE);
                mTvBookmark.setTextColor(0xFF0099FF);
                mTvBookmark.setBackgroundResource(R.drawable.libepub_navigation_tab_selected);
            }
        }
    };

    private View.OnClickListener mOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            int id = v.getId();
            if (id == R.id.ib_left) {
                onBackPressed();
            } else if (id == R.id.ib_right) {
                popupMenu();
            } else if (id == R.id.empty_cover) {
                switchBar();
            } else if (id == R.id.ib_navigation) {
                if (mDrawerLayout.isDrawerOpen(GravityCompat.START)) {
                    mDrawerLayout.closeDrawer(GravityCompat.START);
                } else {
                    mDrawerLayout.openDrawer(GravityCompat.START);
                }
            } else if (id == R.id.ib_forward) {
                mVfBottomPanel.setDisplayedChild(0);
            } else if (id == R.id.ib_reading_style) {
                mVfBottomPanel.setDisplayedChild(1);
            } else if (id == R.id.ib_text_style) {
                mVfBottomPanel.setDisplayedChild(2);
            } else if (id == R.id.tab_catalog) {
                mVpNavigation.setCurrentItem(0, true);
            } else if (id == R.id.tab_bookmark) {
                mVpNavigation.setCurrentItem(1, true);
            }
        }
    };

    private void popupMenu() {
        PopupMenu popupMenu = new PopupMenu(this, mIbRight);
        popupMenu.getMenuInflater().inflate(R.menu.libepub_epub, popupMenu.getMenu());
        popupMenu.show();
    }

    private void switchBar() {
        if (mToolbar.getVisibility() == View.VISIBLE || mBottomBar.getVisibility() == View.VISIBLE) {
            hideBar();
        } else {
            showBar();
        }
    }

    private ObjectAnimator mShowToolbarAnimator;
    private Animator.AnimatorListener mShowToolbarAnimatorListener = new SimpleAnimatorListener() {
        @Override
        public void onAnimationStart(Animator animation) {
            getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(null);
            EpubUtils.setSystemUIVisible(EpubActivity.this, true);
            mToolbar.setVisibility(View.VISIBLE);
            if (mHideToolbarAnimator != null && mHideToolbarAnimator.isRunning()) {
                mHideToolbarAnimator.cancel();
            }
        }

        @Override
        public void onAnimationEnd(Animator animation) {
            mToolbar.setTranslationY(0);
            mToolbar.setVisibility(View.VISIBLE);
            getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(mOnSystemUiVisibilityChangeListener);
        }
    };
    private ObjectAnimator mHideToolbarAnimator;
    private Animator.AnimatorListener mHideToolbarAnimatorListener = new SimpleAnimatorListener() {
        @Override
        public void onAnimationStart(Animator animation) {
            getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(null);
            EpubUtils.setSystemUIVisible(EpubActivity.this, false);
            if (mShowToolbarAnimator != null && mShowToolbarAnimator.isRunning()) {
                mShowToolbarAnimator.cancel();
            }
        }

        @Override
        public void onAnimationEnd(Animator animation) {
            mToolbar.setVisibility(View.GONE);
            mToolbar.setTranslationY(0);
            getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(mOnSystemUiVisibilityChangeListener);
        }
    };

    private ObjectAnimator mShowBottomBarAnimator;
    private Animator.AnimatorListener mShowBottomBarAnimatorListener = new SimpleAnimatorListener() {
        @Override
        public void onAnimationStart(Animator animation) {
            getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(null);
            EpubUtils.setSystemUIVisible(EpubActivity.this, true);
            mBottomBar.setVisibility(View.VISIBLE);
            if (mHideBottomBarAnimator != null && mHideBottomBarAnimator.isRunning()) {
                mHideBottomBarAnimator.cancel();
            }
        }

        @Override
        public void onAnimationEnd(Animator animation) {
            mBottomBar.setTranslationY(0);
            mBottomBar.setVisibility(View.VISIBLE);
            getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(mOnSystemUiVisibilityChangeListener);
        }
    };
    private ObjectAnimator mHideBottomBarAnimator;
    private Animator.AnimatorListener mHideBottomBarAnimatorListener = new SimpleAnimatorListener() {
        @Override
        public void onAnimationStart(Animator animation) {
            getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(null);
            EpubUtils.setSystemUIVisible(EpubActivity.this, false);
            if (mShowBottomBarAnimator != null && mShowBottomBarAnimator.isRunning()) {
                mShowBottomBarAnimator.cancel();
            }
        }

        @Override
        public void onAnimationEnd(Animator animation) {
            mBottomBar.setVisibility(View.GONE);
            mBottomBar.setTranslationY(0);
            getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(mOnSystemUiVisibilityChangeListener);
        }
    };

    private void showBar() {
        if (mHideToolbarAnimator != null) {
            mHideToolbarAnimator.cancel();
        }
        if (mHideBottomBarAnimator != null) {
            mHideBottomBarAnimator.cancel();
        }

        if (mToolbar.getVisibility() != View.VISIBLE) {
            if (mShowToolbarAnimator == null) {
                int distance = mToolbar.getTop() + mToolbar.getHeight();
                mShowToolbarAnimator = ObjectAnimator.ofFloat(mToolbar, "translationY", -distance, 0);
                mShowToolbarAnimator.setDuration(getResources().getInteger(android.R.integer.config_shortAnimTime));
                mShowToolbarAnimator.addListener(mShowToolbarAnimatorListener);
            }
            if (!mShowToolbarAnimator.isRunning()) {
                mShowToolbarAnimator.start();
            }
        }

        if (mBottomBar.getVisibility() != View.VISIBLE) {
            if (mShowBottomBarAnimator == null) {
                int distance = ((ViewGroup) mBottomBar.getParent()).getHeight() - mBottomBar.getTop();
                mShowBottomBarAnimator = ObjectAnimator.ofFloat(mBottomBar, "translationY", distance, 0);
                mShowBottomBarAnimator.setDuration(getResources().getInteger(android.R.integer.config_shortAnimTime));
                mShowBottomBarAnimator.addListener(mShowBottomBarAnimatorListener);
            }
            if (!mShowBottomBarAnimator.isRunning()) {
                mShowBottomBarAnimator.start();
            }
        }
    }

    private void hideBar() {
        if (mShowToolbarAnimator != null) {
            mShowToolbarAnimator.cancel();
        }
        if (mShowBottomBarAnimator != null) {
            mShowBottomBarAnimator.cancel();
        }

        if (mToolbar.getVisibility() == View.VISIBLE) {
            if (mHideToolbarAnimator == null) {
                mHideToolbarAnimator = ObjectAnimator.ofFloat(mToolbar, "translationY", 0, -mToolbar.getBottom());
                mHideToolbarAnimator.setDuration(getResources().getInteger(android.R.integer.config_shortAnimTime));
                mHideToolbarAnimator.addListener(mHideToolbarAnimatorListener);
            }
            if (!mHideToolbarAnimator.isRunning()) {
                mHideToolbarAnimator.start();
            }
        }

        if (mBottomBar.getVisibility() == View.VISIBLE) {
            if (mHideBottomBarAnimator == null) {
                int distance = ((ViewGroup) mBottomBar.getParent()).getHeight() - mBottomBar.getTop();
                mHideBottomBarAnimator = ObjectAnimator.ofFloat(mBottomBar, "translationY", 0, distance);
                mHideBottomBarAnimator.setDuration(getResources().getInteger(android.R.integer.config_shortAnimTime));
                mHideBottomBarAnimator.addListener(mHideBottomBarAnimatorListener);
            }
            if (!mHideBottomBarAnimator.isRunning()) {
                mHideBottomBarAnimator.start();
            }
        }
    }

    private RecyclerView.OnScrollListener mOnPagerScrollListener = new RecyclerView.OnScrollListener() {

        @Override
        public void onScrollStateChanged(RecyclerView recyclerView, int newState) {
            super.onScrollStateChanged(recyclerView, newState);
            if (newState == RecyclerView.SCROLL_STATE_DRAGGING) {
                hideBar();
            }
        }

    };

    private PageListener mPageListener = new PageListener() {

        @Override
        public void onClicked(MotionEvent e) {
            switchBar();
        }

        @Override
        public void onDoubleClicked(MotionEvent e) {

        }

        @Override
        public void onLongClicked(MotionEvent e) {

        }

        @Override
        public void loadPage(EpubPage page) {

        }

    };

    @Override
    protected void onResume() {
        super.onResume();
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        hideBar();
    }

    private Observer<Resource<DocumentBinding>> mInitDocumentObserver = new Observer<Resource<DocumentBinding>>() {
        @Override
        public void onChanged(@Nullable Resource<DocumentBinding> resource) {
            if (resource.isError()) {
                mLoadingView.setVisibility(View.GONE);
                mTvMessage.setText(resource.getMessage());
                mTvMessage.setVisibility(View.VISIBLE);
            } else if (resource.isLoading()) {
                mLoadingView.setVisibility(View.VISIBLE);
                mTvMessage.setVisibility(View.GONE);
            } else if (resource.isSuccessful()) {
                mLoadingView.setVisibility(View.GONE);
                openDocument();
            }
        }
    };

    private Observer<Resource<Void>> mOpenDocumentObserver = new Observer<Resource<Void>>() {

        @Override
        public void onChanged(@Nullable Resource<Void> resource) {
            if (resource.isError()) {
                Log.i(EpubActivity.TAG, "open document failed");
                mLoadingView.setVisibility(View.GONE);
                mTvMessage.setText(resource.getMessage());
                mTvMessage.setVisibility(View.VISIBLE);
            } else if (resource.isLoading()) {
                Log.i(EpubActivity.TAG, "open document");
                mLoadingView.setVisibility(View.VISIBLE);
                mTvMessage.setVisibility(View.GONE);
            } else if (resource.isSuccessful()) {
                Log.i(EpubActivity.TAG, "open document success");
                mLoadingView.setVisibility(View.GONE);
                notifyPageCountChanged();
                loadFileCount();
            }
        }
    };

    private Observer<Resource<Integer>> mLoadFileCountObserver = new Observer<Resource<Integer>>() {
        @Override
        public void onChanged(@Nullable Resource<Integer> resource) {
            if (resource.isError()) {
                Log.i(EpubActivity.TAG, "load file count failed");
                mLoadingView.setVisibility(View.GONE);
                mTvMessage.setText(resource.getMessage());
                mTvMessage.setVisibility(View.VISIBLE);
            } else if (resource.isLoading()) {
                Log.i(EpubActivity.TAG, "load file count");
                mLoadingView.setVisibility(View.VISIBLE);
                mTvMessage.setVisibility(View.GONE);
            } else if (resource.isSuccessful()) {
                mLoadingView.setVisibility(View.GONE);
                Log.i(EpubActivity.TAG, "load file count success. file count = " + resource.getData());
                mViewModel.getDocumentBinding().setFileCount(resource.getData());
                loadPageCountByFile(1);
            }
        }
    };

    private Observer<Resource<EpubFile>> mLoadPageCountByFileObserver = new Observer<Resource<EpubFile>>() {
        @Override
        public void onChanged(@Nullable Resource<EpubFile> resource) {
            if (resource.isError()) {
                Log.i(EpubActivity.TAG, "load page count by file failed. file id = " + resource.getData().getId());
                mLoadingView.setVisibility(View.GONE);
                mTvMessage.setText(resource.getMessage());
                mTvMessage.setVisibility(View.VISIBLE);
            } else if (resource.isLoading()) {
                Log.i(EpubActivity.TAG, "load page count by file. file id = " + resource.getData().getId());
                mLoadingView.setVisibility(View.VISIBLE);
                mTvMessage.setVisibility(View.GONE);
            } else if (resource.isSuccessful()) {
                Log.i(EpubActivity.TAG, "load page count by file success. file id = " + resource.getData().getId() + " page count = " + resource.getData().getPageCount());
                mLoadingView.setVisibility(View.GONE);
                EpubFile epubFile = resource.getData();
//                mViewModel.getDocumentBinding().getPageCounts().put(epubFile.getId(), epubFile.getPageCount());
//                notifyPageCountChanged();
            }
        }
    };

    private OnEventListener mOnEventListener = new OnEventListener() {
        @Override
        public void onEvent(int event, String message) {
            if (mEventHandler != null) {
                Message msg = Message.obtain();
                msg.what = event;
                msg.obj = message;
                mEventHandler.sendMessage(msg);
            }
        }
    };

    static class EventHandler extends Handler {
        WeakReference<EpubActivity> mReference;

        EventHandler(EpubActivity activity) {
            mReference = new WeakReference<>(activity);
        }

        @Override
        public void handleMessage(Message msg) {
            final EpubActivity activity = mReference.get();
            if (activity != null && !activity.isFinishing()) {
                activity.handleEvent(msg);
            }
        }
    }

    private void handleEvent(Message msg) {
        int event = msg.what;
        String message = (String) msg.obj;
        Log.i(EpubActivity.TAG, "onEvent() event = " + event + " message = " + message);
        Toast.makeText(this, "onEvent() event = " + event + " message = " + message, Toast.LENGTH_SHORT).show();
        if (event == EpubDocument.EVENT_STOP) {
        }
    }

    @Override
    public void finish() {
        EpubDocument.get().closeDocument();
        super.finish();
    }
}
