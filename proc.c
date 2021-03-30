//Oświadczam, że niniejsza praca stanowiąca podstawę do uznania osiągnięcia efektów
//uczenia się z przedmiotu SOP2 została wykonana przeze mnie samodzielnie.
//Filip Spychala
//305797
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <mqueue.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define ELAPSED(start,end) ((end).tv_sec-(start).tv_sec)+(((end).tv_nsec - (start).tv_nsec) * 1.0e-9)
#define MIDDLE_CHARS_COUNT 3
#define LAST_CHARS_COUNT 5
#define MSG_SIZE 256
#define MAX_MSG 10
#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
                                     exit(EXIT_FAILURE))
typedef unsigned int UINT;                                     
typedef struct timespec timespec_t;

// void sethandler( void (*f)(int, siginfo_t*, void*), int sigNo) {
//         struct sigaction act;
//         memset(&act, 0, sizeof(struct sigaction));
//         act.sa_sigaction = f;
//         act.sa_flags=SA_SIGINFO; // todo zostawic?
//         if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
// }

void usage(void){
        fprintf(stderr,"This is the processor.\nUSAGE: ./b q2\n");
        exit(EXIT_FAILURE);
}

void msleep(UINT milisec) {
    time_t sec= (int)(milisec/1000);
    milisec = milisec - (sec*1000);
    timespec_t req= {0};
    req.tv_sec = sec;
    req.tv_nsec = milisec * 1000000L;
    if(nanosleep(&req,&req)) ERR("nanosleep");
}

void transformMsg(char *in, char *out, size_t buf_size){
    char buf1[MIDDLE_CHARS_COUNT + 1], buf2[LAST_CHARS_COUNT + 1];
    long pid;
    sscanf(in, "%ld/%s/%s", &pid, buf1, buf2);
    snprintf(out, buf_size, "%d/%s/%s", getpid(), "000", buf2);
    printf("DEBUG: proc: Przeksztalcono do: %s\n", out);
}

int shouldPublish(int p){
    int ret = rand()%100 < p;
    printf("DEBUG: wylosowano %d\n", ret);
    return ret;
}

mqd_t openQueue(const char* q_name){
    mqd_t ret;
    struct mq_attr attr;
    attr.mq_maxmsg=MAX_MSG;
    attr.mq_msgsize=MSG_SIZE;
    if((ret=TEMP_FAILURE_RETRY(mq_open(q_name, O_RDWR, 0600, &attr)))==(mqd_t)-1) 
    {
        if (errno == ENOENT) ERR("Such a queue does not exist!");
        ERR("mq open q2");
    }
    return ret;
}

void receiveFromQueue(mqd_t q, char *msg) {
        int len;
        if((len = mq_receive(q,msg,MSG_SIZE+1,NULL))<1) ERR("mq_receive");
        msg[len - 1]='\0';
        printf("DEBUG: len = %d\n", len);
        printf("proc: Otrzymano: %s\n", msg);
}

void publishToQueue(mqd_t q, const char *msg) {
    size_t msglen = strnlen(msg,MSG_SIZE);
    printf("DEBUG: proc: Opublikowano 1: %s\n", msg);
    if(TEMP_FAILURE_RETRY(mq_send(q,msg,msglen+1,0)))ERR("mq_send");
    printf("DEBUG: proc: Opublikowano 2: %s\n", msg);
}


int main(int argc, char** argv) {
    if (argc != 4) usage();
    unsigned int t = atoi(argv[1]);
    unsigned int p = atoi(argv[2]);
    char* q2_name = argv[3];
    printf("DEBUG1\n");
    if (t < 1 || t > 10) ERR("Wrong t param");
    if (p < 0 || p > 100) ERR("Wrong p param");
    srand(getpid());
    mqd_t q2 = openQueue(q2_name);
    printf("DEBUG2\n");
    
    char msgIn[MSG_SIZE+1];
    char msgOut[MSG_SIZE+1];

    while(1){
        printf("DEBUG3\n");
        receiveFromQueue(q2, msgIn);
        msleep(t*1000);
        printf("DEBUG4\n");
        if(shouldPublish(p)) {
            printf("DEBUG5\n");
            transformMsg(msgIn, msgOut, MSG_SIZE+1);
            publishToQueue(q2, msgOut);
        } else printf("DEBUG: nie publikujemy\n");       
    }

    if(mq_close(q2) != 0) ERR("mq close");
}