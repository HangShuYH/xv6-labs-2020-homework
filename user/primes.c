#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int prime[11] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31};
void process(int sieve, int p[]) {
    int num;
    while (read(p[0], &num, sizeof(num)) != 0) {
    //   printf("process %d received %d\n", prime[sieve], num);
      if (num == prime[sieve]) {
        printf("prime %d\n", num);
      } else if (num % prime[sieve] != 0) {
        int p1[2];
        pipe(p1);
        int pid = fork();
        if (pid == 0) {
          // child
          close(p[0]);
          close(p1[1]);
          process(sieve + 1, p1);
        } else {
          // parent
          close(p1[0]);
        //   printf("process %d send %d\n", prime[sieve], num);
          write(p1[1], &num, sizeof(num));
          close(p1[1]);
          wait((int*)0);
        }
      }
    }
    close(p[0]);
    exit(0);
}
int main(int argc, char* argv[]) {
    int p[2];
    pipe(p);
    for (int i = 2; i <= 35; i++) {
        write(p[1], &i, sizeof(i));
    }
    close(p[1]); // close write
    int pid = fork();
    if (pid == 0) {
        // child
        process(0, p);
    } else {
        // parent
        close(p[0]); // close read
        wait((int*)0);
    }
    exit(0);
}
