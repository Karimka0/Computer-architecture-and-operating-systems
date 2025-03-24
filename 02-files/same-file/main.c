#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

bool is_same_file(const char* lhs_path, const char* rhs_path) {
    struct stat stat1;
    struct stat stat2;

    if (stat(lhs_path, &stat1) != -1 && stat(rhs_path, &stat2) != -1) {
        return stat1.st_ino == stat2.st_ino;
    }
    return false;
}

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        return -1;
    }

    if (is_same_file(argv[1], argv[2])) {
        printf("yes");
    } else {
        printf("no");
    }

    return 0;
}
