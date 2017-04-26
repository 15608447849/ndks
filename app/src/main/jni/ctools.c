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
    //上一次保存的进程pid 记录文件
    char *fpath;
    //日志输出
    char *ctypath;
    //控制文件
    int sleeptime;//休眠时间
};

static struct tparam para;

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
//    LOGE("fork()...tag:%d", tag);
    int cpid = fork();
    if(tag==0){
        LOGE("进程 pid[ %d ] => 创建子进程:[ %d ]", getpid(), cpid);
    }
    if (cpid < 0) {
        if(tag==0){
            LOGE("pid = %d 执行fork函数调用错误,退出程序", getpid());
        }
        exit(1);

    } else if (cpid > 0) {
        if (tag == 0) {

            //LOGE("父进程( %d ),等待子进程执行.", getpid());
            //wait(NULL);
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

int excute_command_popen_new(char *cmd) {
    FILE *fp;
    int res;
    char buf[1024];
    if (cmd == NULL) {
        printf("my_system cmd is NULL!\n");
        return -1;
    }
    if ((fp = popen(cmd, "r")) == NULL) {
        perror("popen");
        printf("popen error: %s/n", strerror(errno));
        return -1;
    }
    else {
        while (fgets(buf, sizeof(buf), fp)) {
            printf("%s", buf);
        }
        if ((res = pclose(fp)) == -1) {
            printf("close popen file pointer fp error!\n");
            return res;
        }
        else if (res == 0) {
            return res;
        }
        else {
            printf("popen res is :%d\n", res);
            return res;
        }
    }
}


/**打开命令*/
void excute_command_popen(char *command) { //char* console,
    FILE *fp;
    fp = popen(command, "r");
    if (NULL != fp) {
//        pclose(fp);
    }
}

/**打开命令*/
void excute_command_system(char *command) { //char* console,
    system(command);
}

/**执行线程打开命令*/
void thread_excute_command(char *fpath, char *command, void *funcation) {
    if (strcmp("null", command) <= 0) {
        return;
    }
    apped_content(fpath, "# create thread \n");
    pthread_t tid;
    int ret;
    ret = pthread_create(&tid, NULL, funcation, command);
    if (ret == 0) {
        apped_content(fpath, "# create pthread success!\n");
    } else {
        apped_content(fpath, "# create pthread error!\n");
    }
//    pthread_exit((void *)0);
}


void is_excute_command(char *command, char *logfile, int print) {
    if (strcmp("null", command) > 0) {
//      thread_excute_command(para.fpath,para.acomd,excute_command_popen_new);
        excute_command_popen(command);
        if (print == 0) {
            char buff[100];
            memset(buff, 0, sizeof(buff));
            sprintf(buff, "excute command:\t%s\tsuccess.\n", command);//
            apped_content(logfile, buff);
        }
    }
}

void singnals(){
    /*忽略终端Ｉ／Ｏ信号，ＳＴＯＰ信号*/
//    signal(SIGTTOU ,SIG_IGN);
//    signal(SIGTTIN ,SIG_IGN);
//    signal(SIGTSTP ,SIG_IGN);
//    signal(SIGHUP ,SIG_IGN);
}
void clear_log_file(char* path){
    char buff[20];
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "current process pid ( %lu ) , print log >>>\n", getpid());//
    save_content(path, buff);//清空上一次的log
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
    sprintf(buffstr, "current process pid ( %lu ) , clear log files,print log :\n", getpid());//
    save_content(para.fpath, buffstr);//清空上一次的log

    result = fork();

    if (result != 0) {
        LOGE("父进程( %d ),退出.", getpid());
        exit(0);
    }else{
        LOGE("子进程( %d ),父进程(%d),标识码(%d),组标识(%d),准备脱离会话组.", getpid(),getppid(),getuid(),getgid());
    }
    //脱离进程组
    result = setsid();
    memset(buffstr, 0, sizeof(buffstr));
    sprintf(buffstr, "current process pid ( %lu ) ,ppid ( %lu ), uid ( %lu ),gid( %lu ) ,setpid result = %d .\n", getpid(),getppid(),getuid(),getgid(),result);//
    apped_content(para.fpath,buffstr);

    if (result < 0) {
        exit(-1);
    };
    umask(0);//重设文件创建掩码
    apped_content(para.fpath, "umask(0).\n");
    result = fork();
    if(result == 0){
        LOGE("fork第二次,子进程( %d ),执行.", getpid());
        //孙子进程
        chdir("/");//更改目录
        apped_content(para.fpath, "chdir(/).\n");
        for (int i = 0; i < 5; i++)
        {
            close (i);
        }
        apped_content(para.fpath, "close all file descriptor.\n");
        int stdfd = open ("/dev/null", O_RDWR);
        dup2(stdfd, STDOUT_FILENO);
        dup2(stdfd, STDERR_FILENO);
        apped_content(para.fpath, "singnal over.\n");

        //记录本次pid

        memset(buffstr, 0, sizeof(buffstr));
        sprintf(buffstr, "%lu", getpid());
        save_content(para.pfile, buffstr);//保存pid
        //记录操作日志
        memset(buffstr, 0, sizeof(buffstr));//清空
        sprintf(buffstr, "%d recored success, recored file path :%s\n", getpid(),para.pfile);
        apped_content(para.fpath,buffstr);

        memset(buffstr, 0, sizeof(buffstr));
        sprintf(buffstr, "current process pid ( %lu ) ,ppid ( %lu ), uid ( %lu ),gid( %lu ).\n", getpid(),getppid(),getuid(),getgid());//
        apped_content(para.fpath,buffstr);

        thread_excute_command(para.fpath,para.acomd,excute_command_popen);//activity
        apped_content(para.fpath,"############################ start watch ###############################\n");
        //子进程循环监听
        while (is_enable_run(para.ctypath)==1) {//
            sleep(para.sleeptime);
            is_excute_command(para.command, para.fpath, 1);//server
        }
        apped_content(para.fpath,"############################ stop watch ###############################\n");
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






















//void time_func(){
//    while (1){
//            apped_content(para.fpath,"watch start ...>>>\n");
//            //创建线程
//            thread_excute_open(para.fpath,para.command);
//            apped_content(para.fpath,"watch over .\n");
//            apped_content(para.fpath,"====sleep start====\n");
//            sleep(para.sleeptime);// * 1000000
//            apped_content(para.fpath,"====sleep over====\n\n");
//
//    }
//    apped_content(para.fpath,"thread over\n");
//    pthread_exit((void *)0);


/* memset(buff, 0, sizeof(buff));//清空
 sprintf(buff, "main progress goto sleep %d minute.\n", para.sleeptime);
 apped_content(para.fpath,buff);
 sleep(para.sleeptime);
 apped_content(para.fpath," >>main progress sleep over!\n\n");
*/
//    mmain();

//pthread_t tid;
//    int ret;
//    ret=pthread_create(&tid,NULL,(void *)time_func,NULL);
//    if(ret==0) {
//        apped_content(para.fpath,"create time thread  success!\n");
//        pthread_join(tid, NULL);
//    }else{
//        apped_content(para.fpath,"create time thread  error!\n");
//    }


/*void thread(void *arg) {//

    struct tparam *param = (struct tparam *) arg;
    while (1) {
        apped_content(param->fpath,"watch server start...>>>\n");
//        save_content(param->fpath,"");//清空上一次的log
//        excute_opon(param->command);
        //创建线程
        thread_excute_open(param->fpath,param->command);
        apped_content(param->fpath,"watch server over.\n");
        sleep(5);
        apped_content(param->fpath,"====sleep over====\n\n");
    }
}*/



//    thread(&para); //循环监听
//    while (1) {
//        apped_content(para.fpath,"watch start ...>>>\n");
//        //创建线程
//        thread_excute_open(para.fpath,para.command);
//        apped_content(para.fpath,"watch over .\n");
//        apped_content(para.fpath,"====sleep start====\n");
//        sleep(para.sleeptime);
//        apped_content(para.fpath,"====sleep over====\n\n");
//    }


//        save_content(param->fpath,"");//清空上一次的log
//        excute_opon(param->command);




//    apped_content(para.fpath,"nomal exit!\n");
//    char tmp[10];
//    memset(tmp, 0, sizeof(tmp));//清空
//    sprintf(tmp, "progress pid = %lu ,over\n", getpid());
//    apped_content(para.fpath, tmp);



//apped_content(console,"===================================== excute_opon() =============================\n");
//    int rc = 0;
//    char result_buf[1024];
//    char *line;


//    {
//       //apped_content(console,"popen() is error,file is null.\n");
//    }else{
//apped_content(console,"popen() is success.\n");

//memset(result_buf, 0, 1024);//清空数组
//line = result_buf;// 指针指向数组
// sprintf(line, "current process pid = %lu\n", getpid());//
//        apped_content(console,line);

/*memset(result_buf, 0, sizeof(result_buf));//清空数组
 while( fgets(result_buf, 1024, fp) != NULL)
 {
//            if('\n' == result_buf[strlen(result_buf)-1])
//            {
//                result_buf[strlen(result_buf)-1] = '\0';
//            }
 }
 apped_content(console,line);*/
//apped_content(console,"excute >> pclose() ... \n");
//        rc = pclose(fp);
//apped_content(console,"excute >> pclose() over >> ");
//        apped_content(console,"excute >> success() ... \n");
//        sprintf(line, "pclose() = %lu\n", rc);//
//        if(rc == -1)
//        {
//            //apped_content(console,"\nerror!\n");
//            exit(1);
//        }
//        else{
//apped_content(console,"success! \n");
//memset(result_buf, 0, sizeof(result_buf));//清空数组
//sprintf(line, "\npclose() result:%d ,return:%d \n",  rc, WEXITSTATUS(rc));
//        }
//        apped_content(console,line);
//   }




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


//char tme[20];
//memset(tme,0,sizeof(tme));
//strcpy(tme,path);//C语言标准库函数，将第二个参数的内容复制到第一个参数;
//sprintf(tme,CONTROL,tme); //控制文件
//sprintf(path,FFPID,path); //PID保存

//    fp = fopen(csd, "a");   //以可读的方式打开1.txt文件
//    if (fp == NULL) {
//        ftruncate(fp, 0);
//        lseek(fp, 0, SEEK_SET);
//    }
//    fclose(fp);//关闭文件流

//    execlp("am", "am", "start", "--user","0","-a", "android.intent.action.VIEW","-d","http://www.baidu.com");
//    execlp("am", "am", "startservice", "--user","0","-n", "com.lzp.ndks","com.wos.play.rootdir.model_monitor.soexcute.WatchServer");
//    popen();
//    system("am startservice --user 0 -n com.lzp.ndks/com.wos.play.rootdir.model_monitor.soexcute.WatchServer");
//    system("am start --user 0 -a android.intent.action.VIEW -d  http://www.baidu.com");
//  excute_command_popen(para.acomd);