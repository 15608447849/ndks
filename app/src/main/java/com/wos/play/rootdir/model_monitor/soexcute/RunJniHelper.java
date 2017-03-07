package com.wos.play.rootdir.model_monitor.soexcute;


import android.content.Context;
import android.os.Environment;

/**
 * Created by user on 2017/1/18.
 */

public class RunJniHelper {
    private static final String TAG = "PingGG_Jin_log";
    static {
        try {
            System.loadLibrary("serverHelper"); //加载so库
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    //单例
    private static RunJniHelper theInstance = null;
    private RunJniHelper() {
    }
    public static RunJniHelper getInstance() {
        if (theInstance == null)
            theInstance = new RunJniHelper();
        return theInstance;
    }
    public native void startMservice(String srvName, String sdcard);
    public native void stopMservice(String sdcard);
//    //打开监听服务 -请勿修改
    public void startWatch(Context context) {
        //获取包名
        String packageName = context.getPackageName();
        String temPath = createRootPath(context);
        String watchServerPath = packageName+"/com.wos.play.rootdir.model_monitor.soexcute.WatchServer";
        startMservice(watchServerPath, temPath);
    }
    /**
     * sd卡是否可用
     *
     * @return
     */
    private boolean isSdCardAvailable() {
        return Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED);
    }
    /**
     * 创建根缓存目录
     *
     * @return
     */
    public String createRootPath(Context context ) {
        String cacheRootPath = "/mnt/sdcard/wosplayer";
        if (isSdCardAvailable()) {
            // /sdcard/Android/data/<application package>/cache
            cacheRootPath = context.getExternalCacheDir().getPath();
        } else {
            // /data/data/<application package>/cache
            cacheRootPath = context.getCacheDir().getPath();
        }
        return cacheRootPath;
    }
}
