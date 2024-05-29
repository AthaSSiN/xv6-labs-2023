#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {

    int pid = fork();

    if(pid < 0) {
        fprintf(2, "xargs: error in fork");
        exit(1);
    }
    else if(pid == 0) {

        // read from stdin, can be multiple lines

        char *sendargv[MAXARG];
        int i;
        for(i = 0; i < argc - 1; ++i)
            sendargv[i] = argv[i + 1];
        
        char buf[512] = "";
        char *p = buf;

        while(read(0, p, sizeof(char))) { 
            // fprintf(1, "%c", *p);
            if(*p == '\n' || *p == ' ') {

                if(i == MAXARG) {
                    fprintf(2, "xargs: too many arguments");
                    exit(1);
                }
                *p = '\0';
                sendargv[i] = (char*)malloc(sizeof buf);
                strcpy(sendargv[i++], buf);
                p = buf - 1;
            }
            *(++p) = '\0';
        }

        if(i == MAXARG) {
            fprintf(2, "xargs: too many arguments");
            exit(1);
        }
        if(*buf != '\0'){
            sendargv[i] = (char*)malloc(sizeof buf);
            strcpy(sendargv[i++], buf);

        }

        // for(int j = 0; j < MAXARG; ++j)
        //     fprintf(2, "%s ", sendargv[j]);

        exec(sendargv[0], sendargv);

        fprintf(2, "xargs: error in executing command");
        exit(1);

    }
    else {
        wait((int*) 0);
        exit(0);
    }
}