#include <com_wos_play_rootdir_model_monitor_soexcute_RunJniHelper.h>
#include <jni.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>
#include <android/log.h>

#define  TAG "CLibs"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
#define  FFPID "%s/c_nunber"
#define  CONTROL "%s/cys"

//线程参数
struct tparam {
    char *command;//server命令
    char *acomd;//activity命令
    char *pfile;//上一次保存的进程pid 记录文件
    char *fpath;//日志输出
    char *ctypath;//控制文件
    int sleep;//休眠时间
};


/**
 * jstring 转 String
 */
char* jstringTostring(JNIEnv *env, jstring jstr) {
    char *rtn = NULL;
    jclass clsstring = (*env)->FindClass(env, "java/lang/String");
    jstring strencode = (*env)->NewStringUTF(env, "utf-8");
    jmethodID mid = (*env)->GetMethodID(env, clsstring, "getBytes","(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray)(*env)->CallObjectMethod(env, jstr, mid, strencode);
    jsize alen = (*env)->GetArrayLength(env, barr);
    jbyte *ba = (*env)->GetByteArrayElements(env, barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char *) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    (*env)->ReleaseByteArrayElements(env, barr, ba, 0);
    return rtn;
}

/**
 * 杀死自己
 * @param csd
 * @return
 */
int killself(char *csd) {
    //读取文件内容
    int result = -1;
    FILE *fp;
    fp = fopen(csd, "a");   //以可读的方式打开1.txt文件
    if (fp == NULL) {
        ftruncate(fp, 0);
        lseek(fp, 0, SEEK_SET);
    }
    fclose(fp);//关闭文件流
    fp = fopen(csd, "r"); //读方式打开文件
    if (fp > 0) {
        char buff1[6];

        memset(buff1, 0, sizeof(buff1));
        fseek(fp, 0, SEEK_SET);
        fgets(buff1, 6, fp);  //读取一行

        if (strlen(buff1) > 1) { // 有值
            //比较字符串
            char *tem = "kill";
            result = strcmp("kill", buff1);
        }
    }
    fclose(fp);
    return result;
}
//文件是否存在,不存在创建
void fd_exist(char *path) {
    FILE *fp;
    fp = fopen(path, "a"); //追加方式打开
    if (NULL == fp) {
        LOGE("文件不存在[%s]准备创建中.", path);
        ftruncate(fp, 0);
        lseek(fp, 0, SEEK_SET);
    }
    fclose(fp);//关闭文件流
}
void apped_content(char* sd, char* buff) {
    FILE *fp;
    fp = fopen(sd, "a+");//以追加的方式打开文件
    fputs(buff, fp);//将内容 追加到  pf文件中.
    fflush(fp);
    fclose(fp);
}

//保存内容到文件
void save_content(char* sd, char* buff) {
    LOGE("文件: %s 写入内容 [%s]", sd, buff);
    FILE *fp; //文件指针
    fp = fopen(sd, "w"); //
    fprintf(fp, "%s", buff); // 把进程号写入文件
    fflush(fp);
    fclose(fp);
}

//杀死进程
void killProgress(char *sd) {
    fd_exist(sd);
    FILE *fp;
    fp = fopen(sd, "r"); //只读方式打开文件
    char buff[10];
    memset(buff, 0, sizeof(buff));//为申请的内存做初始化工作
    fseek(fp, 0, SEEK_SET);
    fgets(buff, 10, fp);  //读取一行
    LOGE("文件[ %s ]中读取到进程号:[ %s ]",sd, buff);
    if (strlen(buff) > 1) { // 有值
        int rpid = atoi(buff);
        kill(atoi(buff), SIGTERM);  // atoi int->string  , itoa string->int itoa(num, str, 10);
        LOGE("已经杀死进程[ %d ]", rpid);
    }
    fclose(fp);
}
//关闭文件描述符
int close_all_fd(void) {
    struct rlimit lim;
    unsigned int i;

    if (getrlimit(RLIMIT_NOFILE, &lim) < 0)
        return -1;
    if (lim.rlim_cur == RLIM_INFINITY)
        lim.rlim_cur = 1024;
    for (i = 0; i < lim.rlim_cur; i++) {
#ifdef MYPERF
        if (i == 1) continue;
#endif
        if (close(i) < 0 && errno != EBADF)
            return -1;
    }
    return 0;
}

void fork_self() {
    int cpid = fork();
    LOGE("当前进程 pid[ %d ] , 创建子进程:[ %d ]", getpid(), cpid);
    if (cpid < 0) {
        LOGE("pid = %d 执行fork函数调用错误,退出程序", getpid());
        exit(1);
    } else if (cpid > 0) {
        LOGE("当前是父进程( %d ),退出.", getpid());
        exit(0);
    }
//    LOGE("准备继续执行的当前子进程pid = %d ",getpid());
}

//转变守护进程
void tanslationDeams(char* console) {
    int spid = setsid();
    if (spid < 0) {
        LOGE("当前子进程( %d )转变会话期组长失败.返回值 : %d ", getpid(), spid);
        exit(0);
    };
    LOGE("当前子进程( %d )转变会话期组长成功. ( %d )", getpid(), spid);
    chdir("/");
    apped_content(console,"chdir() success!\n");
    umask(0);
    apped_content(console,"umask(0) success!\n");
    //关闭文件描述符
    if(close_all_fd() != 0){
        //记录到日志文件
        apped_content(console,"close_all_fd error!\n");
    };
    apped_content(console,"close_all_fd success!\n");
}


void excute_opon(char* console,char* command){

    int rc = 0;
    char result_buf[1024];
    char* line;
    FILE *fp;
    fp = popen(command, "r");
    if(NULL == fp)
    {
       apped_content(command,"popen() is error.file is null.\n");
    }else{

        memset(result_buf, 0, sizeof(result_buf));//清空
        sprintf(result_buf, " opon _ pid = %lu \n", getpid());
        line = result_buf;
        apped_content(console,line);
        while(fgets(result_buf, sizeof(result_buf), fp) != NULL)
        {
            if('\n' == result_buf[strlen(result_buf)-1])
            {
                result_buf[strlen(result_buf)-1] = '\0';
                line = result_buf;
                apped_content(console,line);
            }
        }
    }

    rc = pclose(fp);
    if(-1 == rc)
    {
        apped_content(console,"\npclose() is error.\n");
        exit(1);
    }else{
        sprintf(result_buf, "\npclose() result:%d ,return:%d \n",  rc, WEXITSTATUS(rc));
        apped_content(console,result_buf);
    }
}
/**
 * 通过am命令启动一个laucher服务,
 am startservice [--user <USER_ID> | current] <INTENT>
*/

void thread(void *arg) {//
    struct tparam *param = (struct tparam *) arg;
    int res;
    while (1) {
//        save_content(param->fpath,"");//清空上一次的log
        excute_opon(param->fpath,param->command);
        sleep(param->sleep);
    }
}

//创建进程
void createProgress(char *srvname,char* acty, char* sd, char* trl, char* tinf, int sleep) {
    struct tparam para;
        para.command = srvname;
        para.acomd = acty;
        para.pfile = sd;
        para.ctypath = trl;//控制
        para.fpath = tinf;//日志
        para.sleep = sleep;//休眠

    LOGE("当前进程 pid = %d", getpid());
    LOGE("服务命令=[%s]\n活动命令=[%s]\npid保存路径=[%s]\n控制文件=[%s]\n日志文件=[%s]\n休眠时间 = %d 毫秒",
         para.command,
         para.acomd,
         para.pfile,
         para.ctypath,
         para.fpath,
         para.sleep);

    fd_exist(para.pfile);//pid记录文本
    fd_exist(para.ctypath);//创建控制文本
    fd_exist(para.fpath);//日志打印文本

    save_content(para.fpath,"");//清空上一次的log
    killProgress(para.pfile);//结束可能存在的进程
    fork_self();
    tanslationDeams(para.fpath);

    //记录本次pid
    char wpid[10];
    memset(wpid, 0, sizeof(wpid));//清空
    sprintf(wpid, "%lu", getpid());
    save_content(para.pfile, wpid);
    apped_content(para.fpath,"recored pid success!\n");

    excute_opon(para.fpath,para.acomd);//执行打开 activity
    thread(&para); //循环监听
//    char tmp[10];
//    memset(tmp, 0, sizeof(tmp));//清空
//    sprintf(tmp, "progress pid = %lu ,over\n", getpid());
//    apped_content(para.fpath, tmp);
}

/**
停止服务
 */
JNIEXPORT void JNICALL
Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_stopMservice
(JNIEnv
*env,
jobject thiz, jstring
sdpath)
{
char *sd = jstringTostring(env, sdpath);
sprintf(sd,FFPID,sd);//拼接路径
killProgress(sd);
}
//允许打开
JNIEXPORT void JNICALL
Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_liveAll
(JNIEnv
*env,
jobject thiz, jstring
trlpath){
char* trl = jstringTostring(env, trlpath);
sprintf(trl,CONTROL,trl);
save_content(trl,"");
}
// 关闭所有
JNIEXPORT void JNICALL
Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_killAll
(JNIEnv
*env,
jobject thiz, jstring
trlpath){
char* trl = jstringTostring(env, trlpath);
sprintf(trl,CONTROL,trl);
//写入 kill
save_content(trl,"kill");
}
/**
 *
 * 开始服务(入口)
 */
JNIEXPORT void JNICALL
Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_startMservice
(JNIEnv
*env,
jobject thiz, jstring
server,
jstring sdpath, jstring
trlpath,
jstring infopath, jint
sleep)
{
char* sernam = jstringTostring(env, server);//实时监听的服务名
char* activity = jstringTostring(env, sdpath);//打开的activity命令
char* path = jstringTostring(env, trlpath);
char* inf = jstringTostring(env, infopath);
int time = sleep;
LOGE("%s\n%s\n%s\n%s\n",sernam,activity,path,inf);


char tme[20];
memset(tme,0,sizeof(tme));
strcpy(tme,path);//C语言标准库函数，将第二个参数的内容复制到第一个参数;
sprintf(tme,CONTROL,tme); //控制文件
sprintf(path,FFPID,path); //PID保存
createProgress(sernam,activity,path,tme,"/mnt/sdcard/clog",time);
}






































//    //输出到null
//    int stdfd = open ("/dev/null", O_RDWR);
//    dup2(stdfd, STDOUT_FILENO);
//    dup2(stdfd, STDERR_FILENO);




//    LOGE("%d再次执行fork函数.",getpid());
//    int cpid = fork();
//    LOGE("当前进程 pid[ %d ] >> 二次fork() 结果 >> [ %d ]", getpid(),cpid);
//    if (cpid < 0) {
//        LOGE("fork函数调用错误,退出程序");
//        exit(1);
//    }else if(cpid>0){
//        LOGE("当前二次fork() 是父进程( %d ),杀死!", getpid() );
//        kill(getpid(), SIGTERM);//结束
//    }
//    LOGE("二次fork() 子进程( %d ) .", getpid());
//    struct rlimit r;
//    if (r.rlim_max == RLIM_INFINITY) {
//        r.rlim_max = 1024;
//    }
//    int i;
//    for (i = 0; i < r.rlim_max; i++) {
//        close(i);
//    }
//
//    for (; i < 3; ++i) {
//        close(i);
//        open("/dev/null", O_RDWR);
//        dup(0);
//        dup(0);
//    }
//    umask(0);



//    int t_res;
//    pthread_t trd;
//    t_res = pthread_create(&trd, NULL, (void *) thread, &para);//创建线程
//    if (t_res != 0) {
//        save_content(para.fpath, "create thread error.\n");
//        //写入日志
//        exit(1);
//    }