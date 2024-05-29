#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int p[2], q[2];
    pipe(p);
    pipe(q);

    if(fork() == 0) {
        char buf[1];
        read(p[0], buf, sizeof buf);
        close(p[0]);
        close(p[1]);

        fprintf(1, "%d: received pong\n", getpid());

        write(q[1], buf, sizeof buf);
        close(q[1]);
    }
    else {
        char buf[1];
        buf[0] = 1;
        write(p[1], buf, sizeof buf);
        close(p[1]);

        read(q[0], buf, sizeof buf);
        close(q[0]);
        close(q[1]);

        fprintf(1, "%d: received ping\n", getpid());
    }

    exit(0);
}