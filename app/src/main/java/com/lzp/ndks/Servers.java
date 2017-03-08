package com.lzp.ndks;

import android.app.IntentService;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.support.annotation.IntDef;
import android.support.annotation.Nullable;
import android.util.Log;

import com.wos.play.rootdir.model_monitor.soexcute.JniHelper;
import com.wos.play.rootdir.model_monitor.soexcute.RunJniHelper;

import java.io.File;
import java.io.IOException;

/**
 * Created by 79306 on 2017/3/7.
 */

public class Servers extends IntentService {

    public Servers() {
        super("qweqweqwewq");
    }
//
//    @Nullable
//    @Override
//    public IBinder onBind(Intent intent) {
//        return null;
//    }

    @Override
    protected void onHandleIntent(@Nullable Intent intent) {
        Log.e("过度服务","+++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
        final String packageName = this.getPackageName();
        final String path = JniHelper.createRootPath(this);
        final String serverName = packageName+"/com.wos.play.rootdir.model_monitor.soexcute.WatchServer";
        RunJniHelper.getInstance().startMservice(serverName,path,path,path+"/console",1);
    }
//
//    @Override
//    public void onCreate() {
//
//    }
//
//    @Override
//    public int onStartCommand(Intent intent,  int flags, int startId) {
//        return super.onStartCommand(intent, flags, startId);
//    }
}
