#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main (int argc, char *argv[])
{
    if(argc < 2) {
        fprintf(2, "sleep: missing operand");
    }

    for(int i = 1; i < argc; ++i) {
        if(sleep(atoi(argv[i])) < 0) {
            fprintf(2, "sleep: %s failed to sleep for\n", argv[i]);
            break;
        }
    }

    exit(0);
}