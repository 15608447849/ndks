package com.wos.play.rootdir.model_monitor.soexcute;

import android.content.Context;
import android.os.Environment;

/**
 * Created by 79306 on 2017/3/8.
 */

public class JniHelper {
    //    //打开监听服务 -请勿修改
    public static void startWatch(Context context) {
        //获取包名
        String packageName = context.getPackageName();
        String temPath = createRootPath(context);
        String watchServerPath = packageName+"/com.wos.play.rootdir.model_monitor.soexcute.WatchServer";
        //startMservice(watchServerPath, temPath);
    }
    /**
     * sd卡是否可用
     *
     * @return
     */
    private static boolean isSdCardAvailable() {
        return Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED);
    }
    /**
     * 创建根缓存目录
     *
     * @return
     */
    public static String createRootPath(Context context ) {
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
