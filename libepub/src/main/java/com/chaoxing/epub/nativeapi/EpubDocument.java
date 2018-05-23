package com.chaoxing.epub.nativeapi;

import android.graphics.Bitmap;
import android.os.Build;

/**
 * Created by HUWEI on 2018/4/24.
 */
public class EpubDocument {

    static {
        if (Build.VERSION.SDK_INT == Build.VERSION_CODES.JELLY_BEAN_MR1) {
            System.loadLibrary("iconv");
        }
        System.loadLibrary("epub");
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

    /**
     * 设置页面前景色（文字颜色）
     *
     * @param color argb色值（如：0xFF666666）
     * @return 0:成功 or 失败
     */
    public native int nativeSetForegroundColor(int color);

    /**
     * 设置页面背景色
     *
     * @param color argb色值（如：0xFF666666）
     * @return 0:成功 or 失败
     */
    public native int nativeSetBackgroundColor(int color);

    /**
     * 设置字体
     *
     * @param fontResource fontResource[0]:中文字体;fontResource[1]:英文字体
     * @return 0:成功 or 失败
     */
    public native int nativeSetFontResource(String[] fontResource);

    /**
     * 设置文字大小级别（0：默认级别；-，+1调整级别）
     *
     * @param level 文字大小级别
     * @return 0:成功 or 失败
     */
    public native int nativeSetTextLevel(int level);

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
    public native int nativeLayout(int width, int height, int left, int top, int right, int bottom, float density);


    /**
     * 打开epub
     *
     * @param path epub路径
     * @return epub元数据
     */
    public native EpubInfo nativeOpenDocument(String path);

    /**
     * 获取目录
     *
     * @return
     */
    public native Catalog[] nativeGetCatalog();


    /**
     * 获取epub的总文件数
     *
     * @return
     */
    public native int nativeGetFileCount();

    /**
     * 获取单个文件的页数
     *
     * @param fileId 文件id
     * @return 页数
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
     * @return 0:成功 or 失败
     */
    public native int nativeDrawPage(int fileId, int pageNumber, Bitmap bitmap);

    public void closeDocument() {
        nativeCloseDocument();
        sInstance = null;
    }

    /**
     * 销毁解析器
     */
    public native void nativeCloseDocument();

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
