package com.chaoxing.pdf;

import android.arch.lifecycle.LiveData;
import android.net.Uri;
import android.support.annotation.NonNull;
import android.text.TextUtils;
import android.util.Log;

import com.artifex.mupdf.fitz.Document;
import com.artifex.mupdf.fitz.Outline;
import com.chaoxing.pdf.util.PdfUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by HUWEI on 2018/4/18.
 */
public class PdfLoader {

    private static final String TAG = PdfLoader.class.getSimpleName();

    public LiveData<Resource<PdfBinding>> openDocument(String path) {
        ExecuteBoundResource<String, PdfBinding> execute = new ExecuteBoundResource<String, PdfBinding>() {
            @Override
            public Resource<PdfBinding> onExecute(String params) {

                Log.i(TAG, "open document : " + params);

                try {
                    if (PdfUtils.isBlank(params) || !new File(params).exists()) {
                        return Resource.error("文档不存在", null);
                    }

                    PdfBinding binding = new PdfBinding();
                    binding.setPath(params);
                    Document document = Document.openDocument(params);
                    binding.setDocument(document);
                    binding.setMd5(PdfUtils.md5(new File(params)));
                    binding.setNeedsPassword(document.needsPassword());

                    return Resource.success(binding);
                } catch (Throwable t) {
                    return Resource.error(t.getMessage(), null);
                }
            }
        };
        return execute.execute(path);
    }

    public LiveData<Resource<Boolean>> checkPassword(Document document, String password) {
        ExecuteBoundResource<Object[], Boolean> execute = new ExecuteBoundResource<Object[], Boolean>() {
            @NonNull
            @Override
            public Resource<Boolean> onExecute(Object params[]) {
                return Resource.success(((Document) params[0]).authenticatePassword((String) params[1]));
            }
        };
        return execute.execute(new Object[]{document, password});
    }

    public LiveData<Resource<PdfBinding>> loadDocument(PdfBinding pdfBinding, int width, int height) {
        ExecuteBoundResource<Object[], PdfBinding> execute = new ExecuteBoundResource<Object[], PdfBinding>() {
            @NonNull
            @Override
            protected Resource<PdfBinding> onExecute(Object[] params) {
                String message = null;
                try {
                    PdfBinding binding = (PdfBinding) params[0];
                    int width = (int) params[1];
                    int height = (int) params[2];

                    Log.i(TAG, "load document");

                    final String path = binding.getPath();
                    final Document document = binding.getDocument();
                    PdfBinding newPdfBinding = new PdfBinding();
                    newPdfBinding.setPath(path);
                    newPdfBinding.setDocument(document);
                    newPdfBinding.setMd5(binding.getMd5());
                    newPdfBinding.setNeedsPassword(binding.isNeedsPassword());
                    String title = document.getMetaData(Document.META_INFO_TITLE);
                    String format = document.getMetaData(Document.META_FORMAT);
                    if (TextUtils.isEmpty(title)) {
                        title = Uri.parse(path).getLastPathSegment();
                    }
                    newPdfBinding.setTitle(title);

                    newPdfBinding.setReflowable(document.isReflowable());
                    if (newPdfBinding.isReflowable()) {
                        document.layout(width, height, 8);
                    }

                    newPdfBinding.setPageCount(document.countPages());

                    return Resource.success(newPdfBinding);
                } catch (Throwable t) {
                    message = t.getMessage();
                }

                return Resource.error(message, null);
            }
        };
        return execute.execute(new Object[]{pdfBinding, width, height});
    }

    public LiveData<Resource<PdfBinding>> loadOutline(PdfBinding pdfBinding) {
        ExecuteBoundResource<PdfBinding, PdfBinding> execute = new ExecuteBoundResource<PdfBinding, PdfBinding>() {
            @NonNull
            @Override
            protected Resource<PdfBinding> onExecute(PdfBinding args) {
                Resource<PdfBinding> result = null;
                try {
                    Outline[] outline = args.getDocument().loadOutline();
                    if (outline != null) {
                        List<OutlineItem> outlineItemList = new ArrayList<>();
                        flattenOutline(outlineItemList, outline, "");
                        args.setOutlineItemList(outlineItemList);
                    }
                    result = Resource.success(args);
                } catch (Throwable e) {
                    e.printStackTrace();
                    result = Resource.error(e.getMessage(), args);
                }
                return result;
            }
        };
        return execute.execute(pdfBinding);
    }

    private void flattenOutline(List<OutlineItem> outlineItemList, Outline[] outline, String indent) {
        for (Outline node : outline) {
            if (node.title != null) {
                outlineItemList.add(new OutlineItem(indent + node.title.replaceAll("\r", "").replaceAll("\n", ""), node.page));
            }
            if (node.down != null) {
                flattenOutline(outlineItemList, node.down, indent + "    ");
            }
        }
    }

}
