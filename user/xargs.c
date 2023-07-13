#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"
#define PRINT_ARGV \
        do { \
        for (int i = 0; i < argc_exec; i++) { \
        printf("%s ", argv_exec[i]); \
        } \
        printf("\n"); \
        } while(0);
int main(int argc, char* argv[]) {
    char ch;
    char* argv_exec[MAXARG];
    int argc_exec = 0;
    for (argc_exec = 0;argc_exec < argc - 1; argc_exec++) {
        argv_exec[argc_exec] = argv[argc_exec + 1];
    }
    argv[argc - 1] = (char*)0;
    char buf[512];
    int idx = 0;
    // PRINT_ARGV;
    while(read(0, &ch, sizeof(ch)) != 0) {
        if (ch == '\n' || ch == ' ') {
            char buf2[512];
            memcpy(buf2, buf, idx);
            buf2[idx] = '\0';
            argv_exec[argc_exec++] = buf2;
            idx = 0;
            if (ch == '\n') {
                if (fork() == 0) {
                    argv_exec[argc_exec] = (char *)0;
                    // PRINT_ARGV;
                    exec(argv_exec[0], argv_exec);
                    fprintf(2, "Exec Failed!\n");
                    exit(1);
                } else {
                    wait((int*)0);
                    argc_exec = argc - 1;
                }
            }
        } else {
            buf[idx++] = ch;
        }
    }
    exit(0);
}