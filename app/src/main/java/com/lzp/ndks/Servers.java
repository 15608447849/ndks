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

    @Override
    protected void onHandleIntent(@Nullable Intent intent) {
        Log.e("过度服务","onHandleIntent()");
        final String packageName = this.getPackageName();
        final String path = JniHelper.createRootPath(this);
        final String serverName ="am startservice --user 0 -n "+ packageName+"/com.wos.play.rootdir.model_monitor.soexcute.WatchServer";
        final String activity ="am start -a android.intent.action.MAIN -c android.intent.category.LAUNCHER -n "+packageName+"/com.lzp.ndks.MainActivity";
        RunJniHelper.getInstance().startMservice(serverName,activity,path,"/mnt/sdcard/logs",10);
    }
}
