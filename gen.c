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
#include <time.h>
#include <mqueue.h>

#define MSG_SIZE 256
#define MAX_MSG 10
#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
                                     exit(EXIT_FAILURE))
typedef unsigned int UINT;                                     
typedef struct timespec timespec_t;

void msleep(UINT milisec) {
    time_t sec= (int)(milisec/1000);
    milisec = milisec - (sec*1000);
    timespec_t req= {0};
    req.tv_sec = sec;
    req.tv_nsec = milisec * 1000000L;
    if(nanosleep(&req,&req)) ERR("nanosleep");
}

void usage(void){
        fprintf(stderr,"This is the generator.\nUSAGE: ./a q1 q2 n \n");
        exit(EXIT_FAILURE);
}

char rnd_char()
{
    return rand()%('z'-'a') + 'a';
}

int shouldPublish(int p){
    int ret = rand()%100 < p;
    return ret;
}

mqd_t createQueue(char *q_name){
    mqd_t q;
    struct mq_attr attr;
    attr.mq_maxmsg=MAX_MSG;
    attr.mq_msgsize=MSG_SIZE;
    if((q=TEMP_FAILURE_RETRY(mq_open(q_name, O_RDWR | O_EXCL | O_CREAT, 0600, &attr)))==(mqd_t)-1) ERR("mq open q2");
    return q;
}

mqd_t connectWithQueue(char *q_name) {
    mqd_t q;
    struct mq_attr attr;
    attr.mq_maxmsg=MAX_MSG;
    attr.mq_msgsize=MSG_SIZE;
    if((q=TEMP_FAILURE_RETRY(mq_open(q_name, O_RDWR, 0600, &attr)))==(mqd_t)-1) ERR("Couldnt connect with queue");
    return q;
}

size_t genShort(char *msg)
{
    snprintf(msg, MSG_SIZE, "%d/%c%c%c", getpid(), rnd_char(),rnd_char(),rnd_char());
    printf("DEBUG: wygenerowano short: %s\n", msg);
    return strnlen(msg,MSG_SIZE);
}

size_t genLong(char *msg, char *prevMsg)
{
    char buf[4];
    long ignore;
    sscanf(prevMsg, "%ld/%s", &ignore, buf);
    snprintf(msg, MSG_SIZE, "%d/%s/%c%c%c%c%c", getpid(), buf,
                    rnd_char(),rnd_char(),rnd_char(),rnd_char(),rnd_char());
    printf("DEBUG: wygenerowano long: %s\n", msg);
    return strnlen(msg,MSG_SIZE);
}

void work(int n, int p, int t, mqd_t q1, mqd_t q2)
{
    char msg[MSG_SIZE+1], msg2[MSG_SIZE+1];
    for (int i = 0; i < n; i++) {
        size_t msglen = genShort(msg);
        if(TEMP_FAILURE_RETRY(mq_send(q1,msg,msglen,0)))ERR("mq_send");
        printf("DEBUG: gen: Wyslano: %s\n",msg);
    }
    
    for (;;) {
        if(mq_receive(q1,msg,MSG_SIZE+1,NULL)<1) ERR("mq_receive");
        printf("DEBUG: gen: Otrzymano: %s\n",msg);
        msleep(t*1000);
        if(shouldPublish(p)) {
            size_t msglen = strnlen(msg,MSG_SIZE);
            genLong(msg2, msg);
            size_t msg2len = strnlen(msg2,MSG_SIZE);
            if(TEMP_FAILURE_RETRY(mq_send(q2,msg2,msg2len+1,1)))ERR("mq_send");
            if(TEMP_FAILURE_RETRY(mq_send(q1,msg,msglen+1,1)))ERR("mq_send");
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 5) usage();
    int n = -1;
    unsigned int t = atoi(argv[1]);
    unsigned int p = atoi(argv[2]);
    char *q1_name = argv[3];
    char *q2_name = argv[4];
    if (t < 1 || t > 10) ERR("Wrong t param");
    if (p < 0 || p > 100) ERR("Wrong p param");

    if (argc == 6) {
        n = atoi(argv[5]);
        if (n < 1 || n > 10) ERR("Wrong n param");
    }    
    
    mqd_t q1;
    mqd_t q2;

    if (n>-1) {
        q1 = createQueue(q1_name);
        q2 = createQueue(q2_name);
    } else {
        q1 = connectWithQueue(q1_name);
        q2 = connectWithQueue(q2_name);
    }
    srand(getpid());
    work(n, p, t, q1, q2);
    if(mq_close(q2) != 0) ERR("mq close");
}