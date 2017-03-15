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

void saveContentAdd(char* sd,char* buff){
    char * buf;
    buf = buff;
    FILE * fp;
    fp = fopen(sd,"a+");//以追加的方式打开文件
    fputs(buf,fp);//将内容 追加到  pf文件中.
    fflush(fp);
    fclose(fp);
}
//保存内容到文件
void saveContent(char* sd,char* buff){
    LOGE("file : %s ,write content: [ %s ]",sd,buff);
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
}
//转变守护进程
void tanslationDeams(){
    LOGE("当前子进程( %d )转变组长中...", getpid());
    if(setsid()<0){
        exit(0);
    };
    umask(0);
    LOGE("%d再次执行fork函数.",getpid());
    int cpid = fork();
    LOGE("当前进程 pid[ %d ] >> 二次fork() 结果 >> [ %d ]", getpid(),cpid);
    if (cpid < 0) {
        LOGE("fork函数调用错误,退出程序");
        exit(1);
    }else if(cpid>0){
        LOGE("当前二次fork() 是父进程( %d ),杀死!", getpid() );
        kill(getpid(), SIGTERM);//结束
    }
    LOGE("二次fork() 子进程( %d ) .", getpid());
    chdir("/");
    struct rlimit r;
    if (r.rlim_max == RLIM_INFINITY) {
        r.rlim_max = 1024;
    }
    int i;
    for (i = 0; i < r.rlim_max; i++) {
        close(i);
    }
//
//    for (; i < 3; ++i) {
//        close(i);
//        open("/dev/null", O_RDWR);
//        dup(0);
//        dup(0);
//    }
    umask(0);
}

//线程参数
struct tparam
{
    char* command;//命令
    char* fpath;//输出
    int sleep;//休眠时间
};
/**
 * 检测服务，如果不存在服务则启动.
 * 通过am命令启动一个laucher服务,
 * 由laucher服务负责进行主服务的检测,
	laucher服务在检测后自动退出
 am startservice [--user <USER_ID> | current] <INTENT>
 am startservice --user 0 %s &
*/
//        if(killself(csd) == 0){
//                saveContentAdd(csd,"killself");
//                kill(getpid(), SIGTERM);//结束
//            }else{
//
//        }
//            if(system(command) != 0){
//                saveContentAdd(csd,"Excute system() funcation start service command fait");
//                FILE * fp;
//                fp = popen(command, "r");
//                    if(fp != NULL){
//                        pclose(fp);
//                    }
//            }
void thread(void* arg) {//
    struct tparam * param ;
    char* command;
    char* csd;
    int stime;
    param = (struct tparam *) arg;
    command = param->command;
    csd = param->fpath;
    stime = param->sleep;

    saveContentAdd(csd,"commnd = ");
    saveContentAdd(csd,command);
    saveContentAdd(csd,"\n");
    int res;
    while(1){
            res = system(command);
                  if( res == 0){

                  }else{
                      saveContentAdd(csd,"excute system() failt");
                      saveContentAdd(csd,"\n");
                      FILE * fp;
                      fp = popen(command, "r");
                      pclose(fp);
                  }
            sleep(stime);
    }
}

//创建进程
void createProgress(char* srvname,char* sd,char* trl,char* tinf,int sleep){

    saveContentAdd(tinf,"c文件执行中");
    killProgress(sd);//结束可能存在的进程
    int cpid = fork();
    LOGE("当前进程 pid[ %d ] >> fork结果 >> [ %d ]", getpid(),cpid);
    if (cpid < 0) {
        LOGE("fork函数调用错误,退出程序");
        exit(1);
    }else if(cpid>0){
        LOGE("当前是父进程( %d ),退出.", getpid());
        exit(0);
    }
    LOGE("当前子进程( %d ),执行判断完毕.");
    tanslationDeams();
    char wpid[10];
    sprintf(wpid, "%lu", getpid());
    saveContent(sd,wpid);
    memset(wpid,0,10);
//    thread(srvname,csd,sleep);
    struct tparam para;
        para.command = srvname;
        para.fpath = tinf;
        para.sleep = sleep;
    pthread_t trd;
    int t_res;
    t_res = pthread_create(&trd, NULL, (void *) thread, &para);//创建线程
    if(t_res!=0){
        saveContent(trl,"create thread error.");
        exit(1);
    }
    saveContent(trl,"progress over");
}




/**
 * 开始服务
 */
JNIEXPORT void JNICALL Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_startMservice
(JNIEnv *env, jobject thiz, jstring server, jstring sdpath, jstring trlpath, jstring infopath, jint sleep)
{
    //输出到null
    int stdfd = open ("/dev/null", O_RDWR);
    dup2(stdfd, STDOUT_FILENO);
    dup2(stdfd, STDERR_FILENO);

    LOGE("PID = %d",getpid());

    char* sernam = jstringTostring(env, server);
    char* sd = jstringTostring(env, sdpath);
    char* trl = jstringTostring(env, trlpath);
    char* inf = jstringTostring(env, infopath);

    sprintf(sd,"%s/pef",sd);
    sprintf(trl,"%s/ctty",trl);
    int time = sleep;
    LOGE("service=[%s]\npid保存路径sd=[%s]\n进程信息trl=[%s]\n线程信息inf=[%s]\n休眠时间: %d 毫秒",sernam,sd,trl,inf,time);
    createProgress(sernam,sd,trl,inf,time);
}


/**
停止服务
 */
JNIEXPORT void JNICALL Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_stopMservice
(JNIEnv *env, jobject thiz, jstring sdpath)
{
    char * sd = jstringTostring(env, sdpath);
    sprintf(sd,"%s/pef",sd);//拼接路径
    killProgress(sd);
}


JNIEXPORT void JNICALL Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_liveAll
(JNIEnv *env, jobject thiz, jstring trlpath){

char* trl = jstringTostring(env, trlpath);
sprintf(trl,"%s/ctty",trl);
    //写入空白
    saveContent(trl,"nokill");
}


JNIEXPORT void JNICALL Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_killAll
(JNIEnv *env, jobject thiz, jstring trlpath){

char* trl = jstringTostring(env, trlpath);
sprintf(trl,"%s/ctty",trl);
    //写入 kill
    saveContent(trl,"kill");
}