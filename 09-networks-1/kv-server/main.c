#include <fcntl.h>
#include <linux/limits.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct StorageItem {
    char key[PATH_MAX];
    char value[PATH_MAX];
    struct StorageItem* next;
} StorageItem;

typedef struct Storage {
    struct StorageItem* head;
} Storage;

StorageItem* find(Storage* storage, char* key) {
    StorageItem* current = storage->head;
    while (current != NULL) {
        if (strncmp(current->key, key, PATH_MAX) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void set(Storage* storage, char* key, char* value) {
    StorageItem* element = find(storage, key);
    if (element == NULL) {
        element = malloc(sizeof(StorageItem));
        strncpy(element->key, key, PATH_MAX - 1);
        element->key[PATH_MAX - 1] = '\0';
        element->next = storage->head;
        storage->head = element;
    }
    strncpy(element->value, value, PATH_MAX - 1);
    element->value[PATH_MAX - 1] = '\0';
}

char* get(Storage* storage, char* key) {
    StorageItem* element = find(storage, key);
    if (element == NULL) {
        return "";
    } else {
        return element->value;
    }
}

void process_client(Storage* storage, int fd) {
    char buffer[512];
    int res = read(fd, buffer, sizeof(buffer) - 1);

    if (res <= 0) {
        close(fd);
        return;
    }
    buffer[res] = '\0';

    if (strncmp(buffer, "get ", 4) == 0) {
        char* key = buffer + 4;
        key[strcspn(key, "\r\n")] = '\0';
        char* value = get(storage, key);
        dprintf(fd, "%s\n", value);
    } else if (strncmp(buffer, "set ", 4) == 0) {
        char* key = buffer + 4;
        char* space = strchr(key, ' ');
        if (space == NULL) {
            dprintf(fd, "Invalid command\n");
            close(fd);
            return;
        }
        *space = '\0';
        char* value = space + 1;
        value[strcspn(value, "\r\n")] = '\0';
        set(storage, key, value);
    } else {
        dprintf(fd, "Unknown command\n");
    }
}

int main(int argc, char* argv[]) {
    int server_port = atoi(argv[1]);
    if (server_port <= 0) {
        fprintf(stderr, "Invalid port number\n");
        return 1;
    }

    Storage* storage = malloc(sizeof(Storage));
    storage->head = NULL;

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        return 1;
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_port = htons(server_port),
            .sin_addr.s_addr = INADDR_ANY,
    };

    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &(int){1}, sizeof(int));
    if (bind(socket_fd, (struct sockaddr*)(&addr), sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    if (listen(socket_fd, SOMAXCONN) < 0) {
        perror("listen");
        return 1;
    }

    int flags = fcntl(socket_fd, F_GETFD);
    flags |= O_NONBLOCK;
    fcntl(socket_fd, F_SETFD, flags);

    struct epoll_event ev = {.events = EPOLLIN, .data.fd = socket_fd};
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev);

    struct epoll_event events[8];
    for (;;) {
        int n = epoll_wait(epoll_fd, events, 8, -1);
        for (int i = 0; i < n; ++i) {
            if (events[i].data.fd == socket_fd) {
                int client_fd = accept(socket_fd, NULL, NULL);
                if (client_fd == -1) {
                    perror("accept");
                    continue;
                }

                flags = fcntl(client_fd, F_GETFD);
                flags |= O_NONBLOCK;
                fcntl(client_fd, F_SETFD, flags);

                struct epoll_event ev = {.events = EPOLLIN, .data.fd = client_fd};
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
            } else {
                process_client(storage, events[i].data.fd);
            }
        }
    }

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
    return 0;
}
