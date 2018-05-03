package com.chaoxing.epub;

import android.animation.Animator;
import android.animation.ObjectAnimator;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.os.Bundle;
import android.support.v4.view.GravityCompat;
import android.support.v4.view.ViewPager;
import android.support.v4.widget.DrawerLayout;
import android.support.v4.widget.ViewDragHelper;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.PagerSnapHelper;
import android.support.v7.widget.RecyclerView;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.ImageButton;
import android.widget.PopupMenu;
import android.widget.TextView;
import android.widget.ViewFlipper;

import com.chaoxing.epub.util.EpubUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by HUWEI on 2018/4/18.
 */
public class EpubActivity extends AppCompatActivity {

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

    private RecyclerView mDocumentPager;
    private EpubPagerAdapter mPagerAdapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        EpubUtils.setSystemUIVisible(this, false);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.libepub_activity_epub);
        initView();

        List<Resource<EpubPage>> pageList = new ArrayList<>();
        for (int i = 0; i < 60; i++) {
            pageList.add(Resource.success(new EpubPage()));
        }
        mPagerAdapter.setPageList(pageList);
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
        mDocumentPager = findViewById(R.id.document_pager);
        mDocumentPager.setLayoutManager(new LinearLayoutManager(this, LinearLayoutManager.HORIZONTAL, false));
        mDocumentPager.setHasFixedSize(true);
        PagerSnapHelper snapHelper = new PagerSnapHelper();
        snapHelper.attachToRecyclerView(mDocumentPager);
        mDocumentPager.setItemViewCacheSize(1);
        mPagerAdapter = new EpubPagerAdapter(this);
        mPagerAdapter.setPageListener(mPageListener);
        mDocumentPager.setAdapter(mPagerAdapter);
        mDocumentPager.addOnScrollListener(mOnPagerScrollListener);

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
}
