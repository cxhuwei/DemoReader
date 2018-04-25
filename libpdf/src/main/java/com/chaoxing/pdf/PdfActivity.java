package com.chaoxing.pdf;

import android.animation.Animator;
import android.animation.ObjectAnimator;
import android.arch.lifecycle.Observer;
import android.arch.lifecycle.ViewModelProviders;
import android.content.DialogInterface;
import android.content.res.Configuration;
import android.graphics.Color;
import android.graphics.RectF;
import android.net.Uri;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.view.GravityCompat;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.AppCompatEditText;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.PagerSnapHelper;
import android.support.v7.widget.RecyclerView;
import android.text.method.PasswordTransformationMethod;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.chaoxing.pdf.util.PdfUtils;
import com.chaoxing.pdf.util.UriUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class PdfActivity  extends AppCompatActivity {

    private static final String TAG = PdfActivity.class.getSimpleName();

    private PDFViewModel mViewModel;

    private View mToolbar;
    private ImageButton mIbLeft;
    private TextView mTvTitle;
    private ImageButton mIbRight;
    private View mBottomBar;
    private TextView mTvPageNumber;
    private ImageButton mIbNavigation;
    private ImageButton mIbMark;
    private ImageButton mIbBrightness;
    private ImageButton mIbForward;
    private ImageButton mIbRotate;

    private RecyclerView mDocumentPager;
    private PageAdapter mPageAdapter;
    private AlertDialog mInputPasswordDialog;
    private ProgressBar mPbLoading;
    private TextView mTvMessage;


    private DrawerLayout mDrawerLayout;
    private TextView mTvOutline;
    private TextView mTvNote;
    private TextView mTvBookmark;
    private ViewPager mVpNavigation;
    private OutlineNavigationView mOutlineNavigationView;
    private NoteNavigationView mNoteNavigationView;
    private BookmarkNavigationView mBookmarkNavigationView;


    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        WindowManager.LayoutParams params = getWindow().getAttributes();
        params.flags |= WindowManager.LayoutParams.FLAG_FULLSCREEN;
        getWindow().setAttributes(params);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.libpdf_activity_pdf);

        mViewModel = ViewModelProviders.of(this).get(PDFViewModel.class);

        Uri uri = getIntent().getData();
        String mimetype = getIntent().getType();
        String uriStr = uri.toString();

        final String path = UriUtils.getRealPath(this, uri);

        if (path == null) {
            finish();
            return;
        }

        initView();
        initDocument();

        mViewModel.openDocument(path);
    }

    private void initView() {
        mToolbar = findViewById(R.id.toolbar);
        mIbLeft = findViewById(R.id.ib_left);
        mIbLeft.setOnClickListener(mOnClickListener);
        mTvTitle = findViewById(R.id.tv_title);
        mIbRight = findViewById(R.id.ib_right);
        mIbRight.setOnClickListener(mOnClickListener);
        mBottomBar = findViewById(R.id.bottom_bar);
        mTvPageNumber = findViewById(R.id.tv_page_number);
        mIbNavigation = findViewById(R.id.ib_navigation);
        mIbNavigation.setOnClickListener(mOnClickListener);
        mIbMark = findViewById(R.id.ib_mark);
        mIbMark.setOnClickListener(mOnClickListener);
        mIbBrightness = findViewById(R.id.ib_brightness);
        mIbBrightness.setOnClickListener(mOnClickListener);
        mIbForward = findViewById(R.id.ib_forward);
        mIbForward.setOnClickListener(mOnClickListener);
        mIbRotate = findViewById(R.id.ib_rotate);
        mIbRotate.setOnClickListener(mOnClickListener);

        mDocumentPager = findViewById(R.id.document_pager);
        setPagerLayoutManager();
        mDocumentPager.setHasFixedSize(true);
        PagerSnapHelper snapHelper = new PagerSnapHelper();
        snapHelper.attachToRecyclerView(mDocumentPager);
        mDocumentPager.addOnScrollListener(mOnPagerScrollListener);
        mPageAdapter = new PageAdapter();
        mPageAdapter.setPageListener(mPageListener);
        mDocumentPager.setItemViewCacheSize(1);
        mDocumentPager.setAdapter(mPageAdapter);
        mPbLoading = findViewById(R.id.pb_loading);
        mTvMessage = findViewById(R.id.tv_message);

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
        });
        mDrawerLayout.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        mDrawerLayout.closeDrawers();
                        break;
                }
                return false;
            }
        });
        mTvOutline = findViewById(R.id.tv_outline);
        mTvOutline.setOnClickListener(mOnClickListener);
        mTvNote = findViewById(R.id.tv_note);
        mTvNote.setOnClickListener(mOnClickListener);
        mTvBookmark = findViewById(R.id.tv_bookmark);
        mTvBookmark.setOnClickListener(mOnClickListener);
        mVpNavigation = findViewById(R.id.vp_navigation);
        mOutlineNavigationView = new OutlineNavigationView(this);
        mOutlineNavigationView.setOutlineListener(mOutlineListener);
        mNoteNavigationView = new NoteNavigationView(this);
        mBookmarkNavigationView = new BookmarkNavigationView(this);
        List<View> navigationViewList = new ArrayList<>();
        navigationViewList.add(mOutlineNavigationView);
        navigationViewList.add(mNoteNavigationView);
        navigationViewList.add(mBookmarkNavigationView);
        mVpNavigation.setAdapter(new NavigationAdapter(navigationViewList));
        mVpNavigation.addOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener() {
            @Override
            public void onPageSelected(int position) {
                if (position < mVpNavigation.getAdapter().getCount() - 1) {
                    mDrawerLayout.setDrawerLockMode(DrawerLayout.LOCK_MODE_LOCKED_OPEN);
                } else {
                    mDrawerLayout.setDrawerLockMode(DrawerLayout.LOCK_MODE_UNLOCKED);
                }

                if (position == 0) {
                    mTvOutline.setBackgroundColor(0xFF0099FF);
                    mTvOutline.setTextColor(Color.WHITE);
                    mTvNote.setBackgroundColor(Color.TRANSPARENT);
                    mTvNote.setTextColor(0xFF0099FF);
                    mTvBookmark.setBackgroundColor(Color.TRANSPARENT);
                    mTvBookmark.setTextColor(0xFF0099FF);
                } else if (position == 1) {
                    mTvOutline.setBackgroundColor(Color.TRANSPARENT);
                    mTvOutline.setTextColor(0xFF0099FF);
                    mTvNote.setBackgroundColor(0xFF0099FF);
                    mTvNote.setTextColor(Color.WHITE);
                    mTvBookmark.setBackgroundColor(Color.TRANSPARENT);
                    mTvBookmark.setTextColor(0xFF0099FF);
                } else if (position == 2) {
                    mTvOutline.setBackgroundColor(Color.TRANSPARENT);
                    mTvOutline.setTextColor(0xFF0099FF);
                    mTvNote.setBackgroundColor(Color.TRANSPARENT);
                    mTvNote.setTextColor(0xFF0099FF);
                    mTvBookmark.setBackgroundColor(0xFF0099FF);
                    mTvBookmark.setTextColor(Color.WHITE);
                }
            }
        });
    }

    private void setPagerLayoutManager() {
        if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
            mDocumentPager.setLayoutManager(new LinearLayoutManager(this, LinearLayoutManager.HORIZONTAL, false));
        } else {
            mDocumentPager.setLayoutManager(new LinearLayoutManager(this, LinearLayoutManager.VERTICAL, false));
        }
    }

    class NavigationAdapter extends PagerAdapter {

        private List<View> mNavigationViewList;

        NavigationAdapter(List<View> navigationViewList) {
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

    private View.OnClickListener mOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            int id = v.getId();
            if (id == R.id.ib_left) {
                onBackPressed();
            } else if (id == R.id.ib_right) {

            } else if (id == R.id.ib_navigation) {
                if (mDrawerLayout.isDrawerOpen(GravityCompat.START)) {
                    mDrawerLayout.closeDrawer(GravityCompat.START);
                } else {
                    mDrawerLayout.openDrawer(GravityCompat.START);
                    hideBar();
                }
            } else if (id == R.id.ib_mark) {

            } else if (id == R.id.ib_brightness) {

            } else if (id == R.id.ib_forward) {

            } else if (id == R.id.ib_rotate) {

            } else if (id == R.id.tv_outline) {
                mVpNavigation.setCurrentItem(0);
            } else if (id == R.id.tv_note) {
                mVpNavigation.setCurrentItem(1);
            } else if (id == R.id.tv_bookmark) {
                mVpNavigation.setCurrentItem(2);
            }
        }
    };

    private OutlineNavigationView.OutlineListener mOutlineListener = new OutlineNavigationView.OutlineListener() {
        @Override
        public void toPage(int pageNumber) {
            mDocumentPager.scrollToPosition(pageNumber);
            mDrawerLayout.closeDrawer(GravityCompat.START);
        }
    };

    private void initDocument() {
        mViewModel.getOpenDocumentResult().observe(this, mObserverOpenDocument);
        mViewModel.getCheckPasswordResult().observe(this, mObserverCheckPassword);
        mViewModel.getLoadDocumentResult().observe(this, mObserverLoadDocument);
        mViewModel.getLoadPageResult().observe(this, mObserverLoadPage);
        mViewModel.getLoadOutlineResult().observe(this, mObserverLoadOutline);
    }


    private Observer<Resource<PdfBinding>> mObserverOpenDocument = new Observer<Resource<PdfBinding>>() {
        @Override
        public void onChanged(@Nullable Resource<PdfBinding> pdfBinding) {
            Status status = pdfBinding.getStatus();
            if (status == Status.LOADING) {
                mTvMessage.setVisibility(View.GONE);
                mPbLoading.setVisibility(View.VISIBLE);
            } else if (status == Status.ERROR) {
                mPbLoading.setVisibility(View.GONE);
                mTvMessage.setText(pdfBinding.getMessage());
                mTvMessage.setVisibility(View.VISIBLE);
            } else if (status == Status.SUCCESS) {
                if (pdfBinding.getData().isNeedsPassword()) {
                    mPbLoading.setVisibility(View.GONE);
                    askPassword();
                } else {
                    loadDocument();
                }
            }
        }
    };

    private void askPassword() {
        if (mInputPasswordDialog == null) {
            final LinearLayout layout = new LinearLayout(this);
            layout.setOrientation(LinearLayout.VERTICAL);
            layout.setPadding(PdfUtils.dp2px(this, 24), PdfUtils.dp2px(this, 12), PdfUtils.dp2px(this, 24), PdfUtils.dp2px(this, 12));
            final AppCompatEditText etPassword = new AppCompatEditText(this);
            etPassword.setInputType(EditorInfo.TYPE_TEXT_VARIATION_PASSWORD);
            etPassword.setTransformationMethod(PasswordTransformationMethod.getInstance());
            layout.addView(etPassword);

            mInputPasswordDialog = new AlertDialog.Builder(this)
                    .setTitle("输入密码")
                    .setView(layout)
                    .setPositiveButton("确定", null)
                    .setNegativeButton("取消", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    })
                    .setCancelable(false)
                    .create();
            mInputPasswordDialog.show();
            mInputPasswordDialog.getButton(AlertDialog.BUTTON_POSITIVE).setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (etPassword.length() > 0) {
                        mInputPasswordDialog.getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(false);
                        mInputPasswordDialog.getButton(AlertDialog.BUTTON_NEGATIVE).setEnabled(false);
                        mViewModel.checkPassword(etPassword.getText().toString());
                    }
                }
            });
        } else {
            mInputPasswordDialog.show();
        }
    }

    private Observer<Resource<Boolean>> mObserverCheckPassword = new Observer<Resource<Boolean>>() {
        @Override
        public void onChanged(@Nullable Resource<Boolean> result) {
            mInputPasswordDialog.getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(true);
            mInputPasswordDialog.getButton(AlertDialog.BUTTON_NEGATIVE).setEnabled(true);
            if (result.getData()) {
                mInputPasswordDialog.dismiss();
                loadDocument();
            } else {
                Toast.makeText(PdfActivity.this, "密码错误", Toast.LENGTH_SHORT).show();
            }
        }
    };

    private void loadDocument() {
        mViewModel.loadDocument();
    }

    private Observer<Resource<PdfBinding>> mObserverLoadDocument = new Observer<Resource<PdfBinding>>() {
        @Override
        public void onChanged(@Nullable Resource<PdfBinding> pdfBinding) {
            if (pdfBinding.isLoading()) {
                mPbLoading.setVisibility(View.VISIBLE);
            } else if (pdfBinding.isSuccessful()) {
                mPbLoading.setVisibility(View.GONE);
                mTvTitle.setText(mViewModel.getPdfBinding().getTitle());
                int pageCount = pdfBinding.getData().getPageCount();
                List<Resource<PdfPage>> pageList = new ArrayList<>(Collections.nCopies(pageCount, Resource.idle(null)));
                mPageAdapter.setPageList(pageList);
                setPageNumberText();
                mViewModel.loadOutline(mViewModel.getPdfBinding());
            } else {
                mPbLoading.setVisibility(View.GONE);
                Toast.makeText(PdfActivity.this, pdfBinding.getMessage(), Toast.LENGTH_SHORT).show();
                finish();
            }
        }
    };

    private Observer<Resource<PdfPage>> mObserverLoadPage = new Observer<Resource<PdfPage>>() {
        @Override
        public void onChanged(@Nullable Resource<PdfPage> resource) {
            mPageAdapter.updatePage(resource);
        }
    };

    private Observer<Resource<PdfBinding>> mObserverLoadOutline = new Observer<Resource<PdfBinding>>() {
        @Override
        public void onChanged(@Nullable Resource<PdfBinding> result) {
            if (result.isLoading()) {
                mOutlineNavigationView.loading();
            } else if (result.isError()) {
                mOutlineNavigationView.error(result.getMessage());
            } else if (result.isSuccessful()) {
                mOutlineNavigationView.setup(result.getData().getOutlineItemList());
            }
        }
    };

    private RecyclerView.OnScrollListener mOnPagerScrollListener = new RecyclerView.OnScrollListener() {

        @Override
        public void onScrolled(RecyclerView recyclerView, int dx, int dy) {
            super.onScrolled(recyclerView, dx, dy);
            setPageNumberText();
        }

    };

    private PageAdapter.PageListener mPageListener = new PageAdapter.PageListener() {
        @Override
        public void loadPage(int pageNumber) {
            mViewModel.loadPage(pageNumber, mDocumentPager.getWidth());
        }

        @Override
        public void onPageClicked(View view, MotionEvent e) {
            RectF centerRect;
            RectF leftEdgeRect;
            RectF rightEdgeRect;
            if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
                centerRect = new RectF(view.getWidth() / 4, view.getHeight() / 7, view.getWidth() / 4 * 3, view.getHeight() / 7 * 6);
                leftEdgeRect = new RectF(0, view.getHeight() / 7, view.getWidth() / 4, view.getHeight() / 7 * 6);
                rightEdgeRect = new RectF(view.getWidth() / 4 * 3, view.getHeight() / 7, view.getWidth(), view.getHeight() / 7 * 6);
            } else {
                centerRect = new RectF(view.getWidth() / 7, view.getHeight() / 4, view.getWidth() / 7 * 6, view.getHeight() / 4 * 3);
                leftEdgeRect = new RectF(0, view.getHeight() / 7, view.getWidth() / 4, view.getHeight() / 7 * 6);
                rightEdgeRect = new RectF(view.getWidth() / 4 * 3, view.getHeight() / 7, view.getWidth(), view.getHeight() / 7 * 6);
            }
            if (centerRect.contains(e.getX(), e.getY())) {
                switchBar();
            } else if (leftEdgeRect.contains(e.getX(), e.getY())) {
                int currentPage = ((LinearLayoutManager) mDocumentPager.getLayoutManager()).findFirstCompletelyVisibleItemPosition();
                if (currentPage > 0) {
                    mDocumentPager.smoothScrollToPosition(currentPage - 1);
                    hideBar();
                }
            } else if (rightEdgeRect.contains(e.getX(), e.getY())) {
                int currentPage = ((LinearLayoutManager) mDocumentPager.getLayoutManager()).findFirstCompletelyVisibleItemPosition();
                if (currentPage < mViewModel.getPdfBinding().getPageCount() - 1) {
                    mDocumentPager.smoothScrollToPosition(currentPage + 1);
                    hideBar();
                }
            }
        }
    };

    private void setPageNumberText() {
        int currentPage = ((LinearLayoutManager) mDocumentPager.getLayoutManager()).findFirstVisibleItemPosition();
        if (currentPage >= 0) {
            mTvPageNumber.setText((currentPage + 1) + "/" + (mViewModel.getPdfBinding().getPageCount()));
        }
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
            mToolbar.setVisibility(View.VISIBLE);
            if (mHideToolbarAnimator != null && mHideToolbarAnimator.isRunning()) {
                mHideToolbarAnimator.cancel();
            }
        }

        @Override
        public void onAnimationEnd(Animator animation) {
            mToolbar.setTranslationY(0);
            mToolbar.setVisibility(View.VISIBLE);
        }
    };
    private ObjectAnimator mHideToolbarAnimator;
    private Animator.AnimatorListener mHideToolbarAnimatorListener = new SimpleAnimatorListener() {
        @Override
        public void onAnimationStart(Animator animation) {
            if (mShowToolbarAnimator != null && mShowToolbarAnimator.isRunning()) {
                mShowToolbarAnimator.cancel();
            }
        }

        @Override
        public void onAnimationEnd(Animator animation) {
            mToolbar.setVisibility(View.GONE);
            mToolbar.setTranslationY(0);
        }
    };

    private ObjectAnimator mShowBottomBarAnimator;
    private Animator.AnimatorListener mShowBottomBarAnimatorListener = new SimpleAnimatorListener() {
        @Override
        public void onAnimationStart(Animator animation) {
            mBottomBar.setVisibility(View.VISIBLE);
            if (mHideBottomBarAnimator != null && mHideBottomBarAnimator.isRunning()) {
                mHideBottomBarAnimator.cancel();
            }
        }

        @Override
        public void onAnimationEnd(Animator animation) {
            mBottomBar.setTranslationY(0);
            mBottomBar.setVisibility(View.VISIBLE);
        }
    };
    private ObjectAnimator mHideBottomBarAnimator;
    private Animator.AnimatorListener mHideBottomBarAnimatorListener = new SimpleAnimatorListener() {
        @Override
        public void onAnimationStart(Animator animation) {
            if (mShowBottomBarAnimator != null && mShowBottomBarAnimator.isRunning()) {
                mShowBottomBarAnimator.cancel();
            }
        }

        @Override
        public void onAnimationEnd(Animator animation) {
            mBottomBar.setVisibility(View.GONE);
            mBottomBar.setTranslationY(0);
        }
    };

    private void showBar() {
        if (mToolbar.getVisibility() == View.VISIBLE && mBottomBar.getVisibility() == View.VISIBLE) {
            return;
        }

        if ((mShowToolbarAnimator != null && mShowToolbarAnimator.isRunning()) && (mShowBottomBarAnimator != null && mShowBottomBarAnimator.isRunning())) {
            return;
        }

        if (mShowToolbarAnimator != null && mShowToolbarAnimator.isRunning()) {
            mShowToolbarAnimator.cancel();
        }
        int distance = mToolbar.getTop() + mToolbar.getHeight();
        mShowToolbarAnimator = ObjectAnimator.ofFloat(mToolbar, "translationY", -distance, 0);
        mShowToolbarAnimator.addListener(mShowToolbarAnimatorListener);
        mShowToolbarAnimator.start();

        if (mShowBottomBarAnimator != null && mShowBottomBarAnimator.isRunning()) {
            mShowBottomBarAnimator.cancel();
        }
        distance = ((ViewGroup) mBottomBar.getParent()).getHeight() - mBottomBar.getTop();
        mShowBottomBarAnimator = ObjectAnimator.ofFloat(mBottomBar, "translationY", distance, 0);
        mShowBottomBarAnimator.addListener(mShowBottomBarAnimatorListener);
        mShowBottomBarAnimator.start();
    }

    private void hideBar() {
        if (mToolbar.getVisibility() != View.VISIBLE && mBottomBar.getVisibility() != View.VISIBLE) {
            return;
        }

        if ((mHideToolbarAnimator != null && mHideToolbarAnimator.isRunning()) && (mHideBottomBarAnimator != null && mHideBottomBarAnimator.isRunning())) {
            return;
        }

        if (mHideToolbarAnimator != null && mHideToolbarAnimator.isRunning()) {
            mHideToolbarAnimator.cancel();
        }
        mHideToolbarAnimator = ObjectAnimator.ofFloat(mToolbar, "translationY", 0, -mToolbar.getBottom());
        mHideToolbarAnimator.addListener(mHideToolbarAnimatorListener);
        mHideToolbarAnimator.start();

        if (mHideBottomBarAnimator != null && mHideBottomBarAnimator.isRunning()) {
            mHideBottomBarAnimator.cancel();
        }
        int distance = ((ViewGroup) mBottomBar.getParent()).getHeight() - mBottomBar.getTop();
        mHideBottomBarAnimator = ObjectAnimator.ofFloat(mBottomBar, "translationY", 0, distance);
        mHideBottomBarAnimator.addListener(mHideBottomBarAnimatorListener);
        mHideBottomBarAnimator.start();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        int position = 0;
        if (mDocumentPager != null && mDocumentPager.getLayoutManager() != null) {
            position = ((LinearLayoutManager) mDocumentPager.getLayoutManager()).findFirstVisibleItemPosition();
        }
        if (position < 0) {
            position = 0;
        }
        setPagerLayoutManager();
        mDocumentPager.scrollToPosition(position);
    }


    @Override
    public void onBackPressed() {
        if (mDrawerLayout.isDrawerOpen(GravityCompat.START)) {
            mDrawerLayout.closeDrawer(GravityCompat.START);
        } else {
            super.onBackPressed();
        }
    }

    @Override
    public void finish() {
        if (mViewModel.getPdfBinding() != null && mViewModel.getPdfBinding().getDocument() != null) {
            mViewModel.getPdfBinding().getDocument().destroy();
        }
        super.finish();
    }

}
