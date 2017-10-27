#include <com_wos_play_rootdir_model_monitor_soexcute_RunJniHelper.h>
#include <jni.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include<time.h>
#include <pthread.h>
#include <android/log.h>

#define  TAG "CLibs"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
#define  PIDFILE "/mnt/sdcard/pfile"
#define  CONTROL_FILE "/mnt/sdcard/control"
#define  LOG_FILE "/mnt/sdcard/clog"


//传递的参数对象
struct tparam {
    char *command;
    //server命令
    char *acomd;
    //activity命令
    char *pfile;
    //上一次保存的进程pid 记录文件(本程序)
    char *fpath;
    //日志输出
    char *ctypath;
    //控制文件
    int sleeptime;//休眠时间
    char* commandArr[10];
    char* acomdArr[10];
};

static struct tparam para;

/**
 * 切割字符串 变成数组
 */
void cstringSpli(const char* srcStr, const char* seps, char* p_array[]) {
    char temp[256];
    memset(temp, 0, sizeof(temp));
    strcpy(temp, srcStr);
    char *token;
    token = strtok(temp, seps);
    int index = 0;
    while (token != NULL)
    {
        p_array[index] = (char*)malloc(sizeof(char) * strlen(token));
        strcpy(p_array[index], token);
        token = strtok(NULL, seps);
        index++;
    }
}





/**
 * jstring 转 String
 */
char *jstringTostring(JNIEnv *env, jstring jstr) {
    char *rtn = NULL;
    jclass clsstring = (*env)->FindClass(env, "java/lang/String");
    jstring strencode = (*env)->NewStringUTF(env, "utf-8");
    jmethodID mid = (*env)->GetMethodID(env, clsstring, "getBytes", "(Ljava/lang/String;)[B");
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
/**文件是否存在,不存在创建*/
void file_exist(char *path) {
    FILE *fp;
    fp = fopen(path, "a"); //追加方式打开
    if (NULL == fp) {
        LOGE("文件不存在[%s]准备创建中.", path);
        ftruncate(fp, 0);
        lseek(fp, 0, SEEK_SET);
    }
    fclose(fp);//关闭文件流
}

/**
 *是否允许执行
 * 1 允许
 * 0 禁止
 */
int is_enable_run(char *csd) {
    file_exist(csd);
    //读取文件内容
    int result = 1;
    FILE *fp;
    fp = fopen(csd, "r"); //读方式打开文件
    if (fp > 0) {
        char buff1[5];
        memset(buff1, 0, sizeof(buff1));
        fseek(fp, 0, SEEK_SET);
        fgets(buff1, 6, fp);  //读取一行

        if (strlen(buff1) > 1) { // 有值
            //比较字符串
           if(strcmp("true", buff1)==0){
               result = 0;
           }else{
               result = 1;
           }
        }
    }
    fclose(fp);
    return result;
}

/**
 * 追加内容到指定文件
 */
void apped_content(char *sd, char *buff) {
    file_exist(sd);
    FILE *fp;
    fp = fopen(sd, "a+");//以追加的方式打开文件
    fputs(buff, fp);//将内容 追加到  pf文件中.
    fflush(fp);
    fclose(fp);
}

/**保存内容到文件*/
void save_content(char *sd, char *buff) {
    file_exist(sd);
    LOGE("文件: %s 写入内容 : %s", sd, buff);
    FILE *fp; //文件指针
    fp = fopen(sd, "w"); //
    fprintf(fp, "%s", buff); // 把进程号写入文件
    fflush(fp);
    fclose(fp);
}

/**读取文本保存的进程pid杀死进程*/
void kill_progress(char *sd) {
    file_exist(sd);
    FILE *fp;
    fp = fopen(sd, "r"); //只读方式打开文件
    char buff[10];
    memset(buff, 0, sizeof(buff));//为申请的内存做初始化工作
    fseek(fp, 0, SEEK_SET);
    fgets(buff, 10, fp);  //读取一行
    LOGE("文件[ %s ]中读取到进程号:[ %s ]", sd, buff);
    if (strlen(buff) > 1) { // 有值
        int rpid = atoi(buff);
        kill(atoi(buff), SIGTERM);  // atoi int->string  , itoa string->int itoa(num, str, 10);
        LOGE("已经杀死进程[ %d ]", rpid);
    }
    fclose(fp);
}

/**关闭文件描述符*/
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

/**fork*/
int fork_self(int tag) {
    int cpid = fork();
    if(tag==0){
        LOGE("进程 pid[ %d ] => 创建子进程:[ %d ]", getpid(), cpid);
    }
    if (cpid < 0) {
        if(tag==0){
            LOGE("pid = %d 执行fork函数调用错误,退出程序", getpid());
        }
        exit(-1);

    } else if (cpid > 0) {
        if (tag == 0) {

            LOGE("父进程( %d ),退出.", getpid());
            exit(0);
        }else{
            return cpid;
        }

    } else if (cpid == 0) {
        if (tag == 0) {
            LOGE("当前子进程pid = %d ,准备脱离会话组 ...  \n\n", getpid());
        }
        return tag;
    }
    return -1;
}

//转变守护进程
int tanslation_deams(char *console) {
    char buff[20];
    int spid = setsid();
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "current process pid ( %lu ) setpid() result = %d \n", getpid(),spid);//
    apped_content(console,buff);
    if (spid < 0) {
        exit(0);
    };
    chdir("/");
    apped_content(console, "chdir(/) success!\n");
    umask(0);
    apped_content(console, "umask(0) success!\n");
    //关闭文件描述符
    if (close_all_fd() != 0) {
        //记录到日志文件
        apped_content(console, "close all file descriptor error!\n");
    } else {
        apped_content(console, "close all file descriptor success!\n");
    }
}




void execute_command(char* paramArr[]) {

    if(fork()==0){
        if(execvp("am",paramArr)<0) {
            exit(0);
        }
    }
}


void recored_pid(char* recredfile,char* logfile){
    char buff[35];
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "%lu", getpid());
    save_content(recredfile, buff);//保存pid

    memset(buff, 0, sizeof(buff));//清空
    sprintf(buff, "%d recored success, recored file path is [ %s ]\n", getpid(),recredfile);
    apped_content(logfile,buff);
};
//创建进程
void mmain() {
    int result = 0;
    kill_progress(para.pfile);//结束可能存在的进程
    //清空日志
    char buffstr[100];
    memset(buffstr, 0, sizeof(buffstr));
    sprintf(buffstr, "current process pid ( %lu ) , clear log files. \n\r", getpid());//
    save_content(para.fpath, buffstr);//清空上一次的log
    result = fork();
    if (result != 0) {
        LOGE("父进程( %d ),退出.", getpid());
        exit(0);
    }else{
        LOGE("子进程( %d ),父进程( %d ),标识码( %d ),组标识( %d ),准备脱离会话组.", getpid(),getppid(),getuid(),getgid());
    }
    //脱离进程组
    result = setsid();
    memset(buffstr, 0, sizeof(buffstr));
    sprintf(buffstr, "current process pid ( %lu )     , ppid ( %lu )    , uid ( %lu )    , gid( %lu )     , setpid result = %d .\n\r", getpid(),getppid(),getuid(),getgid(),result);//
    apped_content(para.fpath,buffstr);

    if (result < 0) {
        exit(-1);
    }
    umask(0);//重设文件创建掩码
    apped_content(para.fpath, "umask(0).\n\r");
    result = fork();
    if(result == 0){
        LOGE("fork第二次,子进程( %d ),执行.", getpid());
        //孙子进程
        chdir("/");//更改目录
        apped_content(para.fpath, "chdir(/).\n\r");
        for (int i = 0; i < 5; i++)
        {
            close (i);
        }
        apped_content(para.fpath, "close all file descriptor.\n\r");
        int stdfd = open ("/dev/null", O_RDWR);
        dup2(stdfd, STDOUT_FILENO);
        dup2(stdfd, STDERR_FILENO);
        apped_content(para.fpath, "stdio signal close.\n\r");

        //记录本次pid
        memset(buffstr, 0, sizeof(buffstr));
        sprintf(buffstr, "current process pid ( %lu ) ,ppid ( %lu ), uid ( %lu ),gid( %lu ).\n\r", getpid(),getppid(),getuid(),getgid());//
        apped_content(para.fpath,buffstr);

        memset(buffstr, 0, sizeof(buffstr));
        sprintf(buffstr, "%lu", getpid());
        save_content(para.pfile, buffstr);//保存pid
        //记录操作日志
        memset(buffstr, 0, sizeof(buffstr));//清空
        sprintf(buffstr, "%d record success, file path :%s\n\r", getpid(),para.pfile);
        apped_content(para.fpath,buffstr);
        //执行监听
        apped_content(para.fpath,"watch android sever>>>\n\r");
//        execute_command(para.commandArr);
        execute_command(para.acomdArr);//activity
        //子进程循环监听
        while (is_enable_run(para.ctypath)==1) {//
            sleep(para.sleeptime);
            execute_command(para.commandArr);
        }
        apped_content(para.fpath,"<<<\n\rclose watch");
    }else{
        //第二次fork 父进程
        LOGE("fork第二次,父进程( %d ),退出.", getpid());
        exit(0);
    }

}

//转变对象
void tanslation_param(char *srvname, char *acty, char *sd, char *trl, char *tinf, int sleeptime) {
    para.command = srvname;
    para.acomd = acty;
    para.pfile = sd;
    para.ctypath = trl;//控制
    para.fpath = tinf;//日志
    para.sleeptime = sleeptime;//休眠

    memset(para.commandArr, NULL, sizeof(para.commandArr));
    cstringSpli(para.command," ", para.commandArr);

    for (int index = 0; index < 10; index++) {
        if (para.commandArr[index] == NULL) continue;
        LOGE("%d : %s,", index,para.commandArr[index]);
    }

    memset(para.acomdArr, NULL, sizeof(para.acomdArr));
    cstringSpli(para.acomd," ", para.acomdArr);

    for (int index = 0; index < 10; index++) {
        if (para.acomdArr[index] == NULL) continue;
        LOGE("%d : %s,", index,para.acomdArr[index]);
    }

    LOGE("服务命令=[%s]\n活动命令=[%s]\npid保存路径=[%s]\n控制文件=[%s]\n日志文件=[%s]\n休眠时间 = %d 毫秒",
         para.command,
         para.acomd,
         para.pfile,
         para.ctypath,
         para.fpath,
         para.sleeptime);

    //初始化文件
    file_exist(para.pfile);//pid记录文本
    file_exist(para.ctypath);//创建控制文本
    file_exist(para.fpath);//日志打印文本
    mmain();
}

/**
 *
 * 开始服务(入口)
 */
JNIEXPORT void JNICALL Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_startMservice
(JNIEnv* env,jobject thiz,jstring server,jstring sdpath, jstring trlpath,jstring infopath, jint sleep)
{
//实时监听的服务名
char *sernam = jstringTostring(env, server);
//打开的activity命令
char *activity = jstringTostring(env, sdpath);
//pid保存
char *pid = jstringTostring(env, trlpath);
//pid保存文件
char *log = jstringTostring(env, infopath);//日志保存文件
if(pid==NULL){
pid = PIDFILE;
}
if(log==NULL){
log = LOG_FILE;
}
tanslation_param(sernam, activity, pid,CONTROL_FILE,log,sleep);
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
//pid保存
char *pid = jstringTostring(env, sdpath);
if(pid==NULL){
pid = PIDFILE;
}
kill_progress(pid);
}
//允许运行
JNIEXPORT void JNICALL
Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_liveAll
(JNIEnv
*env,
jobject thiz, jstring
trlpath){
save_content(CONTROL_FILE,"");
}
// 不允许运行
JNIEXPORT void JNICALL
Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_killAll
(JNIEnv
*env,
jobject thiz, jstring
trlpath){
save_content(CONTROL_FILE,"true");
}
