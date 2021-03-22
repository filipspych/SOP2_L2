#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <mqueue.h>

#define MSG_SIZE 256
#define MAX_MSG 10
#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
                                     exit(EXIT_FAILURE))

// void sethandler( void (*f)(int, siginfo_t*, void*), int sigNo) {
//         struct sigaction act;
//         memset(&act, 0, sizeof(struct sigaction));
//         act.sa_sigaction = f;
//         act.sa_flags=SA_SIGINFO; // todo zostawic?
//         if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
// }
void usage(void){
        fprintf(stderr,"USAGE: ./a q2 n \n");
        exit(EXIT_FAILURE);
}

char rnd_char()
{
    return rand()%('z'-'a') + 'a';
}

int main(int argc, char** argv)
{
    if (argc < 3) usage();
    char* q2_name = argv[1];
    int n = atoi(argv[2]);
    srand(time(0));
    mqd_t q2;
    struct mq_attr attr;
    attr.mq_maxmsg=MAX_MSG;
    attr.mq_msgsize=MSG_SIZE;
    if((q2=TEMP_FAILURE_RETRY(mq_open(q2_name, O_RDWR | O_EXCL | O_CREAT, 0600, &attr)))==(mqd_t)-1) ERR("mq open q2");
    
    char msg[MSG_SIZE+1];
    for (int i = 0; i < n; i++) {
        snprintf(msg, MSG_SIZE, "%d/%c%c%c", getpid(), rnd_char(),rnd_char(),rnd_char());
        size_t msglen = strnlen(msg,MSG_SIZE);
        if(TEMP_FAILURE_RETRY(mq_send(q2,(const char*)&msg,msglen,0)))ERR("mq_send");
    }

    if(mq_close(q2) != 0) ERR("mq close");
}