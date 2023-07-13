#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
void find(char* path, char* fn) {
//   printf("find in %s\n", path);
  int fd;
  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
    }
    struct stat st;
    if(fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    struct dirent de;
    char buf[512], *p;
    switch (st.type) {
        case T_FILE:
            for(p = path + strlen(path); p >= path && *p != '/';p--);
            p++;
            if (!strcmp(p, fn)) {
                printf("%s\n", path);
            }
            break;
        case T_DIR:
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                printf("find: path too long\n");
                break;
            }
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';
            while(read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0) {
                    continue;
                }
                if (strcmp(de.name, "..") && strcmp(de.name, ".")) {
                    memmove(p, de.name, DIRSIZ);
                    p[DIRSIZ] = 0;
                    find(buf, fn);
                }
            }
            break;
    }
    close(fd);
}
int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(2, "Format: Find {} {}\n");
    }
    find(argv[1], argv[2]);
    exit(0);
}