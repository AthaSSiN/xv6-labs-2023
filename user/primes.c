#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void primes(int* fdl) {

    close(fdl[1]);
    int p;
    if(read(fdl[0], &p, sizeof p)) {
        fprintf(1, "prime %d\n", p);
        int fd[2];
        pipe(fd);
        fprintf(2, "%d %d\n", fd[0], fd[1]);
        if(fork() == 0) {
            // right process
            primes(fd);
        }
        else {
            // left process
            close(fd[0]);
            int n;
            while(read(fdl[0], &n, sizeof n)) {
                if(n % p) {
                    // forward to right
                    write(fd[1], &n, sizeof n);
                }
            }
            close(fd[1]);
            wait((int *)0);
        }
    }
    close(fdl[0]);

}

int main(int argc, char *argv[]) {

    // make generating process
    int fd[2];
    pipe(fd);
    fprintf(2, "%d %d\n", fd[0], fd[1]);
    if(fork() == 0) {
        primes(fd);
    }
    else {
        close(fd[0]);
        for(int i = 2; i <= 36; ++i) {
            // forward to right
            write(fd[1], &i, sizeof i);
        }
        close(fd[1]);
        wait((int *)0);
    }
    exit(0);
}
