#include <com_wos_play_rootdir_model_monitor_soexcute_RunJniHelper.h>

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
            result = strcmp(tem, buff1);
        }
    }
    fclose(fp);
//
//    fp=fopen(csd,"w"); //
//    char buff[10];
//    sprintf(buff,"%lu",result);
//    fprintf(fp,"%s\n",buff); // 把进程号写入文件
//    fflush(fp);
//    fclose(fp);
    return result;
}





/**
 * 检测服务，如果不存在服务则启动.
 * 通过am命令启动一个laucher服务,
 * 由laucher服务负责进行主服务的检测,
	laucher服务在检测后自动退出
 am startservice [--user <USER_ID> | current] <INTENT>
 */
void thread(char* srvname,char* csd) {
    char command[200];//命令
    command[199] = '\0';
    sprintf(command, "am startservice --user 0 %s", srvname);
    FILE * fp;
    while(1){
        if(killself(csd)==0){
            exit(0);
        }
        //读取文件
        fp = popen(command, "r");
        pclose(fp);
        sleep(15);
    }
}


//保存进程编号
void saveProgress(char* sd){
    FILE *fp ; //文件指针
    fp=fopen(sd,"w"); //
    char buff[10];
    sprintf(buff,"%lu",getpid());
    fprintf(fp,"%s\n",buff); // 把进程号写入文件
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
    if(fp == NULL){
        LOGE("文件 不存在: %s",sd);
        return;
    }else{
        char buff[10];
        memset(buff,0,sizeof(buff));//为申请的内存做初始化工作
        fseek(fp,0,SEEK_SET);
        fgets(buff,10,fp);  //读取一行
        LOGE("从文件中读取的进程号[ %s ]",buff);
        if(strlen(buff)>1){ // 有值
            kill(atoi(buff), SIGTERM);
            LOGE("杀死进程[ %d ]",atoi(buff));
        }
    }
    fclose(fp);
    LOGE("----------------------------------------------------------------------");
}

//创建进程
void createProgress(char* srvname,char* sd,char *csd){
    LOGI("准备打开的server全路径名[ %s ];\n进程号保存文件的路径:[ %s ];\n控制文件路劲:[%s] ", srvname,sd,csd);
    int cpid = fork();
    LOGI("当前进程 pid[ %d ] fork结果 : [ %d ]", getpid(),cpid);
    if (cpid < 0) {
        LOGE("fork函数调用错误,退出程序");
        exit(0);
    }else if(cpid>0){
        LOGE("当前是父进程( %d ),退出.", getpid());
        exit(0);
    }
    LOGI("子进程(%d)", getpid());
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
    killProgress(sd);
    saveProgress(sd);
    //输出到null
    int stdfd = open ("/dev/null", O_RDWR);
    dup2(stdfd, STDOUT_FILENO);
    dup2(stdfd, STDERR_FILENO);
    thread(srvname,csd);
}

/**
 * 开始服务
 */
JNIEXPORT void JNICALL Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_startMservice
(JNIEnv *env, jobject thiz, jstring cchrptr_ProcessName, jstring sdpath)
{
    char* sernam = jstringTostring(env, cchrptr_ProcessName); // 得到服务名称
    char* sd = jstringTostring(env, sdpath);
    char csd[30]= "";
    strcpy(csd,sd);
    //拼接路径
    sprintf(sd,"%s/pidfile",sd);
    sprintf(csd,"%s/ctty",csd);
    createProgress(sernam,sd,csd);
}

/**
停止服务
 */
JNIEXPORT void JNICALL Java_com_wos_play_rootdir_model_1monitor_soexcute_RunJniHelper_stopMservice
(JNIEnv *env, jobject thiz, jstring sdpath)
{
    char * sd = jstringTostring(env, sdpath);
    sprintf(sd,"%s/pidfile",sd);//拼接路径
    killProgress(sd);
}