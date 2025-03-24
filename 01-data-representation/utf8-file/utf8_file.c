#include "utf8_file.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>


char *encode_utf8(uint32_t codepoint, char *out, size_t *outlen) {
    if (codepoint <= 0x7F) {
        out[0] = (char) codepoint;
        *outlen = 1;
    } else if (codepoint <= 0x7FF) {
        out[0] = (char) ((codepoint >> 6) | 0xC0);
        out[1] = (char) ((codepoint & 0x3F) | 0x80);
        *outlen = 2;
    } else if (codepoint <= 0xFFFF) {
        out[0] = (char) ((codepoint >> 12) | 0xE0);
        out[1] = (char) (((codepoint >> 6) & 0x3F) | 0x80);
        out[2] = (char) ((codepoint & 0x3F) | 0x80);
        *outlen = 3;
    } else if (codepoint <= 0x1FFFFF) {
        out[0] = (char) ((codepoint >> 18) | 0xF0);
        out[1] = (char) (((codepoint >> 12) & 0x3F) | 0x80);
        out[2] = (char) (((codepoint >> 6) & 0x3F) | 0x80);
        out[3] = (char) ((codepoint & 0x3F) | 0x80);
        *outlen = 4;
    } else if (codepoint <= 0x3FFFFFF) {
        out[0] = (char) ((codepoint >> 24) | 0xF8);
        out[1] = (char) (((codepoint >> 18) & 0x3F) | 0x80);
        out[2] = (char) (((codepoint >> 12) & 0x3F) | 0x80);
        out[3] = (char) (((codepoint >> 6) & 0x3F) | 0x80);
        out[4] = (char) ((codepoint & 0x3F) | 0x80);
        *outlen = 5;
    } else if (codepoint <= 0x7FFFFFFF) {
        out[0] = (char) ((codepoint >> 30) | 0xFC);
        out[1] = (char) (((codepoint >> 24) & 0x3F) | 0x80);
        out[2] = (char) (((codepoint >> 18) & 0x3F) | 0x80);
        out[3] = (char) (((codepoint >> 12) & 0x3F) | 0x80);
        out[4] = (char) (((codepoint >> 6) & 0x3F) | 0x80);
        out[5] = (char) ((codepoint & 0x3F) | 0x80);
        *outlen = 6;
    }
    return out;
}

size_t decode_utf8(const char *input, uint32_t *output) {
    const unsigned char *ptr = (const unsigned char *) input;
    if (ptr[0] < 0x80) {
        *output = ptr[0];
        return 1;
    } else if ((ptr[0] & 0xE0) == 0xC0 && (ptr[1] & 0xC0) == 0x80) {
        *output = ((ptr[0] & 0x1F) << 6) | (ptr[1] & 0x3F);
        return 2;
    } else if ((ptr[0] & 0xF0) == 0xE0 && (ptr[1] & 0xC0) == 0x80
               && (ptr[2] & 0xC0) == 0x80) {
        *output = ((ptr[0] & 0x0F) << 12) | ((ptr[1] & 0x3F) << 6) | (ptr[2] & 0x3F);
        return 3;
    } else if ((ptr[0] & 0xF8) == 0xF0 && (ptr[1] & 0xC0) == 0x80
               && (ptr[2] & 0xC0) == 0x80 && (ptr[3] & 0xC0) == 0x80) {
        *output = ((ptr[0] & 0x07) << 18) | ((ptr[1] & 0x3F) << 12) |
                  ((ptr[2] & 0x3F) << 6) | (ptr[3] & 0x3F);
        return 4;
    } else if ((ptr[0] & 0xFC) == 0xF8 && (ptr[1] & 0xC0) == 0x80
               && (ptr[2] & 0xC0) == 0x80 && (ptr[3] & 0xC0) == 0x80
               && (ptr[4] & 0xC0) == 0x80) {
        *output = ((ptr[0] & 0x03) << 24) | ((ptr[1] & 0x3F) << 18) |
                  ((ptr[2] & 0x3F) << 12) | ((ptr[3] & 0x3F) << 6) |
                  (ptr[4] & 0x3F);
        return 5;
    } else if ((ptr[0] & 0xFE) == 0xFC && (ptr[1] & 0xC0) == 0x80
               && (ptr[2] & 0xC0) == 0x80 && (ptr[3] & 0xC0) == 0x80
               && (ptr[4] & 0xC0) == 0x80 && (ptr[5] & 0xC0) == 0x80) {
        *output = ((ptr[0] & 0x01) << 30) | ((ptr[1] & 0x3F) << 24) |
                  ((ptr[2] & 0x3F) << 18) | ((ptr[3] & 0x3F) << 12) |
                  ((ptr[4] & 0x3F) << 6) | (ptr[5] & 0x3F);
        return 6;
    }
    return 0;
}


int utf8_read(utf8_file_t *f, uint32_t *res, size_t count) {
    unsigned char in[6];
    size_t total_read = 0;

    while (total_read < count) {
        ssize_t result = read(f->fd, &in[0], 1);
        if (result < 0) {
            if (errno == EINTR) continue;
            return -1;
        } else if (result == 0) break;

        size_t symbols_needed = 0;
        uint32_t codepoint = 0;

        if (in[0] < 0x80) {
            res[total_read++] = in[0];
            continue;
        } else if ((in[0] & 0xE0) == 0xC0) {
            symbols_needed = 2;
        } else if ((in[0] & 0xF0) == 0xE0) {
            symbols_needed = 3;
        } else if ((in[0] & 0xF8) == 0xF0) {
            symbols_needed = 4;
        } else if ((in[0] & 0xFC) == 0xF8) {
            symbols_needed = 5;
        } else if ((in[0] & 0xFE) == 0xFC) {
            symbols_needed = 6;
        } else {
            return -1;
        }

        ssize_t read_bytes = read(f->fd, &in[1], symbols_needed - 1);
        if (read_bytes < (ssize_t)(symbols_needed - 1)) {
            return -1;
        }

        size_t symbols_read = decode_utf8((char *)in, &codepoint);
        if (symbols_read == 0) {
            return -1;
        }
        res[total_read++] = codepoint;
    }
    return (int)total_read;
}


int utf8_write(utf8_file_t *f, const uint32_t *str, size_t count) {
    char out[6];
    size_t written = 0;

    for (size_t i = 0; i < count; ++i) {
        size_t outlen = 0;
        encode_utf8(str[i], out, &outlen);

        ssize_t result = write(f->fd, out, outlen);
        if (result != (ssize_t) outlen) {
            errno = EIO;
            return -1;
        }
        ++written;
    }
    return written;
}

utf8_file_t *utf8_fromfd(int fd) {
    if (fd >= 0) {
        utf8_file_t *file = (utf8_file_t *) malloc(sizeof(utf8_file_t));
        file->fd = fd;
        return file;
    }
    return NULL;
}
