#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096

void follow_file(const char *filename) {

    int fd = open(filename, O_RDONLY);
    int inotify_fd = inotify_init();
    int wd = inotify_add_watch(inotify_fd, filename, IN_MODIFY);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, bytes_read, stdout);
    }
    fflush(stdout);


    off_t offset = lseek(fd, 0, SEEK_CUR);
    while (0==0) {
        char event_buf[BUFFER_SIZE];
        ssize_t event_length = read(inotify_fd, event_buf, sizeof(event_buf));

        for (char *ptr = event_buf; ptr < event_buf + event_length;) {
            struct inotify_event *event = (struct inotify_event *)ptr;

            if (event->mask & IN_MODIFY) {
                lseek(fd, offset, SEEK_SET);
                while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
                    fwrite(buffer, 1, bytes_read, stdout);
                    fflush(stdout);
                }

                offset = lseek(fd, 0, SEEK_CUR);
            }
            ptr += sizeof(struct inotify_event) + event->len;
        }
    }

    inotify_rm_watch(inotify_fd, wd);
    close(inotify_fd);
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Incorrect number of arguments");
        exit(1);
    }

    follow_file(argv[1]);
    return 0;
}
