
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>

/* Getrlimit - get resource limits and check for errors
 */
void Getrlimit( int limitType, struct rlimit *r) {
    int res = getrlimit( limitType, r);
    if(res < 0){ perror("getrlimit"); exit(1);}
}


void doit() {       // TODO: do the changes here

    char  var[9 * 1024 * 1024];
    
    while(1);
}


/* main
 */
int main(int argc, char **argv)
{
    struct rlimit r;
    printf("memteste\n");
    Getrlimit( RLIMIT_AS, &r);
    printf("addres space limit: soft limit=%16lx, hard limit=%16lx\n",
               (unsigned long)r.rlim_cur, (unsigned long)r.rlim_max);

    Getrlimit( RLIMIT_DATA, &r);
    printf("data limit:         soft limit=%16lx, hard limit=%16lx\n",
               (unsigned long)r.rlim_cur, (unsigned long)r.rlim_max);

    Getrlimit( RLIMIT_STACK, &r);
    printf("stack limit:        soft limit=%16lx, hard limit=%16lx\n",
               (unsigned long)r.rlim_cur, (unsigned long)r.rlim_max);

    doit();

    return 0;
}

