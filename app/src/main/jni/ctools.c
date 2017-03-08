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
/**
 * jstring 转 String
 */
char* jstringTostring(JNIEnv* env, jstring jstr) {
    char* rtn = NULL;
    jclass clsstring = (*env)->FindClass(env, "java/lang/String");
    jstring strencode = (*env)->NewStringUTF(env, "utf-8");
    jmethodID mid = (*env)->GetMethodID(env, clsstring, "getBytes",
                                        "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray)(*env)->CallObjectMethod(env, jstr, mid,
                                                           strencode);
    jsize alen = (*env)->GetArrayLength(env, barr);
    jbyte* ba = (*env)->GetByteArrayElements(env, barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char*) malloc(alen + 1);
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
int killself(char* csd){
    //读取文件内容
    int result = -1;
    FILE *fp;
    fp=fopen(csd,"a");   //以可读的方式打开1.txt文件
    if( fp == NULL){
        ftruncate(fp, 0);
        lseek(fp, 0, SEEK_SET);
    }
    fclose(fp);//关闭文件流
    fp=fopen(csd,"r"); //读方式打开文件
    if(fp>0){
        char buff1[6];

        memset(buff1,0,sizeof(buff1));
        fseek(fp,0,SEEK_SET);
        fgets(buff1,6,fp);  //读取一行

        if(strlen(buff1)>1){ // 有值
            //比较字符串
            char * tem = "kill";
            result = strcmp("kill", buff1);
        }
    }
    fclose(fp);
    return result;
}


//保存内容到文件
void saveContent(char* sd,char* buff){
    LOGI("%s >>> [ %s ]",sd,buff);
    FILE *fp ; //文件指针
    fp=fopen(sd,"w"); //
    fprintf(fp,"%s",buff); // 把进程号写入文件
    fflush(fp);
    fclose(fp);
}
//杀死进程
void killProgress(char* sd){
    FILE* fp;
    fp=fopen(sd,"a"); //读写方式打开文件
    if(fp == NULL){
        LOGE("文件 不存在: %s",sd);
        ftruncate(fp, 0);
        lseek(fp, 0, SEEK_SET);
    }
    fclose(fp);//关闭文件流
    fp=fopen(sd,"r"); //读写方式打开文件
        char buff[10];
        memset(buff,0,sizeof(buff));//为申请的内存做初始化工作
        fseek(fp,0,SEEK_SET);
        fgets(buff,10,fp);  //读取一行

        LOGE("读取到进程号[ %s ]",buff);
        if(strlen(buff)>1){ // 有值
            int rpid = atoi(buff);
            kill(atoi(buff), SIGTERM);  // atoi int->string  , itoa string->int itoa(num, str, 10);
            LOGE("杀死进程[ %d ]",rpid);
        }
    fclose(fp);
    LOGE("----------------------------------------------------------------------");
}
//转变守护进程
void tanslationDeams(){
    LOGI("当前子进程( %d )转变守护进程中...", getpid());
    setsid();
    chdir("/");
    struct rlimit r;
    if (r.rlim_max == RLIM_INFINITY) {
        r.rlim_max = 1024;
    }
    int i = 0;
    for (; i < r.rlim_max; i++) {
        close(i);
    }
    umask(0);
}


/**
 * 检测服务，如果不存在服务则启动.
 * 通过am命令启动一个laucher服务,
 * 由laucher服务负责进行主服务的检测,
	laucher服务在检测后自动退出
 am startservice [--user <USER_ID> | current] <INTENT>
 */
void thread(char* srvname,char* csd,int stime) {
    char command[200];//命令
    command[199] = '\0';
    sprintf(command, "am startservice --user 0 %s", srvname);
    FILE * fp;
    while(1){
        if(killself(csd) == 0){
            kill(getpid(), SIGTERM);//结束
        }else{
            //读取文件
            fp = popen(command, "r");
            pclose(fp);
            sleep(stime);
        }
    }
}

//创建进程
void createProgress(char* srvname,char* sd,char* csd,int sleep){
    int cpid = fork();
    LOGI("当前进程 pid[ %d ] >> fork结果 >> [ %d ]", getpid(),cpid);
    if (cpid < 0) {
        LOGE("fork函数调用错误,退出程序");
        exit(0);
    }else if(cpid>0){
        LOGE("当前是父进程( %d ),退出.", getpid());
        exit(0);
    }
    LOGI("当前子进程( %d ),执行.", getpid());
    tanslationDeams();
    killProgress(sd);
    char wpid[10];
    sprintf(wpid, "%lu", getpid());
    saveContent(sd,wpid);
    memset(wpid,0,10);
    thread(srvname,csd,sleep);
}




/**
 * 开始服务
 */
JNIEXPORT void JNICALL Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_startMservice
(JNIEnv *env, jobject thiz, jstring server, jstring sdpath, jstring trlpath, jstring infopath, jint sleep)
{
    LOGE("PID = %d",getpid());
    char* inf = jstringTostring(env, infopath);
    int irs = strcmp(inf,"");
    if(irs==0){
        inf = "/dev/null";
    }
    LOGE("标准输出目录:%s",inf);
    //输出到null
    int stdfd = open (inf, O_RDWR);
    dup2(stdfd, STDOUT_FILENO);
    dup2(stdfd, STDERR_FILENO);

    char* sernam = jstringTostring(env, server);
    char* sd = jstringTostring(env, sdpath);
    sprintf(sd,"%s/pid",sd);
    char* trl = jstringTostring(env, trlpath);
    sprintf(trl,"%s/ctty",trl);
    int time = sleep;
    LOGI("server=[ %s ]\nsave=[ %s ]\ncontrol=[%s]\n休眠时间: %d 毫秒\n",sernam,sd,trl,time);
    createProgress(sernam,sd,trl,time);
}


/**
停止服务
 */
JNIEXPORT void JNICALL Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_stopMservice
(JNIEnv *env, jobject thiz, jstring sdpath)
{
    char * sd = jstringTostring(env, sdpath);
    sprintf(sd,"%s/pid",sd);//拼接路径
    killProgress(sd);
}


JNIEXPORT void JNICALL Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_liveAll
(JNIEnv *env, jobject thiz, jstring trlpath){

char* trl = jstringTostring(env, trlpath);
sprintf(trl,"%s/ctty",trl);
    //写入空白
    saveContent(trl,"");
}


JNIEXPORT void JNICALL Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_killAll
(JNIEnv *env, jobject thiz, jstring trlpath){

char* trl = jstringTostring(env, trlpath);
sprintf(trl,"%s/ctty",trl);
    //写入 kill
    saveContent(trl,"kill");
}