package com.lzp.ndks;

import android.app.IntentService;
import android.content.Intent;
import android.support.annotation.Nullable;
import android.util.Log;

import com.wos.play.rootdir.model_monitor.soexcute.RunJniHelper;

/**
 * Created by 79306 on 2017/3/7.
 */

public class Servers extends IntentService {

    public Servers() {
        super("qweqweqwewq");
    }
    public static final String pidpath = "/mnt/sdcard/ndks";
    public static final String logpath = "/mnt/sdcard/nlogs";

    @Override
    protected void onHandleIntent(@Nullable Intent intent) {
        Log.e("过度服务","onHandleIntent()");
        final String packageName = this.getPackageName();
        final String serverName ="am startservice --user 0 -n "+ packageName+"/com.wos.play.rootdir.model_monitor.soexcute.WatchServer";
//      final String activity ="am start -a android.intent.action.MAIN -c android.intent.category.LAUNCHER -n "+packageName+"/com.lzp.ndks.MainActivity";
        RunJniHelper.getInstance().startMservice(serverName,"null",pidpath,logpath,10);
    }
}
