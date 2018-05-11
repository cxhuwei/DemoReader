package com.chaoxing.epub.nativeapi;

import android.graphics.Bitmap;
import android.graphics.Rect;

/**
 * Created by HUWEI on 2018/4/24.
 */
public class EpubDocument {

    static {
        System.loadLibrary("EpubReader9");
    }

    private String path;               // epub路径
    private EpubInfo epubInfo;         // epub元数据
    private Catalog[] catalogs;        // 目录
    private int fileCount;             // 文件数量

    private int width;                 // 页面宽度
    private int height;                // 页面高度
    private Rect bounds;               // 页面绘制区域
    private float density;
    private int foregroundColor;       // 前景色
    private int backgroundColor;       // 背景色
    private int textLevel;             // 文字大小级别

    public String getPath() {
        return path;
    }

    public EpubInfo getEpubInfo() {
        return epubInfo;
    }

    public Catalog[] getCatalogs() {
        return catalogs;
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public Rect getBounds() {
        return bounds;
    }

    public float getDensity() {
        return density;
    }

    public int getForegroundColor() {
        return foregroundColor;
    }

    public int getBackgroundColor() {
        return backgroundColor;
    }

    public int getTextLevel() {
        return textLevel;
    }

    private static EpubDocument sInstance;

    private EpubDocument() {
    }

    public static EpubDocument get() {
        if (sInstance == null) {
            synchronized (EpubDocument.class) {
                if (sInstance == null) {
                    sInstance = new EpubDocument();
                }
            }
        }
        return sInstance;
    }

    public int setForegroundColor(int color) {
        int result = nativeSetForegroundColor(color);
        this.foregroundColor = result;
        return result;
    }

    /**
     * 设置页面前景色（文字颜色）
     *
     * @param color argb色值（如：0xFF666666）
     * @return 解析器响应的色值
     */
    private native int nativeSetForegroundColor(int color);

    public int setBackgroundColor(int color) {
        int result = nativeSetBackgroundColor(color);
        this.backgroundColor = result;
        return result;
    }

    /**
     * 设置页面背景色
     *
     * @param color argb色值（如：0xFF666666）
     * @return 解析器响应的色值
     */
    private native int nativeSetBackgroundColor(int color);

    /**
     * 设置字体
     *
     * @param fontResource fontResource[0]:中文字体;fontResource[1]:英文字体
     * @return 1:设置成功
     */
    private native int nativeSetFontResource(String[] fontResource);

    public int setTextLevel(int level) {
        int result = nativeSetTextLevel(level);
        this.textLevel = result;
        return result;
    }

    /**
     * 设置文字大小级别（0：默认级别；-，+1调整级别）
     *
     * @param level 文字大小级别
     * @return 解析器响应的级别
     */
    private native int nativeSetTextLevel(int level);

    public boolean layout(int width, int height, Rect bounds, float density) {
        boolean result = nativeLayout(width, height, bounds.left, bounds.top, bounds.right, bounds.bottom, density);
        if (result) {
            this.width = width;
            this.height = height;
            this.bounds = new Rect(bounds.left, bounds.top, bounds.right, bounds.bottom);
            this.density = density;
        }
        return result;
    }

    /**
     * 页面宽高及绘制区域
     *
     * @param width   宽度
     * @param height  高度
     * @param right   左起始点
     * @param top     上起始点
     * @param left    右结束点
     * @param bottom  下结束点
     * @param density 设备像素密度
     * @return true or false
     */
    private native boolean nativeLayout(int width, int height, int left, int top, int right, int bottom, float density);

    /**
     * 打开epub
     *
     * @return epub元数据
     */
    public EpubInfo openDocument(String path) {
        this.path = path;
        this.epubInfo = nativeOpenDocument(path);
        return this.epubInfo;
    }

    /**
     * 打开epub
     *
     * @param path epub路径
     * @return epub元数据
     */
    private native EpubInfo nativeOpenDocument(String path);


    public Catalog[] getCatalog() {
        Catalog[] catalogs = nativeGetCatalog();
        this.catalogs = catalogs;
        return catalogs;
    }

    /**
     * 获取目录
     *
     * @return
     */
    private native Catalog[] nativeGetCatalog();

    public int getFileCount() {
        int result = nativeGetFileCount();
        this.fileCount = result;
        return result;
    }

    /**
     * 获取epub的总文件数
     *
     * @return
     */
    private native int nativeGetFileCount();

    /**
     * 获取文件的总页码
     *
     * @param fileId 文件id
     * @return
     */
    public native int nativeGetPageCountByFile(int fileId);

    /**
     * 定位目录
     *
     * @param index 目录index
     * @return int[0]:所在文件id，int[1]:文件内页码
     */
    public native int[] nativeLocateCatalog(int index);

    /**
     * 绘制页面
     *
     * @param fileId     文件id
     * @param pageNumber 文件内部页码
     * @param bitmap     页面位图
     * @return 1:绘制成功;
     */
    public native int nativeDrawPage(int fileId, int pageNumber, Bitmap bitmap);

    public void coloseDocument() {
        try {
            nativeColoseDocument();
        } catch (Throwable e) {
            e.printStackTrace();
        } finally {
            sInstance = null;
        }
    }

    /**
     * 销毁解析器
     */
    public native void nativeColoseDocument();

    /**
     * 解析器回调事件标记
     */
    public static final int EVENT_CALC_PAGE_START = 0;              // 开始计算页码
    public static final int EVENT_CALC_PAGE_COMPLETE = 1;           // 页码计算完成
    public static final int EVENT_CALC_FILE_PAGE_COMPLETE = 2;      // 当前文件页码计算完成
    public static final int EVENT_CALC_CATALOG_PAGE_COMPLETE = 3;   // 目录页码计算完成
    public static final int EVENT_STOP = 4;                         // 停止计算
    public static final int EVENT_PROGRESS = 5;                     // 计算进度

    /**
     * 解析器内部回调接口
     *
     * @param event   事件标记
     * @param message 事件信息
     */
    public void onEvent(int event, String message) {
        if (onEventListener != null) {
            onEventListener.onEvent(event, message);
        }
    }

    private OnEventListener onEventListener;

    public void setOnEventListener(OnEventListener onEventListener) {
        this.onEventListener = onEventListener;
    }

}
