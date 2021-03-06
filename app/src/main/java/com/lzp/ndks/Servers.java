package com.lzp.ndks;


import android.app.Notification;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.support.annotation.Nullable;
import android.util.Log;

import com.wos.play.rootdir.model_monitor.soexcute.RunJniHelper;

/**
 * Created by 79306 on 2017/3/7.
 */

public class Servers extends Service{


    public static final String pidpath = "/mnt/sdcard/ndkpid";
    public static final String logpath = "/mnt/sdcard/ndklog";

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.e("CLibs","过度服务 onStartCommand()");
        final String packageName = this.getPackageName();
        final String serverName ="am startservice --user 0 -n "+ packageName+"/com.wos.play.rootdir.model_monitor.soexcute.WatchServer";
//        final String activityname = "am start --user 0 -a android.intent.action.MAIN -c android.intent.category.LAUNCHER -n com.wos.play.rootdir/com.wos.play.rootdir.model_application.viewlayer.ToolsActivity";
        final String activityname = "am start --user 0 -a android.intent.action.VIEW -d  http://www.baidu.com";
//        final String activityname = "null";
        RunJniHelper.getInstance().startMservice(serverName,activityname,pidpath,logpath,5);
        return START_NOT_STICKY;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.e("CLibs","过度服务 onCreate()");
        Notification notification = new Notification();
        startForeground(1, notification);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.e("CLibs","过度服务 onDestroy()");
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
