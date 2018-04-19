package com.chaoxing.epub;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import com.chaoxing.epub.nativeapi.EpubDocument;

public class EpubActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.libepub_activity_epub);
        ((TextView) findViewById(R.id.tv_hello)).setText(EpubDocument.sayHello());
    }

}
