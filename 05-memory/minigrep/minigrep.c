#include <dirent.h>
#include <pcre.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>


void process_file(pcre* regex, const char* path) {
    int fd = open(path, O_RDONLY);
    size_t file_size = lseek(fd, 0, SEEK_END);
    char* file = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

    const char* line = file;
    const char* end = file + file_size;
    int line_number = 1;

    while (line < end) {
        const char* newline = memchr(line, '\n', end - line);
        int line_length = (newline) ? (newline - line) : (end - line);

        int result = pcre_exec(regex, NULL, line, line_length, 0, 0, NULL, 0);
        if (result >= 0) {
            printf("%s:%d: ", path, line_number);
            fwrite(line, 1, line_length, stdout);
            printf("\n");
        }

        line += line_length + 1;
        line_number++;
    }


    munmap(file, file_size);
    close(fd);
}


void process_dir(pcre* regex, const char* path) {
    DIR* dir = opendir(path);
    struct dirent* entry;
    while((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            struct stat path_stat;
            stat(full_path, &path_stat);

            if (S_ISDIR(path_stat.st_mode)) {
                process_dir(regex, full_path);
            } else if (S_ISREG(path_stat.st_mode)) {
                process_file(regex, full_path);
            }
        }
    }
    closedir(dir);
}


int main(int argc, char* argv[]) {
    const char* regex = argv[1];
    const char* dir = argv[2];

    const char *pcre_error;
    int error_offset;
    pcre* re = pcre_compile(regex, 0, &pcre_error, &error_offset, NULL);

    if (!re) {
        write(2, pcre_error, strlen(pcre_error));
        return 1;
    }

    process_dir(re, dir);

    pcre_free(re);
    return 0;
}
