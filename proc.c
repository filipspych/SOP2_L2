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

char rnd_char()
{
    return rand()%('z'-'a') + 'a';
}

void msleep(UINT milisec) {
    time_t sec= (int)(milisec/1000);
    milisec = milisec - (sec*1000);
    timespec_t req= {0};
    req.tv_sec = sec;
    req.tv_nsec = milisec * 1000000L;
    if(nanosleep(&req,&req)) ERR("nanosleep");
}

void transformMsg(char *in, char *out, size_t buf_size)
{
    char buf1[MIDDLE_CHARS_COUNT + 1], buf2[LAST_CHARS_COUNT + 1];
    long pid;
    sscanf(in, "%ld/%s/%s", &pid, buf1, buf2);
    snprintf(out, buf_size, "%d/%s/%s", getpid(), "000", buf2);
}

int main(int argc, char** argv)
{
    if (argc != 4) usage();
    unsigned int t = atoi(argv[1]);
    unsigned int p = atoi(argv[1]);
    char* q2_name = argv[3];
    
    if (t < 1 || t > 10) ERR("Wrong t param");
    if (p < 0 || p > 100) ERR("Wrong p param");
    
    
    mqd_t q2;
    struct mq_attr attr;
    attr.mq_maxmsg=MAX_MSG;
    attr.mq_msgsize=MSG_SIZE;
    if((q2=TEMP_FAILURE_RETRY(mq_open(q2_name, O_RDWR, 0600, &attr)))==(mqd_t)-1) 
    {
        if (errno == ENOENT) ERR("Such a queue does not exist!");
        ERR("mq open q2");
    }
    
    char msgIn[MSG_SIZE+1];
    char msgOut[MSG_SIZE+1];

    while(1){
        if(mq_receive(q2,(char*)&msgIn,MSG_SIZE+1,NULL)<1) ERR("mq_receive");
        msleep(t*1000);

        transformMsg(msgIn, msgOut, MSG_SIZE+1);
        printf("proc: Otrzymano: %s\n", msgIn);
        printf("DEBUG: proc: Przeksztalcono do: %s\n", msgOut);
    }

    if(mq_close(q2) != 0) ERR("mq close");
}