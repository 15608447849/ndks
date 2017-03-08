package com.lzp.ndks;


import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import com.wos.play.rootdir.model_monitor.soexcute.JniHelper;
import com.wos.play.rootdir.model_monitor.soexcute.RunJniHelper;

import java.io.File;
import java.io.FileOutputStream;

public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }
    public void testOpen(View view){
        Intent i = new Intent(MainActivity.this,Servers.class);
        startService(i);
    }
    public void testClose(View view){
        String path = JniHelper.createRootPath(this);
        RunJniHelper.getInstance().stopMservice(path);
    }
    public void x(View view){
        final String path = JniHelper.createRootPath(this);
        RunJniHelper.getInstance().killAll(path);

    }
    public void s(View view){
        final String path = JniHelper.createRootPath(this);
        RunJniHelper.getInstance().liveAll(path);
    }



    public void write(String str,String file){
        try {
            FileOutputStream outSTr = new FileOutputStream(new File(file));
            outSTr.write(str.getBytes());
            outSTr.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


}
