#define _GNU_SOURCE
#include <signal.h>


int p[2];

void signal_handler(int sig) {
    write(p[1], &sig, sizeof(int));
}

int signalfd() {
    pipe(p);
    for (int sig = 0; sig < 32; ++sig) {
        signal(sig, signal_handler);
    }
    return p[0];
}
