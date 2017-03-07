package com.wos.play.rootdir.model_monitor.soexcute;

import android.app.IntentService;
import android.app.Service;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.os.IBinder;
import android.support.annotation.IntDef;
import android.support.annotation.Nullable;
import android.util.Log;

/**
 * Created by 79306 on 2017/2/24.
 */
public class WatchServer extends Service {
    private static final String TAG = "守护监听";
    @Override
    public void onCreate() {
        super.onCreate();
        android.util.Log.i(TAG, "创建服务 - pid: "+android.os.Process.myPid());
    }
    @Override
    public void onDestroy() {
        super.onDestroy();
        android.util.Log.i(TAG, "销毁服务 - pid: "+android.os.Process.myPid());
    }
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
    @Override
    public int onStartCommand(Intent intent,int flags, int startId) {
        Log.e(TAG,"["+android.os.Process.myPid()+"]= = = = = = = = = = 监听中　: " +startId);
//        open();
        return super.onStartCommand(intent, flags, startId);
    }

    private void open() {
        Intent intent = new Intent();
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.setAction("android.intent.action.VIEW");
        Uri content_url = Uri.parse("http://www.baidu.com");
        intent.setData(content_url);
        startActivity(intent);
    }
}
