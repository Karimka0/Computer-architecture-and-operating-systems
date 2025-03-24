#include "lca.h"
#include <stdio.h>

pid_t get_parent_pid(pid_t son) {
    char path[256];
    char line[256];
    FILE *status_file;
    int ppid = -1;

    snprintf(path, sizeof(path), "/proc/%d/status", son);

    status_file = fopen(path, "r");

    while (fgets(line, sizeof(line), status_file)) {
        if (sscanf(line, "PPid: %d", &ppid) == 1) {
            break;
        }
    }

    fclose(status_file);

    return ppid;
}


pid_t find_lca(pid_t x, pid_t y) {
    if (x == y) {
        return x;
    }

    pid_t x_parents[MAX_TREE_DEPTH];
    pid_t y_parents[MAX_TREE_DEPTH];

    x_parents[0] = x;
    y_parents[0] = y;

    int h1 = 0;
    int h2 = 0;
    do { // try to change do and while
        ++h1;
        x_parents[h1] = get_parent_pid(x_parents[h1 - 1]);
    } while (x_parents[h1] != 0);

    do { // try to change do and while
        ++h2;
        y_parents[h2] = get_parent_pid(y_parents[h2 - 1]);
    } while (y_parents[h2] != 0);

    int y_up = 1;
    int x_up = 1;

    while (h1 != h2) {
        if (h1 > h2) {
            x = x_parents[x_up++];
            --h1;
        } else {
            y = y_parents[y_up++];
            --h2;
        }
    }
    while (x != y) {
        x = x_parents[x_up++];
        y = y_parents[y_up++];
    }
    return x;
}
